import os
import boto3
import requests
import numpy as np
import io
import json
import torch
import torch.nn as nn
from torchvision import models, transforms
from torchvision.models import ResNet18_Weights
import logging
from PIL import Image

# Initialize the S3 client
s3 = boto3.client('s3')

# Initialize the IoT client
iot = boto3.client('iot-data')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

# Constants
IMG_SIZE = (128, 128)
activity_labels = ["reading", "not_present", "asleep", "computer_use"]
posture_labels = ['good_posture', 'bad_posture']

# Define image transforms
transform = transforms.Compose([
        transforms.Resize(IMG_SIZE),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])

# Function to load and transform an image
def load_image(image_bytes):
    image = Image.open(io.BytesIO(image_bytes))
    return transform(image)

# action -> 0 for activity classification, 1 for posture
def load_model(action):
    model_reloaded = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)
    num_ftrs = model_reloaded.fc.in_features
    if (action == 0):
        model_reloaded.fc = nn.Linear(num_ftrs, 4)  # Match the number of classes
        model_reloaded.load_state_dict(torch.load('activity_detection.pth', map_location=torch.device("cpu")))
    else:
        model_reloaded.fc = nn.Linear(num_ftrs, 2)  # Match the number of classes
        model_reloaded.load_state_dict(torch.load('posture_detection.pth', map_location=torch.device("cpu")))
    model_reloaded.eval()
    return model_reloaded
    
def lambda_handler(event, context):
    try:
        # Check for S3 event records
        if 'Records' in event:
            for record in event['Records']:
                # Check if the event is "ObjectCreated:Put"
                if record['eventName'] == 'ObjectCreated:Put':
                    # Retrieve the S3 bucket and object key
                    s3_bucket = record['s3']['bucket']['name']
                    s3_object_key = record['s3']['object']['key']

                    # Get the latest added (most recently modified) object in the S3 bucket
                    try:
                        objs = s3.list_objects_v2(Bucket=s3_bucket)['Contents']
                        if objs:
                            get_last_modified = lambda obj: int(obj['LastModified'].strftime('%s'))
                            latest_object = sorted(objs, key=get_last_modified, reverse=True)[0]
                            s3_object_key = latest_object['Key']
                        else:
                            logger.info("S3 bucket is empty.")
                            return {
                                'statusCode': 200,
                                'body': json.dumps('S3 bucket is empty.')
                            }
                    except Exception as e:
                        logger.error(f"Error retrieving S3 objects: {str(e)}")
                        return {
                            'statusCode': 500,
                            'body': json.dumps('Error retrieving S3 objects.')
                        }

                    # Construct the S3 URL for the image
                    input_image = f'https://{s3_bucket}.s3.ap-southeast-1.amazonaws.com/{s3_object_key}'  # Updated S3 URL
                    logger.info(f"Processing image: {input_image}")

                    # Load and preprocess the image
                    response = requests.get(input_image)
                    if response.status_code == 200:
                        image = load_image(io.BytesIO(response.content))
                        image = image.unsqueeze(0)  # Add batch dimension

                        # Load Activity Classification Model
                        model_activity = load_model(0)

                        # Perform inference for Activity Classification
                        with torch.no_grad():
                            outputs = model_activity(image)
                            _, predicted = torch.max(outputs, 1)
                            predicted_activity = activity_labels[predicted.item()]

                        logger.info(f"Predicted label (activity): {predicted_activity}")

                        # Load Posture Classification Model
                        model_posture = load_model(1)

                        # Perform inference for Posture Classification
                        with torch.no_grad():
                            outputs = model_posture(image)
                            _, predicted = torch.max(outputs, 1)
                            predicted_posture = posture_labels[predicted.item()]

                        logger.info(f"Predicted label (posture): {predicted_posture}")

                        # Perform image classification on the new image
                        result = {
                            'predicted_activity': predicted_activity,
                            'predicted_posture': predicted_posture,
                        }

                        logger.info(f"Predictions: {result}")

                        # Publish a response message to an MQTT topic
                        response_payload = {
                            "response": result
                        }

                        response_message = json.dumps(response_payload)

                        try:
                            iot.publish(
                                topic='esp32/sub',  # Replace with your MQTT topic
                                qos=1,  # Quality of Service
                                payload=response_message
                            )
                            logger.info("Message successfully published to the MQTT topic.")
                        except Exception as e:
                            logger.error(f"Error publishing message to MQTT: {str(e)}")
                    else:
                        logger.error(f"Failed to download the image from S3. Status code: {response.status_code}")
                        # Handle the error or return an appropriate response
        else:
            logger.warning("No S3 event records found in the input event.")
    except Exception as e:
        logger.error(f"An error occurred: {str(e)}")
        # Handle the error or return an appropriate response

    return {
        'statusCode': 200,
        'body': json.dumps('Image classification completed.')
    }
