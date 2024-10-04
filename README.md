# Nightingale - Nursing Assistance System

Nightingale is a nursing assistance system designed to help healthcare professionals monitor patients' vital signs remotely using IoT devices. It is particularly useful in quarantine or isolation scenarios where direct contact is not possible, allowing nurses to monitor the condition of patients from a distance.

## Features

1. **Pulse Monitoring**: 
   - A pulse sensor records real-time pulse readings.
   - The data is sent to a central web server where nurses can view and assess the patient's heart rate continuously.

2. **Fall Detection**: 
   - An MPU6050 sensor monitors 3-axis accelerometer and gyroscope data to detect any falls.
   - When a fall is detected, an alert is sent to the nurses to take immediate action.

3. **Temperature Monitoring**: 
   - A temperature sensor continuously tracks the patient's body temperature.
   - The readings are sent regularly to the web server to ensure the patient remains within a healthy temperature range.

## System Architecture

- **ESP32 Wi-Fi Module**: All sensors are connected to an ESP32 module, which transmits the data to a common web server.
- **Web Server**: Nurses access the web server to view live data from all sensors, allowing them to monitor patients in real time.
  
## Use Cases

- **Quarantine and Isolation**: The Nightingale system is especially beneficial in scenarios where patients are in isolation due to contagious diseases. It minimizes the need for physical contact while ensuring the patients' health is closely monitored.
- **Remote Monitoring**: Allows for the continuous assessment of critical vitals like heart rate, body temperature, and fall detection without the need for constant bedside monitoring.

## Hardware Components

- **Pulse Sensor**
- **MPU6050 Accelerometer & Gyroscope Sensor**
- **Temperature Sensor**
- **ESP32 Wi-Fi Module**

## How It Works

1. **Pulse Sensor**: Sends real-time pulse data to the ESP32, which is then transmitted to the central server for monitoring.
2. **Fall Detection**: The MPU6050 sensor constantly monitors the 3-axis accelerometer and gyroscope data. In the event of a fall, the system detects the sudden changes in these values and sends an alert to the nurses.
3. **Temperature Sensor**: Regularly sends temperature readings to the ESP32, ensuring continuous updates on the patient's condition.

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/nightingale.git
