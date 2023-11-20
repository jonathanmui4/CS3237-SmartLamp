Deploying Docker image to AWS lambda
===========================

## Step 1
`docker build -t [image-name]:[version] .`
## Step 2
`docker tag [image-name]:[version] [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:[version]`
## Step 3
`docker push docker push [user-id].dkr.ecr.[region].amazonaws.com/[repo-name]:latest`


## Files in AWS_production

1. **AWSImageDecoder.py**
    - Listens to the AWS IoT Core MQTT Topic to receive encoded base64 bytes from ESP32 Camera.
    - Decodes and converts the bytes into an image.
    - Stores the image into an S3 Bucket.

2. **AWSLambdaClassification.py**
    - Retrieves the latest image from the S3 Bucket.
    - Sends an API Request to an AWS Lightsail instance to get classification results.
    - Sends the result to an AWS IoT Core MQTT Topic.
    - Stores the result in a Google Firestore Database.
    - Sends a notification to Telegram Bot if bad posture is detected.

3. **firebaseToExcelConverter.py**
    - Retrieves all data collected from the Google Firestore Database.
    - Converts this data into a Microsoft Excel sheet for ML Training, etc.

4. **Dockerfile**
    - Contains the dependencies and instructions to build the Docker image.

5. **requirements.txt**
    - Contains the libraries required to build the Docker image.
