import requests

# response = requests.post("http://localhost:5000/predict", files={'file': open('./reading.jpg', 'rb')})
response = requests.post("http://52.74.67.223/predict", files={'file': open('./reading.jpg', 'rb')})

print(response.text)