import torch
import torch.nn as nn
import torchvision
from torchvision import models, transforms
from torchvision.models import ResNet18_Weights
from PIL import Image
import io

class_labels = ["reading", "not_present", "asleep", "computer_use"]
posture_labels = ["good_posture", "bad_posture"]

model_weight_path = "./app/activity_detection.pth"
model = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)
num_ftrs = model.fc.in_features
model.fc = nn.Linear(num_ftrs, 4)
model.load_state_dict(torch.load(model_weight_path, map_location=torch.device('cpu')))
model.eval()

model1_weight_path = "./app/posture_detection.pth"
model1 = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)
num_ftrs1 = model1.fc.in_features
model1.fc = nn.Linear(num_ftrs1, 2)
model1.load_state_dict(torch.load(model1_weight_path, map_location=torch.device('cpu')))
model1.eval()


# Image transforms
def transform_image(image_bytes):
    transform = transforms.Compose([
        transforms.Resize((128, 128)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])
    image = Image.open(io.BytesIO(image_bytes))
    return transform(image).unsqueeze(0)


def get_activity_prediction(image_tensor):
    with torch.no_grad():
        outputs = model(image_tensor)
        _, predicted = torch.max(outputs, 1)
        predicted_label = class_labels[predicted.item()]
        print(f" Predicted activity label: {predicted_label}")
        return predicted_label


def get_posture_prediction(image_tensor):
    with torch.no_grad():
        outputs = model1(image_tensor)
        _, predicted = torch.max(outputs, 1)
        predicted_label = posture_labels[predicted.item()]
        print(f" Predicted posture label: {predicted_label}")
        return predicted_label
