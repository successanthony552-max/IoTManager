/*
 * ESP32 IoT Controller for Web Dashboard
 * 
 * This code connects to WiFi and communicates with the Flask web application
 * Features:
 * - Sends sensor data (current, voltage, motion detection)
 * - Receives control commands for relay/appliance
 * - HTTP communication with the web dashboard
 * 
 * Hardware connections:
 * - Relay: GPIO 2
 * - Motion sensor (PIR): GPIO 4
 * - Current sensor (analog): GPIO 34
 * - Voltage divider (analog): GPIO 35
 * - Built-in LED: GPIO 2 (same as relay for indication)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// WiFi credentials - Replace with your network details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server configuration - Replace with your Replit app URL
const char* serverURL = "https://your-replit-app-name.replit.app";

// Pin definitions
const int RELAY_PIN = 2;        // Relay control pin
const int MOTION_PIN = 4;       // PIR motion sensor pin
const int CURRENT_PIN = 34;     // Current sensor analog pin
const int VOLTAGE_PIN = 35;     // Voltage divider analog pin
const int LED_PIN = 2;          // Built-in LED (same as relay)

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastCommandCheck = 0;
unsigned long lastWiFiCheck = 0;
const unsigned long SENSOR_INTERVAL = 2000;    // Send sensor data every 2 seconds
const unsigned long COMMAND_INTERVAL = 1000;   // Check for commands every 1 second
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Check WiFi every 10 seconds

// Sensor variables
float currentValue = 0.0;
float voltageValue = 0.0;
bool motionDetected = false;
bool relayState = false;

// WiFi and HTTP client
WiFiClient wifiClient;
HTTPClient http;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 IoT Controller Starting...");
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MOTION_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize relay state
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Setup web server for local status
  setupWebServer();
  
  Serial.println("ESP32 IoT Controller Ready!");
  Serial.println("Local IP: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle web server requests
  server.handleClient();
  
  // Check WiFi connection
  if (currentTime - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      connectToWiFi();
    }
    lastWiFiCheck = currentTime;
  }
  
  // Read sensors and send data
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    readSensors();
    sendSensorData();
    lastSensorRead = currentTime;
  }
  
  // Check for commands
  if (currentTime - lastCommandCheck >= COMMAND_INTERVAL) {
    checkForCommands();
    lastCommandCheck = currentTime;
  }
  
  delay(100); // Small delay for stability
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Blink LED to indicate successful connection
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }
}

void readSensors() {
  // Read motion sensor
  motionDetected = digitalRead(MOTION_PIN);
  
  // Read current sensor (simulate with analog reading)
  int currentRaw = analogRead(CURRENT_PIN);
  currentValue = (currentRaw / 4095.0) * 5.0; // Convert to 0-5A range
  
  // Read voltage sensor (simulate with analog reading)
  int voltageRaw = analogRead(VOLTAGE_PIN);
  voltageValue = (voltageRaw / 4095.0) * 240.0; // Convert to 0-240V range
  
  // Add some realistic variation
  currentValue += (random(-10, 10) / 100.0);
  voltageValue += (random(-5, 5) / 10.0);
  
  // Ensure values are within reasonable bounds
  currentValue = max(0.0, min(5.0, currentValue));
  voltageValue = max(0.0, min(240.0, voltageValue));
  
  Serial.printf("Sensors - Current: %.2fA, Voltage: %.2fV, Motion: %s\n", 
                currentValue, voltageValue, motionDetected ? "YES" : "NO");
}

void sendSensorData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Skipping sensor data transmission.");
    return;
  }
  
  http.begin(wifiClient, String(serverURL) + "/api/sensor-data");
  http.addHeader("Content-Type", "application/json");
  
  // Create JSON payload
  StaticJsonDocument<200> doc;
  doc["current"] = currentValue;
  doc["voltage"] = voltageValue;
  doc["motion_detected"] = motionDetected;
  doc["device_id"] = "ESP32_001";
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Sensor data sent successfully. Response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error sending sensor data. Error code: %d\n", httpResponseCode);
  }
  
  http.end();
}

void checkForCommands() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  http.begin(wifiClient, String(serverURL) + "/api/esp32/commands");
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String response = http.getString();
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      JsonArray commands = doc["commands"];
      
      for (JsonObject command : commands) {
        String cmd = command["command"];
        bool value = command["value"];
        
        Serial.printf("Received command: %s = %s\n", cmd.c_str(), value ? "true" : "false");
        
        if (cmd == "appliance_control") {
          setRelayState(value);
        }
      }
    } else {
      Serial.println("Error parsing command JSON");
    }
  } else if (httpResponseCode != 404) {
    Serial.printf("Error checking commands. Response code: %d\n", httpResponseCode);
  }
  
  http.end();
}

void setRelayState(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  
  Serial.printf("Relay %s\n", state ? "ON" : "OFF");
  
  // Visual feedback
  if (state) {
    // Quick blinks for ON
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }
}

void setupWebServer() {
  // Root endpoint - device status
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 IoT Device</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += ".card{background:white;padding:20px;margin:10px 0;border-radius:10px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
    html += ".status{font-size:24px;font-weight:bold;}";
    html += ".on{color:green;} .off{color:red;}";
    html += "</style></head><body>";
    html += "<h1>ESP32 IoT Device Status</h1>";
    
    html += "<div class='card'>";
    html += "<h3>Connection Status</h3>";
    html += "<p>WiFi: Connected to " + String(ssid) + "</p>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Server: " + String(serverURL) + "</p>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h3>Appliance Control</h3>";
    html += "<p class='status " + String(relayState ? "on" : "off") + "'>";
    html += "Status: " + String(relayState ? "ON" : "OFF") + "</p>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h3>Sensor Readings</h3>";
    html += "<p>Current: " + String(currentValue, 2) + " A</p>";
    html += "<p>Voltage: " + String(voltageValue, 2) + " V</p>";
    html += "<p>Motion: " + String(motionDetected ? "Detected" : "Not Detected") + "</p>";
    html += "</div>";
    
    html += "<div class='card'>";
    html += "<h3>Device Information</h3>";
    html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
    html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p>WiFi RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
    html += "</div>";
    
    html += "<p><a href='/status'>JSON Status</a> | <a href='/'>Refresh</a></p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  });
  
  // JSON status endpoint
  server.on("/status", []() {
    StaticJsonDocument<300> doc;
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["relay_state"] = relayState;
    doc["current"] = currentValue;
    doc["voltage"] = voltageValue;
    doc["motion_detected"] = motionDetected;
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    server.send(200, "application/json", jsonString);
  });
  
  // Manual control endpoints for testing
  server.on("/relay/on", []() {
    setRelayState(true);
    server.send(200, "text/plain", "Relay turned ON");
  });
  
  server.on("/relay/off", []() {
    setRelayState(false);
    server.send(200, "text/plain", "Relay turned OFF");
  });
  
  server.begin();
  Serial.println("Web server started on port 80");
}
