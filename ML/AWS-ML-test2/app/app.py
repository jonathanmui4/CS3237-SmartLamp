import torch
import torchvision
from torchvision import transforms
import torch.nn as nn
import base64
import json
import numpy as np
import logging
import boto3
import gzip
import shutil

from PIL import Image
from io import BytesIO

s3_client = boto3.client('s3')
bucket_name = 'cs3237-smartlamp-test'
compressed_model_file_name = 'activity_detection.pth.gz'
decompressed_model_file_path = '/tmp/activity_detection.pth'

image_file ='./reading.jpg'

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

transform = transforms.Compose([
        transforms.Resize((128, 128)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])

class_labels = ["reading", "not_present", "asleep", "computer_use"]

class CNN(nn.Module):
    def __init__(self):
        super(CNN, self).__init__()
        self.conv1 = nn.Conv2d(in_channels=3, out_channels=16, kernel_size=3, stride=1, padding=1)
        self.conv2 = nn.Conv2d(in_channels=16, out_channels=32, kernel_size=3, stride=1, padding=1)
        self.conv3 = nn.Conv2d(in_channels=32, out_channels=64, kernel_size=3, stride=1, padding=1)
        self.fc1 = nn.Linear(in_features=64*16*16, out_features=128)  # assuming the input image size is 128x128
        self.fc2 = nn.Linear(in_features=128, out_features=4)  # 4 classes

    def forward(self, x):
        x = torch.relu(self.conv1(x))
        x = torch.max_pool2d(x, kernel_size=2, stride=2)
        x = torch.relu(self.conv2(x))
        x = torch.max_pool2d(x, kernel_size=2, stride=2)
        x = torch.relu(self.conv3(x))
        x = torch.max_pool2d(x, kernel_size=2, stride=2)
        x = x.view(x.size(0), -1)  # flatten the tensor
        x = torch.relu(self.fc1(x))
        x = self.fc2(x)
        return x

def load_image(image_path):
    logger.info("loading image")
    image = Image.open(image_path).convert('RGB')
    image = transform(image)
    image = image.unsqueeze(0)  # Add batch dimension
    return image

def lambda_handler(event, context):
    logger.info("Entered lambda")

    image = load_image(image_file)
    logger.info("Loaded image")

    # Download the model from S3
    s3_client.download_file(bucket_name, compressed_model_file_name, '/tmp/activity_detection.pth.gz')
    logger.info("Downloaded model from s3")

    # Decompress the model
    with gzip.open('/tmp/activity_detection.pth.gz', 'rb') as f_in:
        with open(decompressed_model_file_path, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
    logger.info("Unzip file")

    # Load the model
    # Instantiate the model
    model = CNN()
    model.load_state_dict(torch.load(decompressed_model_file_path))
    # model = torch.jit.load(decompressed_model_file_path, map_location=torch.device('cpu'))
    logger.info("Loaded model")

    model.eval()

    with torch.no_grad():
        logger.info("Classifying image")
        outputs = model(image)
        logger.info("Classified image")
        _, predicted = torch.max(outputs, 1)
        predicted_label = class_labels[predicted.item()]
        print(f"Image: {image_file}, Predicted class label: {predicted_label}")

    return {
        'statusCode': 200,
        'body': json.dumps(
            {
                "predicted_label": predicted_label,
            }
        )
    }
