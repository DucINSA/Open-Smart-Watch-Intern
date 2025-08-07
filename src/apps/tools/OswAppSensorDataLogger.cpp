#include "apps/tools/OswAppSensorDataLogger.h"
#include "assets/img/icons/app.png.h"
#include <OswLogger.h>
#include <gfx_util.h>
#include <math_osm.h>

OswAppSensorDataLogger::OswAppSensorDataLogger() : OswAppV2() {
    this->dataUpdated = false;
    this->lastUpdateTime = 0;
    this->lastServerUpdate = 0;
    this->serverConnected = false;
    this->updateInterval = 5000; // 5 seconds
    this->displayMode = 0;
    this->autoUpdate = true;
    this->serverUrl = "http://your-server.com/api/sensor-data";
}

const char* OswAppSensorDataLogger::getAppId() {
    return "osw.sensorlogger";
}

const char* OswAppSensorDataLogger::getAppName() {
    return "Sensor Logger";
}

const OswIcon& OswAppSensorDataLogger::getAppIcon() {
    return app_png; // Using app icon
}

void OswAppSensorDataLogger::onStart() {
    OswAppV2::onStart();
    this->viewFlags = (OswAppV2::ViewFlags) (this->viewFlags | OswAppV2::ViewFlags::KEEP_DISPLAY_ON);
    
    // Initialize sensor data
    this->updateSensorData();
    OSW_LOG_I("Sensor Data Logger started");
}

void OswAppSensorDataLogger::onLoop() {
    OswAppV2::onLoop();
    
    unsigned long currentTime = millis();
    
    // Update sensor data every 2 seconds
    if (currentTime - this->lastUpdateTime > 2000) {
        this->updateSensorData();
        this->lastUpdateTime = currentTime;
        this->dataUpdated = true;
    }
    
    // Send data to server every updateInterval
    if (this->autoUpdate && currentTime - this->lastServerUpdate > this->updateInterval) {
        this->sendDataToServer();
        this->lastServerUpdate = currentTime;
    }
    
    this->needsRedraw = this->dataUpdated;
    this->dataUpdated = false;
}

void OswAppSensorDataLogger::onDraw() {
    OswAppV2::onDraw();
    
    switch (this->displayMode) {
        case 0:
            this->drawSensorData();
            break;
        case 1:
            this->drawConnectionStatus();
            break;
        case 2:
            this->drawSettings();
            break;
    }
}

void OswAppSensorDataLogger::onDrawOverlay() {
    OswAppV2::onDrawOverlay();
    
    // Draw mode indicator
    OswHal::getInstance()->gfx()->setTextSize(1);
    OswHal::getInstance()->gfx()->setTextCursor(5, 5);
    OswHal::getInstance()->gfx()->print("Mode: ");
    switch (this->displayMode) {
        case 0: OswHal::getInstance()->gfx()->print("Sensors"); break;
        case 1: OswHal::getInstance()->gfx()->print("Network"); break;
        case 2: OswHal::getInstance()->gfx()->print("Settings"); break;
    }
}

void OswAppSensorDataLogger::onStop() {
    OswAppV2::onStop();
    OSW_LOG_I("Sensor Data Logger stopped");
}

void OswAppSensorDataLogger::updateSensorData() {
    // Get temperature
    if (OswHal::getInstance()->environment()) {
        this->currentData.temperature = OswHal::getInstance()->environment()->getTemperature();
    }
    
    // Get acceleration data
    if (OswHal::getInstance()->environment()) {
        this->currentData.acceleration[0] = OswHal::getInstance()->environment()->getAccelerationX();
        this->currentData.acceleration[1] = OswHal::getInstance()->environment()->getAccelerationY();
        this->currentData.acceleration[2] = OswHal::getInstance()->environment()->getAccelerationZ();
    }
    
    // Get steps
    if (OswHal::getInstance()->environment()) {
        this->currentData.steps = OswHal::getInstance()->environment()->getStepsToday();
    }
    
    // Get battery level
    this->currentData.batteryLevel = OswHal::getInstance()->getBatteryPercent();
    
    // Get timestamp
    this->currentData.timestamp = millis();
    
    OSW_LOG_D("Sensor data updated - Temp: ", this->currentData.temperature, 
               " Steps: ", this->currentData.steps,
               " Battery: ", this->currentData.batteryLevel);
}

void OswAppSensorDataLogger::sendDataToServer() {
    if (!this->connectToServer()) {
        OSW_LOG_E("Failed to connect to server");
        return;
    }
    
    HTTPClient http;
    http.begin(this->serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    String jsonData = this->formatSensorData();
    
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        OSW_LOG_I("Server response: ", response);
        this->serverConnected = true;
    } else {
        OSW_LOG_E("HTTP error: ", httpResponseCode);
        this->serverConnected = false;
    }
    
    http.end();
}

void OswAppSensorDataLogger::drawSensorData() {
    OswHal::getInstance()->gfx()->setTextSize(1);
    OswHal::getInstance()->gfx()->setTextCursor(10, 30);
    OswHal::getInstance()->gfx()->print("Temperature: ");
    OswHal::getInstance()->gfx()->print(this->currentData.temperature, 1);
    OswHal::getInstance()->gfx()->print("C");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 50);
    OswHal::getInstance()->gfx()->print("Steps: ");
    OswHal::getInstance()->gfx()->print(this->currentData.steps);
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 70);
    OswHal::getInstance()->gfx()->print("Battery: ");
    OswHal::getInstance()->gfx()->print(this->currentData.batteryLevel, 1);
    OswHal::getInstance()->gfx()->print("%");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 90);
    OswHal::getInstance()->gfx()->print("Accel X: ");
    OswHal::getInstance()->gfx()->print(this->currentData.acceleration[0], 2);
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 110);
    OswHal::getInstance()->gfx()->print("Accel Y: ");
    OswHal::getInstance()->gfx()->print(this->currentData.acceleration[1], 2);
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 130);
    OswHal::getInstance()->gfx()->print("Accel Z: ");
    OswHal::getInstance()->gfx()->print(this->currentData.acceleration[2], 2);
}

void OswAppSensorDataLogger::drawConnectionStatus() {
    OswHal::getInstance()->gfx()->setTextSize(1);
    OswHal::getInstance()->gfx()->setTextCursor(10, 30);
    OswHal::getInstance()->gfx()->print("Server Status:");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 50);
    if (this->serverConnected) {
        OswHal::getInstance()->gfx()->print("Connected");
    } else {
        OswHal::getInstance()->gfx()->print("Disconnected");
    }
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 70);
    OswHal::getInstance()->gfx()->print("URL: ");
    OswHal::getInstance()->gfx()->print(this->serverUrl);
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 90);
    OswHal::getInstance()->gfx()->print("Auto Update: ");
    OswHal::getInstance()->gfx()->print(this->autoUpdate ? "ON" : "OFF");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 110);
    OswHal::getInstance()->gfx()->print("Interval: ");
    OswHal::getInstance()->gfx()->print(this->updateInterval / 1000);
    OswHal::getInstance()->gfx()->print("s");
}

void OswAppSensorDataLogger::drawSettings() {
    OswHal::getInstance()->gfx()->setTextSize(1);
    OswHal::getInstance()->gfx()->setTextCursor(10, 30);
    OswHal::getInstance()->gfx()->print("Settings:");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 50);
    OswHal::getInstance()->gfx()->print("Update Interval: ");
    OswHal::getInstance()->gfx()->print(this->updateInterval / 1000);
    OswHal::getInstance()->gfx()->print("s");
    
    OswHal::getInstance()->gfx()->setTextCursor(10, 70);
    OswHal::getInstance()->gfx()->print("Auto Update: ");
    OswHal::getInstance()->gfx()->print(this->autoUpdate ? "ON" : "OFF");
}

String OswAppSensorDataLogger::formatSensorData() {
    DynamicJsonDocument doc(1024);
    
    doc["device_id"] = "OSW_1";
    doc["timestamp"] = this->currentData.timestamp;
    doc["temperature"] = this->currentData.temperature;
    doc["steps"] = this->currentData.steps;
    doc["battery"] = this->currentData.batteryLevel;
    doc["acceleration"]["x"] = this->currentData.acceleration[0];
    doc["acceleration"]["y"] = this->currentData.acceleration[1];
    doc["acceleration"]["z"] = this->currentData.acceleration[2];
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool OswAppSensorDataLogger::connectToServer() {
    // Basic connection check - you can enhance this
    return WiFi.status() == WL_CONNECTED;
}
