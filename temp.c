#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Initialize the AHT10 sensor
Adafruit_AHTX0 aht;

// Wi-Fi credentials
const char* ssid = "Hackathon";          // Replace with your WiFi SSID
const char* password = "Intel@Kp6!3t";  // Replace with your WiFi password

// Flask server URL
const char* serverUrl = "http://172.168.72.89:5000/upload_data";  // Replace with your Flask server IP and port

// Optional: API Key for authentication (set on Flask server)
const char* apiKey = "jobin";  // Replace with your actual API key

// Timing variables
unsigned long previousMillis = 0;
const long interval = 5000;  // Interval to send data (milliseconds)

// Retry mechanism variables
const int maxRetries = 3;
const long retryDelay = 2000;  // Delay between retries (milliseconds)

void setup() {
  // Start serial communication
  Serial.begin(115200);
  delay(1000);  // Allow time for serial monitor to initialize

  // Initialize the AHT10 sensor
  if (!aht.begin()) {
    Serial.println("Could not find AHT10 sensor!");
    while (1) {
      delay(10);  // Halt if sensor is not found
    }
  }
  Serial.println("AHT10 sensor initialized.");

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to send data
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Ensure Wi-Fi is connected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      connectToWiFi();
      // If reconnection fails, skip sending data this cycle
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnection failed. Will retry in the next cycle.");
        return;
      }
    }

    // Read temperature data
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);  // Populate temp with temperature event data

    float temperature = temp.temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Send data to Flask server with retry mechanism
    sendTemperatureData(temperature);
  }

  // Other non-blocking code can be placed here
}

/**
 * Connects to the specified Wi-Fi network.
 */
void connectToWiFi() {
  Serial.printf("Connecting to WiFi SSID: %s\n", ssid);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Keep trying to connect for 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

/**
 * Sends temperature data to the Flask server with retries.
 * @param temperature The temperature value to send.
 */
void sendTemperatureData(float temperature) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot send data.");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Optional: Include API key in the POST data for authentication
  String postData = "sensor=temperature&temperature=" + String(temperature) + "&api_key=" + String(apiKey);

  bool success = false;
  int attempt = 0;

  while (attempt < maxRetries && !success) {
    Serial.printf("Sending data to server (Attempt %d)...\n", attempt + 1);
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      if (httpResponseCode == 200) {
        Serial.println("Data sent successfully.");
        success = true;
      } else {
        Serial.printf("Server responded with code: %d\n", httpResponseCode);
      }
    } else {
      Serial.printf("Error sending POST request: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    if (!success) {
      attempt++;
      if (attempt < maxRetries) {
        Serial.printf("Retrying in %ld ms...\n", retryDelay);
        delay(retryDelay);
      } else {
        Serial.println("Max retries reached. Failed to send data.");
      }
    }
  }

  http.end();
}
