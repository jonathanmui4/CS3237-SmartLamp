import os
import joblib
import boto3
import requests  # Import the requests library
import numpy as np
import io
import json
import onnxruntime as ort
from PIL import Image


# Initialize the S3 client
s3 = boto3.client('s3')

# Load the trained SVM model
iot = boto3.client('iot-data')

# Constants
IMG_SIZE = (128, 128)
BATCH_SIZE = 32

# Load the model (this assumes the model is included in the Lambda function's deployment package)
ort_session = ort.InferenceSession("activity_detection.onnx")

test_image = "john.jpg"
def test():

    input_image = Image.open(test_image)
    image = input_image.resize((128, 128))
    input_array = np.array(image).astype('float32') / 255
    input_array = np.expand_dims(input_array, axis=0)
    input_array = np.transpose(input_array, (0, 3, 1, 2))
    
    input_name = ort_session.get_inputs()[0].name
    output_name = ort_session.get_outputs()[0].name
    ort_inputs = {input_name: input_array}
    ort_outs = ort_session.run([output_name], ort_inputs)
    predicted_label = np.argmax(ort_outs[0])
    print(predicted_label)
    


test()
