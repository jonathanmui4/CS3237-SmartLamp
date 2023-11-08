FROM public.ecr.aws/lambda/python:3.8

# Install Python dependencies (uncomment if you have a requirements.txt)
COPY requirements.txt .
RUN python3.8 -m pip install -r requirements.txt

# Copy the function code to the image.
COPY test_upload.py ${LAMBDA_TASK_ROOT}

# Set the CMD to your handler (could also be done as a parameter override outside of the Dockerfile)
CMD ["test_upload.lambda_handler"]
