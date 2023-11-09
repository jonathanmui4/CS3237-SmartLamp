import logging
import urllib.parse
import json
import boto3
import numpy as np
import pickle
import io
from PIL import Image
import torch
import torch.nn as nn
from torchvision import models, transforms
from torchvision.models import ResNet18_Weights

# Initialize the S3 client
s3 = boto3.client('s3')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

IMG_SIZE = (128, 128)
activity_labels = ["reading", "not_present", "asleep", "computer_use"]
# posture_labels = ['good_posture', 'bad_posture']

def load_and_preprocess_image(bucket, object_key):
    try:
        logger.info("Loading Image")
        image_object = s3.get_object(Bucket=bucket, Key=object_key)
        image_content = image_object['Body'].read()

        # Open the image and convert to RGB (in case it's a different format)
        image = Image.open(io.BytesIO(image_content)).convert('RGB')

        # Define the same transforms you used during model training
        transform = transforms.Compose([
            transforms.Resize(IMG_SIZE),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
        ])

        # Apply the transforms to the image
        image_tensor = transform(image).unsqueeze(0)  # Add batch dimension
        return image_tensor
    except Exception as e:
        # Handle any errors that occur
        logging.error(f'Error loading image: {e}')
        raise e

# Function to load the model (this should match your model's architecture)
def load_model():
    try:
        logger.info("Loading model")
        # Step 1: Recreate the original ResNet18 model
        model_reloaded = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)

        # Step 2: Modify the final layers to match the original changes
        num_ftrs = model_reloaded.fc.in_features
        model_reloaded.fc = nn.Linear(num_ftrs, 4)  # Match the number of classes

        # Step 3: Load the saved state dictionary
        with open('activity_detection.pkl', 'rb') as f:
            model_state = pickle.load(f)
        model_reloaded.load_state_dict(model_state)

        # Step 4: Set the model to evaluation mode if you're doing inference
        model_reloaded.eval()
        return model_reloaded
    except Exception as e:
        # Handle any errors that occur
        logging.error(f'Error loading model: {e}')
        raise e

def lambda_handler(event, context):
    try:
        # Get the S3 bucket and object name from the event.
        bucket = event['Records'][0]['s3']['bucket']['name']
        object_key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')

        # Log the name of the file using logger.
        logger.info(f'File uploaded to S3 bucket: {bucket}, object key: {object_key}')

        image_tensor = load_and_preprocess_image(bucket, object_key)
        model = load_model()

        with torch.no_grad():
            outputs = model(image_tensor)
            _, predicted = torch.max(outputs, 1)
            predicted_label = activity_labels[predicted.item()]
            logger.info(f"Predicted activity label: {predicted_label}")

        # Return the classification result
        return {
            'statusCode': 200,
            'body': json.dumps({'predicted_activity': predicted_label})
        }
    except Exception as e:
        # Handle any errors that occur
        logger.error(e)
        logger.error('Error getting Object {} from Bucket {}. Make sure they exist and bucket is in the same region '
                     'as this function.'.format(object_key, bucket))
        error_response = {
            "error": str(e)
        }
        return {
            'statusCode': 500,
            'body': json.dumps(error_response)
        }
