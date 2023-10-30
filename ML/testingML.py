import numpy as np
from PIL import Image
import torch
import torch.nn as nn
from torchvision import models, transforms
from torchvision.models import ResNet18_Weights

# Function to load and transform an image
def load_image(image_path, transform):
    image = Image.open(image_path).convert('RGB')
    image = transform(image)
    image = image.unsqueeze(0)  # Add batch dimension
    return image

def test_activity():
    # Step 1: Recreate the original ResNet18 model
    model_reloaded = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)

    # Step 2: Modify the final layers to match the original changes
    num_ftrs = model_reloaded.fc.in_features
    model_reloaded.fc = nn.Linear(num_ftrs, 4)  # Match the number of classes

    # Step 3: Load the saved state dictionary
    # Map the device to cpu if it is online, else use the mac GPU
    model_reloaded.load_state_dict(torch.load('activity_detection.pth', map_location=torch.device("mps" if torch.backends.mps.is_available() else "cpu")))

    # Step 4: Set the model to evaluation mode if you're doing inference
    model_reloaded.eval()

    # Step 5: Define transformations for the input images
    transform = transforms.Compose([
        transforms.Resize((128, 128)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])

    # Step 7: Paths of the images
    image_paths = ['test_activity/Computer.jpeg', 'test_activity/nobody.jpg', 'test_activity/sleep.jpg', 'test_activity/reading.jpg']
    class_labels = ["reading", "not_present", "asleep", "computer_use"]
    # Images should classify computer_use, not_present, asleep, reading

    # Perform inference
    for path in image_paths:
        image = load_image(path, transform)
        with torch.no_grad():
            outputs = model_reloaded(image)
            _, predicted = torch.max(outputs, 1)
            predicted_label = class_labels[predicted.item()]
            print(f"Image: {path}, Predicted class label: {predicted_label}")

def test_posture():
    # Step 1: Recreate the original ResNet18 model
    model_reloaded = models.resnet18(weights=ResNet18_Weights.IMAGENET1K_V1)

    # Step 2: Modify the final layers to match the original changes
    num_ftrs = model_reloaded.fc.in_features
    model_reloaded.fc = nn.Linear(num_ftrs, 2)  # Match the number of classes

    # Step 3: Load the saved state dictionary
    # Map the device to cpu if it is online, else use the mac GPU
    model_reloaded.load_state_dict(torch.load('posture_detection.pth', map_location=torch.device("mps" if torch.backends.mps.is_available() else "cpu")))

    # Step 4: Set the model to evaluation mode if you're doing inference
    model_reloaded.eval()

    # Step 5: Define transformations for the input images
    transform = transforms.Compose([
        transforms.Resize((128, 128)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])

    # Step 7: Paths of the images
    image_paths = ['test_posture/Good.jpg', 'test_posture/Bad.jpg']
    class_labels = ['good_posture', 'bad_posture']
    # Images should classify good, bad

    # Perform inference
    for path in image_paths:
        image = load_image(path, transform)
        with torch.no_grad():
            outputs = model_reloaded(image)
            _, predicted = torch.max(outputs, 1)
            predicted_label = class_labels[predicted.item()]
            print(f"Image: {path}, Predicted class label: {predicted_label}")

print('----------------------Activity Detection-------------------------')
test_activity()
print('-----------------------------------------------------------------')
print('----------------------Posture Detection-------------------------')
test_posture()

