import os
import boto3
import requests
import numpy as np
import io
import json
import onnxruntime as ort
import logging

# Initialize the S3 client
s3 = boto3.client('s3')

# Initialize the IoT client
iot = boto3.client('iot-data')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

# Constants
IMG_SIZE = (128, 128)
BATCH_SIZE = 32

# Load the ONNX model (this assumes the model is included in the Lambda function's deployment package)
ort_session = ort.InferenceSession("activity_detection.onnx")

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
                        image = Image.open(io.BytesIO(response.content))
                        image = image.resize(IMG_SIZE)
                        input_array = np.array(image).astype('float32') / 255
                        input_array = np.expand_dims(input_array, axis=0)
                        input_array = np.transpose(input_array, (0, 3, 1, 2))

                        input_name = ort_session.get_inputs()[0].name
                        output_name = ort_session.get_outputs()[0].name
                        ort_inputs = {input_name: input_array}
                        ort_outs = ort_session.run([output_name], ort_inputs)

                        # Post-process the output
                        predicted_label = np.argmax(ort_outs[0])

                        # Perform image classification on the new image
                        result = {
                            'predicted_label': predicted_label
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
