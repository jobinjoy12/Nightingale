from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Global variables to store sensor data
mpu_data = {"acc_x": 0, "acc_y": 0, "acc_z": 0}
temperature_data = {"temperature": 0}
pulse_data = {"pulse": 0}

# Endpoint to receive data from ESP32 devices
@app.route('/upload_data', methods=['POST'])
def upload_data():
    sensor_type = request.form['sensor']
    
    if sensor_type == 'mpu6050':
        global mpu_data
        mpu_data = {
            "acc_x": request.form['acc_x'],
            "acc_y": request.form['acc_y'],
            "acc_z": request.form['acc_z']
        }
    elif sensor_type == 'temperature':
        global temperature_data
        temperature_data = {
            "temperature": request.form['temperature']
        }
    elif sensor_type == 'pulse':
        global pulse_data
        pulse_data = {
            "pulse": request.form['pulse']
        }
    
    return "Data received", 200

# Pages to display real-time data
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/mpu6050')
def mpu6050_page():
    return render_template('mpu6050.html')

@app.route('/temperature')
def temperature_page():
    return render_template('temperature.html')

@app.route('/pulse')
def pulse_page():
    return render_template('pulse.html')

# API endpoints to serve the latest sensor data
@app.route('/get_mpu_data')
def get_mpu_data():
    return jsonify(mpu_data)

@app.route('/get_temperature_data')
def get_temperature_data():
    return jsonify(temperature_data)

@app.route('/get_pulse_data')
def get_pulse_data():
    return jsonify(pulse_data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
