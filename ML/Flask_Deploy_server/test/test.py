import requests

# response = requests.post("http://localhost:5000/predict_activity", files={'file': open('./reading.jpg', 'rb')})
# response1 = requests.post("http://localhost:5000/predict_posture", files={'file': open('./Bad.JPG', 'rb')})
response = requests.post("http://52.74.67.223/predict_activity", files={'file': open('./reading.jpg', 'rb')})
response1 = requests.post("http://52.74.67.223/predict_posture", files={'file': open('./Bad.JPG', 'rb')})

print(response.text)
# print(response1.text)
