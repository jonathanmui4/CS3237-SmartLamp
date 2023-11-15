from flask import Flask, request, jsonify
from .torch_utils import transform_image, get_activity_prediction, get_posture_prediction

app = Flask(__name__)

ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'JPG'}


def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


@app.route("/")
def index():
    return f"Hello world<br>hehe"


@app.route('/predict_activity', methods=['POST'])
def predict_activity():
    if request.method == 'POST':
        file = request.files.get('file')
        if file is None or file.filename == '':
            return jsonify({'error': "no file"})
        if not allowed_file(file.filename):
            return jsonify({'error': "wrong file format"})
        try:
            img_bytes = file.read()
            tensor = transform_image(img_bytes)
            prediction = get_activity_prediction(tensor)
            data = {"predicted_activity": prediction}
            return jsonify(data)
        except Exception as e:
            error_response = {
                "error": str(e)
            }
            return jsonify(error_response)
    return jsonify({'result': 1})


@app.route('/predict_posture', methods=['POST'])
def predict_posture():
    if request.method == 'POST':
        file = request.files.get('file')
        if file is None or file.filename == '':
            return jsonify({'error': "no file"})
        if not allowed_file(file.filename):
            return jsonify({'error': "wrong file format"})
        try:
            img_bytes = file.read()
            tensor = transform_image(img_bytes)
            prediction = get_posture_prediction(tensor)
            data = {"predicted_posture": prediction}
            return jsonify(data)
        except Exception as e:
            error_response = {
                "error": str(e)
            }
            return jsonify(error_response)
    return jsonify({'result': 1})


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000)
