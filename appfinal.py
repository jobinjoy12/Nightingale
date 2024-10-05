from flask import Flask, request, jsonify, render_template
from datetime import datetime
import math
from flask import Flask, render_template, jsonify
import dash
import dash_core_components as dcc
import dash_html_components as html
import plotly.graph_objs as go
import pandas as pd
from dash import dcc, html
import os

app = Flask(__name__)

# Global variables to store sensor data
mpu_data = {"acc_x": 0, "acc_y": 0, "acc_z": 0}
temperature_data = {"temperature": 0}
pulse_data = {"pulse": 0}

def calculate_magnitude(acc_x, acc_y, acc_z):
    """Calculate the magnitude of acceleration considering gravity."""
    acc_x = float(acc_x)
    acc_y = float(acc_y)
    acc_z = float(acc_z)
    
    return math.sqrt(acc_x**2 + acc_y**2 + (acc_z + 9.81)**2)  # Adjust for gravity on the z-axis

def is_data_normal(mpu, temp, pulse):
    # Refine accelerometer threshold (allowing small movements)
    acc_threshold = 2.0  # increased threshold for normal small movements
    acc_x, acc_y, acc_z = float(mpu['acc_x']), float(mpu['acc_y']), float(mpu['acc_z'])
    
    # Label invalid temperature readings as "invalid"
    temperature = float(temp['temperature'])
    if temperature == 0:
        return "invalid"
    elif temperature > 37.5:
        return "abnormal"
    
    # Label invalid pulse readings as "invalid"
    pulse_value = float(pulse['pulse'])
    if pulse_value == 0:
        return "invalid"
    elif pulse_value > 100:
        return "abnormal"
    
    # Calculate magnitude of acceleration
    magnitude = calculate_magnitude(acc_x, acc_y, acc_z)

    # Classify activity based on magnitude
    if magnitude < 10:  # Assuming minimal movement (near gravity)
        return "sitting"
    elif 10 <= magnitude <= 15:  # Moderate movement, walking likely
        return "walking"
    elif magnitude > 15:  # High acceleration likely indicates a fall
        return "falling"
    
    return "normal"

import subprocess

def process_sensor_data():
    # Call the C++ program that processes data in parallel
    subprocess.run(['g++', '-fsycl', 'process_data.cpp', '-o', 'process_data'])
    subprocess.run(['./process_data'])


# Function to log data in CSV format
def log_data(sensor_type):
    global mpu_data, temperature_data, pulse_data
    timestamp = datetime.now().isoformat()
    label = is_data_normal(mpu_data, temperature_data, pulse_data)
    log_entry = f"{timestamp},{mpu_data['acc_x']},{mpu_data['acc_y']},{mpu_data['acc_z']},{temperature_data['temperature']},{pulse_data['pulse']},{label}\n"
    
    with open("sensor_data_log.csv", "a") as f:
        f.write(log_entry)
        
# Initialize Dash app within Flask
dash_app = dash.Dash(__name__, server=app, routes_pathname_prefix='/dashboard/')

# Path to the CSV file (adjust if needed)
csv_file_path = os.path.join(os.getcwd(), 'sensor_data_log.csv')        

def read_sensor_data():
    if os.path.exists(csv_file_path):
        data = pd.read_csv(csv_file_path)
        # Filter out rows where label is 'invalid'
        data = data[data['label'] != 'invalid']
        return data
    else:
        # Return an empty dataframe if the file doesn't exist yet
        return pd.DataFrame(columns=['timestamp', 'acc_x', 'acc_y', 'acc_z', 'temperature', 'pulse', 'label'])

# Dash app layout
dash_app.layout = html.Div(children=[
    html.H1(children='Patient Health Monitoring'),

    dcc.Graph(
        id='pulse-graph',
        figure={
            'data': [
                go.Scatter(
                    x=read_sensor_data()['timestamp'],
                    y=read_sensor_data()['pulse'],
                    mode='lines',
                    name='Pulse Rate'
                )
            ],
            'layout': {
                'title': 'Real-time Pulse Rate'
            }
        }
    ),

    dcc.Graph(
        id='temperature-graph',
        figure={
            'data': [
                go.Scatter(
                    x=read_sensor_data()['timestamp'],
                    y=read_sensor_data()['temperature'],
                    mode='lines',
                    name='Body Temperature'
                )
            ],
            'layout': {
                'title': 'Real-time Body Temperature'
            }
        }
    ),
])


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
    
    log_data(sensor_type)
    return "Data received and logged", 200


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

@app.route('/get_patient_data')
def get_patient_data():
    # Example data for testing
    patient_data = {
        "name": "John Doe",
        "room_no": "302",
        "pulse": pulse_data['pulse'],  # Real pulse data from ESP32
        "temperature": temperature_data['temperature'],  # Real temperature data from ESP32
        "fall_status": "Fall Detected" if calculate_magnitude(mpu_data['acc_x'], mpu_data['acc_y'], mpu_data['acc_z']) > 15 else "No Fall Detected"
    }
    return jsonify(patient_data)

@app.route('/get_temperature_data')
def get_temperature_data():
    return jsonify(temperature_data)

@app.route('/get_pulse_data')
def get_pulse_data():
    return jsonify(pulse_data)

@app.route('/get_data')
def get_data():
    data = read_sensor_data()  # Read the latest data from the CSV
    return jsonify(data.to_dict())

if __name__ == '__main__':
    # Create or clear the log file when the app starts
    with open("sensor_data_log.csv", "w") as f:
        f.write("timestamp,acc_x,acc_y,acc_z,temperature,pulse,label\n")
    app.run(debug=True, host='0.0.0.0')
 