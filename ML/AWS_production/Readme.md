Deploying Docker image to AWS lambda
===========================

## Step 1
`docker build -t [image-name]:[version] .`
## Step 2
`docker tag [image-name]:[version] [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:[version]`
## Step 3
`docker push docker push [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:latest`


Files in AWS_production

1) AWSImageDecoder.py : Listens to the AWS IOT Core MQTT Topic to receive the encoded base64 bytes from ESP32 Camera to decodes and convert the bytes into a image. Store this image into a S3 Bucket.

2) AWSLambdaClassification.py : Retrieves the latest image from the S3 Bucket and sends an API Request to an AWS Lightsail instance to get the classification results as the response. Send this result to a AWS UIT Core MQTT Topic, Store in a Google Firestore Database and send a notification to Telegram Bot (If bad posture detected).

3) firebaseToExcelConverter.py : Retrieves all the data collected from the Google Firestore Database and converts this data into a Microsoft Excel sheet for ML Training etc...
