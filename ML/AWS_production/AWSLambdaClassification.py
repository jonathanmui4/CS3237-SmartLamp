import logging
import json
import boto3
import io
from PIL import Image
import requests
import firebase_admin
from firebase_admin import credentials, firestore


bot_token = "BOTTOKEN"
chat_id = "CHATID"


# Initialize Firebase with your service account credentials
cred = credentials.Certificate("certificate")
firebase_admin.initialize_app(cred)

# Access the Firestore database
db = firestore.client()

# Initialize the S3 client
s3 = boto3.client('s3')
iot = boto3.client('iot-data')

# Initialize the logger
logger = logging.getLogger()
logger.setLevel(logging.INFO)

# Specify the S3 bucket and model file key
s3_bucket = 'cs3237lamp'

# Function to load the model (this should match your model's architecture)
def lambda_handler(event, context):
    try:
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
                        predict_activity = ""
                        predict_posture = ""

                        image = Image.open(io.BytesIO(response.content))
                        try:
                            # Save the opened image to the /tmp directory
                            image_path = '/tmp/1.jpeg'
                            image.save(image_path)

                            logger.info("Image saved to /tmp directory.")

                            # Send a POST request to the prediction endpoint with the saved image file
                            with open(image_path, 'rb') as image_file:
                                response1 = requests.post("http://52.74.67.223/predict_activity", files={'file': image_file})

                            with open(image_path, 'rb') as image_file:
                                response2 = requests.post("http://52.74.67.223/predict_posture", files={'file': image_file})



                            # Do something with the response
                            predict_activity = response1.json()
                            predict_posture = response2.json()

                            logger.info("Successfully processed image and received response.")

                        except Exception as e:
                            logger.error(f"Error: {e}")

                        # Get the predicted activity and posture

                        predict_activity_label = predict_activity['predicted_activity']
                        
                        predict_posture_label = predict_posture['predicted_posture']

                        logger.info(f"Activity: {predict_activity_label}, Posture: {predict_posture_label}")

                        response_payload_pos = {
                            "message": predict_posture_label
                        }

                        response_payload_act = {
                            "message": predict_activity_label,
                        }

                        response_message_pos = json.dumps(response_payload_pos)
                        response_message_act = json.dumps(response_payload_act)
                        
                        if predict_posture_label == "bad_posture" and (predict_activity_label != "not_present" or predict_activity_label != "asleep"):
                            bot_message = "Adjust your posture"
                            send_text = 'https://api.telegram.org/bot' + bot_token + '/sendMessage?chat_id=' + chat_id + \
                '&parse_mode=HTML&text=' + bot_message
                            response = requests.get(send_text)


                        try:
                            iot.publish(
                                topic='laptop/activity',  # Replace with your MQTT topic
                                qos=1,  # Quality of Service
                                payload=response_message_act,
                                retain=True
                            )
                            print("Message successfully published to the MQTT topic.")
                        except Exception as e:
                            print(f"Error publishing message to MQTT: {str(e)}")

                        try:
                            iot.publish(
                                topic='laptop/posture',  # Replace with your MQTT topic
                                qos=1,  # Quality of Service
                                payload=response_message_pos,
                                retain=True
                            )
                            print("Message successfully published to the MQTT topic.")
                        except Exception as e:
                            print(f"Error publishing message to MQTT: {str(e)}")
                
                        # Define the data for the new document
                        new_data = {
                            "dateTime": firestore.SERVER_TIMESTAMP,
                            "activity": predict_activity_label,
                            "posture": predict_posture_label
                        }

                        # Add a new document to the "LampData" collection
                        db.collection("LampData").add(new_data)

                        # Return the classification result
                        return {
                            'statusCode': 200,
                            'body': json.dumps({"activity": predict_activity_label,
                            "posture": predict_posture_label})
                        }
        else:
            logger.warning("No S3 event records found in the input event.")
    except Exception as e:
        logger.error(f"An error occurred: {str(e)}")
        # Handle the error or return an appropriate response

    return {
        'statusCode': 200,
        'body': json.dumps('Image classification completed.')
    }

