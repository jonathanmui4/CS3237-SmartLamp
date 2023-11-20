FROM public.ecr.aws/lambda/python:3.8

# Install Python dependencies (uncomment if you have a requirements.txt)
COPY requirements.txt ./
RUN python3.8 -m pip install -r requirements.txt

# Copy the function code and model weights to the image.
COPY AWSLambdaClassification.py ./

# Copy the Firebase credentials file into the image
COPY FirebaseCredentuals.json ./

# Set the CMD to your handler (could also be done as a parameter override outside of the Dockerfile)
CMD ["AWSLambdaClassification.lambda_handler"]
