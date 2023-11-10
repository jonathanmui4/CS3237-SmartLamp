FROM public.ecr.aws/lambda/python:3.8

COPY requirements.txt ./
RUN #python3.8 -m pip install -r requirements.txt -t .
RUN python3.8 -m pip install torch torchvision Pillow boto3


COPY app.py ./
COPY reading.jpg ./
COPY activity_detection.pt /opt/ml/activity_detection.pt

CMD ["app.lambda_handler"]
