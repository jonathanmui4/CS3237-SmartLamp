import json
import requests
import boto3
import logging
import urllib.parse

# Initialize the S3 client
s3 = boto3.client('s3')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)



def lambda_handler(event, context):
    try:
        # Get the S3 bucket and object name from the event.
        bucket = event['Records'][0]['s3']['bucket']['name']
        file_key = event['Records'][0]['s3']['object']['key']
        object_key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')

        # Log the name of the file using logger.
        logger.info(f'File uploaded to S3 bucket: {bucket}, object key: {object_key}')

        # Download the image file from S3 to /tmp directory
        download_path = f'/tmp/{file_key}'
        logger.info(f'Downloading {file_key} from bucket {bucket} to {download_path}')
        s3.download_file(bucket, file_key, download_path)
        logger.info(f'Successfully downloaded {file_key} to {download_path}')

        try:
            with open(download_path, 'rb') as file:
                # Prepare the file for sending
                files = {'file': open(download_path, 'rb')}

            # Make the POST request to your model
            response = requests.post("http://52.74.67.223/predict", files=files)

            # Process the response
            # Example: print it or send it somewhere
            logger.info(response.text)

            # Return the classification result
            return {
                'statusCode': 200,
                'body': json.dumps(response)
            }

        except IOError as e:
            logger.error(f'Error in reading file {download_path}: {e}')
            raise e

    except Exception as e:
        # Handle any errors that occur
        error_response = {
            "error": str(e)
        }
        return {
            'statusCode': 500,
            'body': json.dumps(error_response)
        }