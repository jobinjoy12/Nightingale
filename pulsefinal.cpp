#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Hackathon";
const char* password = "Intel@Kp6!3t";
const char* serverUrl = "http://172.168.72.89:5000/upload_data";
const char* apiKey = "jobin"; // API Key

const int PULSE_SENSOR_PIN = 34; // Pin for future sensor integration
const int FINGER_DETECTION_THRESHOLD = 1000; // Adjust based on real sensor behavior
const int NUM_READINGS = 10; // Number of readings to average for finger detection
const int DETECTION_CONFIDENCE_COUNT = 5; // Number of consecutive times the finger must be detected

// Parameters for simulated heart rate
const int BASE_HEART_RATE = 70; // Starting point for heart rate
const int MAX_FLUCTUATION = 5;  // Maximum change in heart rate per reading
const int MIN_HEART_RATE = 60;
const int MAX_HEART_RATE = 100;

int currentHeartRate = BASE_HEART_RATE;
int detectionConfidence = 0; // To track confidence in finger presence

void setup() {
  Serial.begin(115200);
  pinMode(PULSE_SENSOR_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

// Simulate heart rate fluctuation
int generateRealisticHeartRate() {
  int fluctuation = random(-MAX_FLUCTUATION, MAX_FLUCTUATION + 1);
  currentHeartRate += fluctuation;
  currentHeartRate = constrain(currentHeartRate, MIN_HEART_RATE, MAX_HEART_RATE);
  return currentHeartRate;
}

// Improved finger presence detection by averaging readings
bool isFingerDetected() {
  long totalSensorValue = 0;
  
  // Take multiple readings and average them
  for (int i = 0; i < NUM_READINGS; i++) {
    int sensorValue = analogRead(PULSE_SENSOR_PIN);
    totalSensorValue += sensorValue;
    delay(10); // Short delay between readings
  }
  
  int averageSensorValue = totalSensorValue / NUM_READINGS;
  Serial.print("Average Sensor Value: ");
  Serial.println(averageSensorValue);

  // Check if the average is above the threshold
  if (averageSensorValue > FINGER_DETECTION_THRESHOLD) {
    detectionConfidence++;
    Serial.println("Finger detected.");
  } else {
    detectionConfidence = 0; // Reset confidence if finger not detected
    Serial.println("No finger detected.");
  }

  // Only return true if finger has been consistently detected
  return detectionConfidence >= DETECTION_CONFIDENCE_COUNT;
}

void loop() {
  static unsigned long lastReportTime = 0;
  unsigned long currentTime = millis();

  // Generate and send heart rate every 3 seconds if finger is detected
  if (currentTime - lastReportTime >= 3000) {
    lastReportTime = currentTime;

    if (isFingerDetected()) {
      int heartRate = generateRealisticHeartRate();

      Serial.print("Heart Rate: ");
      Serial.print(heartRate);
      Serial.println(" BPM");

      // Send data to server
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.addHeader("X-API-Key", apiKey);

        String data = "sensor=pulse&pulse=" + String(heartRate);
        int httpResponseCode = http.POST(data);

        if (httpResponseCode > 0) {
          Serial.println("Data sent successfully");
        } else {
          Serial.println("Error sending data: " + String(httpResponseCode));
        }

        http.end();
      }
    }
  }
}
