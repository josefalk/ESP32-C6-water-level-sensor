// ============================================
// user-wifi.cpp
// ============================================
#include "user-wifi.h"
#include "user-led.h"
#include "user-screen.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// Access Point credentials
const char* ap_ssid = "ESP32-Config";
const char* ap_password = "";

// Create web server on port 80
WebServer server(80);
Preferences preferences;

// Variables
int ledState = LOW;
const int RESET_BUTTON_PIN = 0;
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool isAPMode = true;
void setLedAutoMode(bool state);
bool getScreenState();
void setScreenState(bool state);
void handleLed();
void handleScreen();

// Sensor data variables
float currentDistance = 0.0;
float currentPercent = 0.0;
bool sensorDataValid = false;

// Calibration values (defaults)
float fullDistance = 30.0;
float emptyDistance = 200.0;

// Forward declarations
void checkResetButton();
void connectToWiFi(const char* ssid, const char* password);
void startAPMode();
void handleRoot();
void handleScan();
void handleConnect();
void handleLedOn();
void handleLedOff();
void handleStatus();
void handleData();
void handleScreenOn();
void handleScreenOff();
void handleCalibration();
void handleGetCalibration();
void handleNotFound();

float getFullDistance() {
  return fullDistance;
}

float getEmptyDistance() {
  return emptyDistance;
}

void setCalibration(float full, float empty) {
  fullDistance = full;
  emptyDistance = empty;
  preferences.putFloat("fullDist", full);
  preferences.putFloat("emptyDist", empty);
  Serial.printf("Calibration saved: Full=%.1f cm, Empty=%.1f cm\n", full, empty);
}

void initWiFi() {
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  preferences.begin("wifi", false);
  
  // Load calibration values or use defaults
  fullDistance = preferences.getFloat("fullDist", 30.0);
  emptyDistance = preferences.getFloat("emptyDist", 200.0);
  Serial.printf("Loaded calibration: Full=%.1f cm, Empty=%.1f cm\n", fullDistance, emptyDistance);
  
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  
  if (ssid.length() > 0) {
    Serial.println("Attempting to connect to saved WiFi...");
    connectToWiFi(ssid.c_str(), password.c_str());
  } else {
    Serial.println("No saved WiFi credentials. Starting AP mode...");
    startAPMode();
  }
  server.on("/led", handleLed);
  server.on("/screen", handleScreen);
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/status", handleStatus);
  server.on("/data", handleData);
  server.on("/calibration", HTTP_POST, handleCalibration);
  server.on("/calibration", HTTP_GET, handleGetCalibration);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void handleWiFi() {
  server.handleClient();
  checkResetButton();
}

bool isWiFiConnected() {
  return !isAPMode && WiFi.status() == WL_CONNECTED;
}

String getWiFiIP() {
  if (isAPMode) {
    return WiFi.softAPIP().toString();
  } else {
    return WiFi.localIP().toString();
  }
}

bool isLedAutoMode() {
  return ledAutoMode;
}

void updateSensorData(float distance, float percent) {
  currentDistance = distance;
  currentPercent = percent;
  sensorDataValid = true;
}

void checkResetButton() {
  int buttonState = digitalRead(RESET_BUTTON_PIN);
  
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = millis();
    Serial.println("WiFi reset button pressed...");
  } else if (buttonState == LOW && buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;
    
    if (pressDuration >= 3000) {
      Serial.println("Reset button held for 3 seconds!");
      Serial.println("Clearing WiFi credentials and restarting in AP mode...");
      
      ledOn(255, 0, 0);
      delay(500);
      ledOff();
      delay(200);
      ledOn(255, 0, 0);
      delay(500);
      ledOff();
      
      preferences.clear();
      preferences.end();
      
      Serial.println("Restarting...");
      delay(500);
      
      ESP.restart();
    }
  } else if (buttonState == HIGH && buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;
    Serial.printf("Button released after %lu ms\n", pressDuration);
    buttonPressed = false;
  }
}

void connectToWiFi(const char* ssid, const char* password) {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    ledOn(0, 0, 255);
    delay(100);
    ledOff();
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    isAPMode = false;
    
    ledOn(0, 255, 0);
    delay(1000);
    ledOff();
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi");
    Serial.println("Starting AP mode for configuration...");
    startAPMode();
  }
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("=============================");
  Serial.println("AP Mode Started");
  Serial.println("=============================");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Connect to this network and");
  Serial.println("go to http://192.168.4.1");
  Serial.println("=============================");
  
  isAPMode = true;
  
  ledOn(128, 0, 128);
  delay(500);
  ledOff();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; }";
  html += "h2 { color: #666; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }";
  html += "button { padding: 12px 24px; margin: 5px; font-size: 16px; cursor: pointer; border: none; border-radius: 5px; color: white; }";
  html += ".scan { background-color: #2196F3; width: 100%; }";
  html += ".on { background-color: #4CAF50; }";
  html += ".off { background-color: #f44336; }";
  html += "button:hover { opacity: 0.8; }";
  html += ".info { margin-top: 20px; padding: 15px; background: #e3f2fd; border-radius: 5px; }";
  html += ".network-list { margin-top: 20px; }";
  html += ".network-item { padding: 10px; margin: 5px 0; background: #f0f0f0; border-radius: 5px; cursor: pointer; }";
  html += ".network-item:hover { background: #e0e0e0; }";
  html += "input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }";
  html += ".connect-btn { background-color: #4CAF50; width: 100%; }";
  html += ".warning { background: #fff3cd; border-left: 4px solid #ffc107; padding: 10px; margin: 10px 0; }";
  html += ".tank-container { margin: 20px 0; padding: 20px; background: #f8f9fa; border-radius: 10px; }";
  html += ".tank { width: 100%; height: 30px; background: #e0e0e0; border: 2px solid #333; border-radius: 5px; position: relative; overflow: hidden; }";
  html += ".tank-fill { height: 100%; background: linear-gradient(90deg, #4CAF50, #8BC34A); transition: width 0.5s; }";
  html += ".tank-fill.low { background: linear-gradient(90deg, #f44336, #ff5722); }";
  html += ".tank-fill.medium { background: linear-gradient(90deg, #FF9800, #FFC107); }";
  html += ".sensor-data { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 15px; }";
  html += ".sensor-item { padding: 15px; background: white; border-radius: 5px; text-align: center; }";
  html += ".sensor-label { font-size: 12px; color: #666; }";
  html += ".sensor-value { font-size: 24px; font-weight: bold; color: #333; }";
  html += ".calibration-group { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin: 15px 0; }";
  html += ".calibration-input { display: flex; flex-direction: column; }";
  html += ".calibration-input label { font-size: 14px; color: #666; margin-bottom: 5px; }";
  html += ".save-calibration { background-color: #FF9800; width: 100%; }";
  html += "</style>";
  html += "<script>";

  // -----------------------------
  // LED + SCREEN EXTRA JAVASCRIPT
  // -----------------------------
  html += "function led(state){ fetch('/led?state='+state).then(r=>r.text()).then(console.log); }";
  html += "function screen(state){ fetch('/screen?state='+state).then(r=>r.text()).then(console.log); }";

  // Load calibration values
  html += "function loadCalibration() {";
  html += "  fetch('/calibration').then(r => r.json()).then(data => {";
  html += "    document.getElementById('fullDist').value = data.full;";
  html += "    document.getElementById('emptyDist').value = data.empty;";
  html += "  });";
  html += "}";
  html += "window.onload = loadCalibration;";

  // Save calibration
  html += "function saveCalibration() {";
  html += "  const full = parseFloat(document.getElementById('fullDist').value);";
  html += "  const empty = parseFloat(document.getElementById('emptyDist').value);";
  html += "  if (isNaN(full) || isNaN(empty)) { alert('Please enter valid numbers'); return; }";
  html += "  if (full >= empty) { alert('Full distance must be less than empty distance'); return; }";
  html += "  fetch('/calibration', {";
  html += "    method: 'POST',";
  html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
  html += "    body: 'full='+full+'&empty='+empty";
  html += "  }).then(r => r.text()).then(data => {";
  html += "    alert(data); setTimeout(()=>location.reload(), 1000);";
  html += "  });";
  html += "}";

  // Auto-refresh sensor data
  html += "function updateSensorData() {";
  html += "  fetch('/data').then(r => r.json()).then(data => {";
  html += "    if (data.valid) {";
  html += "      document.getElementById('distance').textContent = data.distance.toFixed(1);";
  html += "      document.getElementById('percent').textContent = data.percent.toFixed(0);";
  html += "      const fill = document.getElementById('tankFill');";
  html += "      fill.style.width = data.percent + '%';";
  html += "      fill.className = 'tank-fill';";
  html += "      if (data.percent < 20) fill.className = 'tank-fill low';";
  html += "      else if (data.percent < 70) fill.className = 'tank-fill medium';";
  html += "    }";
  html += "  });";
  html += "}";
  html += "setInterval(updateSensorData, 1000);";
  html += "updateSensorData();";

  // Network scanning
  html += "function scanNetworks() {";
  html += "  document.getElementById('networks').innerHTML = '<p>Scanning...</p>';";
  html += "  fetch('/scan').then(r => r.json()).then(data => {";
  html += "    let html = '';";
  html += "    data.networks.forEach(n => {";
  html += "      html += '<div class=\"network-item\" onclick=\"selectNetwork(\\''+n.ssid+'\\','+n.secure+')\">';";
  html += "      html += '<strong>'+n.ssid+'</strong> ('+n.rssi+' dBm) '+(n.secure ? ' (SECURE) ' : ' (OPEN) ');";
  html += "      html += '</div>';"; 
  html += "    });";
  html += "    document.getElementById('networks').innerHTML = html;"; 
  html += "  });";
  html += "}";
  html += "function selectNetwork(ssid, secure) {";
  html += "  document.getElementById('ssid').value = ssid;";
  html += "  if (!secure) document.getElementById('password').value = '';";
  html += "}";
  html += "function connectWiFi() {";
  html += "  const ssid = document.getElementById('ssid').value;";
  html += "  const password = document.getElementById('password').value;";
  html += "  if (!ssid) { alert('Please enter SSID'); return; }";
  html += "  fetch('/connect', {";
  html += "    method: 'POST',";
  html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
  html += "    body: 'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(password)";
  html += "  }).then(r => r.text()).then(data => { alert(data); });";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Water Monitor</h1>";

  if (isAPMode) {
    html += "<div class='warning'>⚠ <strong>Configuration Mode</strong><br>Device is in Access Point mode. Connect to WiFi to start monitoring.</div>";
    html += "<h2>WiFi Setup</h2>";
    html += "<p>Connect to your WiFi network:</p>";
    html += "<button class='scan' onclick='scanNetworks()'>📡 Scan Networks</button>";
    html += "<div id='networks' class='network-list'></div>";
    html += "<div style='margin-top: 20px;'>";
    html += "<input type='text' id='ssid' placeholder='WiFi SSID' />";
    html += "<input type='password' id='password' placeholder='WiFi Password' />";
    html += "<button class='connect-btn' onclick='connectWiFi()'>Connect</button>";
    html += "</div>";

  } else {

    html += "<div class='tank-container'>";
    html += "<h2>Water Tank Level</h2>";
    html += "<div class='tank'>";
    html += "<div id='tankFill' class='tank-fill' style='width: 0%'></div>";
    html += "</div>";

    html += "<div class='sensor-data'>";
    html += "<div class='sensor-item'>";
    html += "<div class='sensor-label'>Distance</div>";
    html += "<div class='sensor-value'><span id='distance'>--</span> cm</div>";
    html += "</div>";
    html += "<div class='sensor-item'>";
    html += "<div class='sensor-label'>Level</div>";
    html += "<div class='sensor-value'><span id='percent'>--</span> %</div>";
    html += "</div>";
    html += "</div>";
    html += "</div>";

    // Calibration
    html += "<h2>Tank Calibration</h2>";
    html += "<p>Set the distance measurements for your tank:</p>";
    html += "<div class='calibration-group'>";
    html += "<div class='calibration-input'>";
    html += "<label>Full Tank Distance (cm):</label>";
    html += "<input type='number' id='fullDist' step='0.1' min='0' placeholder='30.0' />";
    html += "</div>";
    html += "<div class='calibration-input'>";
    html += "<label>Empty Tank Distance (cm):</label>";
    html += "<input type='number' id='emptyDist' step='0.1' min='0' placeholder='200.0' />";
    html += "</div>";
    html += "</div>";
    html += "<button class='save-calibration' onclick='saveCalibration()'>Save Calibration</button>";

    // ==============================
    // LED CONTROL (No Page Reload)
    // ==============================
    html += "<h2>LED Control</h2>";
    html += "<p>Control the RGB LED (currently in " + String(ledAutoMode ? "AUTO" : "OFF") + " mode):</p>";
    html += "<button class='on' onclick=\"led('on')\">Enable AUTO Mode</button>";
    html += "<button class='off' onclick=\"led('off')\">Turn OFF</button>";

    // ==============================
    // SCREEN CONTROL (No Page Reload)
    // ==============================
    html += "<h2 style='margin-top: 30px;'>Screen Control</h2>";
    html += "<p>Control the OLED display (currently " + String(getScreenState() ? "ON" : "OFF") + "):</p>";
    html += "<button class='on' onclick=\"screen('on')\">Turn Screen ON</button>";
    html += "<button class='off' onclick=\"screen('off')\">Turn Screen OFF</button>";
  }

  // Info section
  html += "<div class='info'>";
  html += "<p><strong>Device Info:</strong></p>";
  if (isAPMode) {
    html += "<p>Mode: Access Point</p>";
    html += "<p>AP IP: " + WiFi.softAPIP().toString() + "</p>";
  } else {
    html += "<p>Mode: Station (Connected)</p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>SSID: " + WiFi.SSID() + "</p>";
    html += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
  }
  html += "<p><small>Hold GPIO 0 button for 3 seconds to reset WiFi settings</small></p>";
  html += "</div>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleCalibration() {
  if (server.hasArg("full") && server.hasArg("empty")) {
    float full = server.arg("full").toFloat();
    float empty = server.arg("empty").toFloat();
    
    if (full >= empty) {
      server.send(400, "text/plain", "Error: Full distance must be less than empty distance");
      return;
    }
    
    if (full <= 0 || empty <= 0) {
      server.send(400, "text/plain", "Error: Values must be positive");
      return;
    }
    
    setCalibration(full, empty);
    server.send(200, "text/plain", "Calibration saved! Full=" + String(full, 1) + "cm, Empty=" + String(empty, 1) + "cm");
  } else {
    server.send(400, "text/plain", "Missing calibration parameters");
  }
}

void handleGetCalibration() {
  String json = "{";
  json += "\"full\":" + String(fullDistance, 1) + ",";
  json += "\"empty\":" + String(emptyDistance, 1);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleData() {
  String json = "{";
  json += "\"valid\":" + String(sensorDataValid ? "true" : "false") + ",";
  json += "\"distance\":" + String(currentDistance, 1) + ",";
  json += "\"percent\":" + String(currentPercent, 1);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleScan() {
  Serial.println("Scanning networks...");
  int n = WiFi.scanNetworks();
  
  String json = "{\"networks\":[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]}";
  
  server.send(200, "application/json", json);
}

void handleConnect() {
  if (server.hasArg("ssid")) {
    String ssid = server.arg("ssid");
    String password = server.hasArg("password") ? server.arg("password") : "";
    
    Serial.println("Saving WiFi credentials...");
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    
    server.send(200, "text/plain", "Credentials saved! Device will restart and connect to: " + ssid);
    
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing SSID");
  }
}

void handleLed() {
  if (!server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"missing state\"}");
    return;
  }

  String state = server.arg("state");

  if (state == "on") {
    setLedAutoMode(true);
    ledState = HIGH;        // mark that LED is active
    server.send(200, "application/json", "{\"status\":\"LED AUTO enabled\"}");
    Serial.println("LED AUTO enabled");
  }
  else if (state == "off") {
    setLedAutoMode(false);  // disable auto mode
    ledOff();               // actually turn off the LED
    ledState = LOW;         // mark LED state as OFF
    server.send(200, "application/json", "{\"status\":\"LED OFF\"}");
    Serial.println("LED turned OFF manually");
  }
  else {
    server.send(400, "application/json", "{\"error\":\"invalid state\"}");
  }
}



void handleLedOn() {
  ledAutoMode = true;
  ledState = HIGH;
  Serial.println("LED set to AUTO mode (ON)");
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2;url=/'>";
  html += "<style>body { font-family: Arial; text-align: center; margin: 50px; }</style>";
  html += "</head><body>";
  html += "<h1>LED AUTO Mode Enabled</h1>";
  html += "<p>LED will now respond to water level</p>";
  html += "<p>Redirecting back...</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleLedOff() {
  ledAutoMode = false;
  ledState = LOW;
  ledOff();
  Serial.println("LED set to MANUAL mode (OFF)");
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2;url=/'>";
  html += "<style>body { font-family: Arial; text-align: center; margin: 50px; }</style>";
  html += "</head><body>";
  html += "<h1>LED Turned OFF</h1>";
  html += "<p>LED will stay off until turned back on</p>";
  html += "<p>Redirecting back...</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}


void handleScreen() {
  if (!server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"missing state\"}");
    return;
  }

  String state = server.arg("state");

  if (state == "on") {
    setScreenState(true);
    server.send(200, "application/json", "{\"status\":\"SCREEN ON\"}");
  }
  else if (state == "off") {
    setScreenState(false);
    server.send(200, "application/json", "{\"status\":\"SCREEN OFF\"}");
  }
  else {
    server.send(400, "application/json", "{\"error\":\"invalid state\"}");
  }
}



void handleScreenOn() {
  setScreenState(true);
  Serial.println("Screen turned ON via web");
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2;url=/'>";
  html += "<style>body { font-family: Arial; text-align: center; margin: 50px; }</style>";
  html += "</head><body>";
  html += "<h1>Screen Turned ON</h1>";
  html += "<p>OLED display is now active</p>";
  html += "<p>Redirecting back...</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleScreenOff() {
  setScreenState(false);
  Serial.println("Screen turned OFF via web");
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2;url=/'>";
  html += "<style>body { font-family: Arial; text-align: center; margin: 50px; }</style>";
  html += "</head><body>";
  html += "<h1>Screen Turned OFF</h1>";
  html += "<p>OLED display is now inactive</p>";
  html += "<p>Redirecting back...</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleStatus() {
  String json = "{";
  json += "\"led\":\"" + String(ledState == HIGH ? "ON" : "OFF") + "\",";
  json += "\"ledMode\":\"" + String(ledAutoMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"screen\":\"" + String(getScreenState() ? "ON" : "OFF") + "\",";
  json += "\"mode\":\"" + String(isAPMode ? "AP" : "STA") + "\",";
  if (isAPMode) {
    json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
  } else {
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"ssid\":\"" + WiFi.SSID() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  }
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleNotFound() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<style>body { font-family: Arial; text-align: center; margin: 50px; }</style>";
  html += "</head><body>";
  html += "<h1>404 - Not Found</h1>";
  html += "<p>The page you're looking for doesn't exist.</p>";
  html += "<a href='/'>Go to home page</a>";
  html += "</body></html>";
  
  server.send(404, "text/html", html);
}

