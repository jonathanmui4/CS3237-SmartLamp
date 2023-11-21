import json
import boto3
import base64
from datetime import datetime

s3 = boto3.client('s3')

def is_valid_image(data):
    # JPEG file signature (magic number)
    jpeg_signature = b'\xFF\xD8\xFF\xE0'

    try:
        # Check if the first 4 bytes match the JPEG signature
        if data[:4] == jpeg_signature:
            return True
        else:
            print('Invalid image: Not a JPEG file')
            return False
    except Exception as e:
        print(f'Error checking image format: {str(e)}')
        return False


def lambda_handler(event, context):
    try:
        # Extract the message payload from the MQTT event
        mqtt_payload = event
        
        # Get the base64 encoded message from the payload
        encoded_message = mqtt_payload['message']

        # Decode the base64 message to bytes
        decoded_bytes = base64.b64decode(encoded_message)
        print("Decoded Message received.")

        # Check if the decoded bytes represent a valid image
        if not is_valid_image(decoded_bytes):
            return {
                'statusCode': 400,
                'body': 'Invalid image format'
            }

        # Create a unique filename based on the current timestamp
        current_time = datetime.now().strftime('%Y-%m-%d-%H-%M-%S')
        file_name = f'{current_time}.jpg'
        
        try:
            # Upload the image to an S3 bucket
            s3_bucket_name = 'cs3237lamp'  # Replace with your S3 bucket name
            s3.put_object(Bucket=s3_bucket_name, Key=file_name, Body=decoded_bytes)
            print(f'Image "{file_name}" successfully uploaded to S3')
            return {
                'statusCode': 200,
                'body': f'Image "{file_name}" successfully uploaded to S3'
            }
        except Exception as e:
            print(f'Error: {str(e)}')
            return {
                'statusCode': 500,
                'body': f'Error: {str(e)}'
            }
    except Exception as e:
        return {
            'statusCode': 500,
            'body': f'Error: {str(e)}'
        }
