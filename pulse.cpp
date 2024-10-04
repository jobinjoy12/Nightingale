#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Hackathon";
const char* password = "Intel@Kp6!3t";
const char* serverUrl = "http://172.168.72.89:5000/upload_data";
const char* apiKey = "jobin"; // API Key

const int PULSE_SENSOR_PIN = 34;
int pulseValue = 0;
int beatsPerMinute = 0;

unsigned long lastReading = 0;
const long readingInterval = 3000;  // 3 seconds

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

void loop() {
  unsigned long currentMillis = millis();
  static int beatCounter = 0;
  static unsigned long lastBeat = 0;
  
  pulseValue = analogRead(PULSE_SENSOR_PIN);

  // Simple beat detection
  if (pulseValue > 2000) { // Adjust this threshold as needed
    if (currentMillis - lastBeat > 300) { // Debounce
      beatCounter++;
      lastBeat = currentMillis;
    }
  }

  if (currentMillis - lastReading >= readingInterval) {
    beatsPerMinute = beatCounter * 20; // 60 seconds / 3 seconds * beatCounter
    beatCounter = 0;

    // Restrict heart rate to 65-160 BPM
    beatsPerMinute = constrain(beatsPerMinute, 65, 160);

    Serial.print("Heart Rate: ");
    Serial.print(beatsPerMinute);
    Serial.println(" BPM");

    // Send data to server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      http.addHeader("X-API-Key", apiKey);  // Add API key to header
      
      String data = "sensor=pulse&pulse=" + String(beatsPerMinute);
      int httpResponseCode = http.POST(data);
      
      if (httpResponseCode > 0) {
        Serial.println("Data sent successfully");
      } else {
        Serial.println("Error sending data: " + String(httpResponseCode));
      }
      
      http.end();
    }

    lastReading = currentMillis;
  }
}
