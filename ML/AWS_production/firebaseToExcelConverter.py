import firebase_admin
from firebase_admin import credentials, firestore
import openpyxl
from datetime import datetime


# Initialize Firebase with your service account credentials
cred = credentials.Certificate("FirebaseCertificate.json")
firebase_admin.initialize_app(cred)

# Access the Firestore database
db = firestore.client()

# Retrieve data from Firestore
lamp_data_ref = db.collection("LampData")
docs = lamp_data_ref.get()

# Create a new Excel workbook and select the active sheet
workbook = openpyxl.Workbook()
sheet = workbook.active

headers = ["DateTime", "Activity", "Posture"]
sheet.append(headers)

# Add data from Firestore to the Excel sheet
for doc in docs:
    data = doc.to_dict()
    
    # Convert Firestore timestamp to Python datetime without timezone
    datetime_value = data["dateTime"].replace(tzinfo=None)
    
    # Add row to Excel sheet
    row = [datetime_value, data["activity"], data["posture"]]
    sheet.append(row)

# Save the Excel file
workbook.save("output.xlsx")