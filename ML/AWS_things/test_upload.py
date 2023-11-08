import logging
import urllib.parse
import json
import boto3

# Initialize the S3 client
s3 = boto3.client('s3')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

def lambda_handler(event, context):
    try:
        # Get the S3 bucket and object name from the event.
        bucket = event['Records'][0]['s3']['bucket']['name']
        object_key = urllib.parse.unquote_plus(event['Records'][0]['s3']['object']['key'], encoding='utf-8')
        # object_key = event['Records'][0]['s3']['object']['key']

        # Log the name of the file using logger.
        logger.info(f'File uploaded to S3 bucket: {bucket}, object key: {object_key}')

        try:
            response = s3.get_object(Bucket=bucket, Key=object_key)
            logger.info("Content-Type: " + response['ContentType'])
            return response['ContentType']
        except Exception as e:
            logger.error(e)
            logger.error('Error getting Object {} from Bucket {}. Make sure they exist and bucket is in the same region as this function.'.format(object_key, bucket))
            raise e
        '''
        return {
            'statusCode': 200,
            'body': json.dumps('File uploaded successfully.')
        }
        '''
    except Exception as e:
        # Handle any errors that occur
        logging.error(f'Error uploading file: {e}')
        return {
            'statusCode': 500,
            'body': json.dumps(f'Error uploading file: {e}')
        }
