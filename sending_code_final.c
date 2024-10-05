#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "Hackathon";
const char* password = "Intel@Kp6!3t";

// Central server URL
const char* serverUrl = "http://172.168.72.89:5000/upload_data"; // Use your Flask server IP
const char* apiKey = "jobin"; // Replace with your actual API key

Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize MPU6050
    if (!mpu.begin()) {
        Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
        while (1) {
            delay(10);
        }
    }

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Build the data string
        String data = "sensor=mpu6050&acc_x=" + String(a.acceleration.x) + 
                      "&acc_y=" + String(a.acceleration.y) + 
                      "&acc_z=" + String(a.acceleration.z) +
                      "&api_key=" + apiKey; // Include the API key

        // Send the POST request
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        int httpResponseCode = http.POST(data);

        // Debugging information
        Serial.print("Sending data: ");
        Serial.println(data);
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Response: " + response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    } else {
        Serial.println("WiFi not connected, trying to reconnect...");
        WiFi.reconnect();
    }

    delay(5000); // Send data every 5 seconds
}
