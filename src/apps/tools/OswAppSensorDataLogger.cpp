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
    this->updateInterval = 1000; // 1 second for faster updates
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
    OswHal* hal = OswHal::getInstance();
    
    // Get all available sensor data in real-time (like OswAppPrintDebug)
    if (hal->environment()) {
        // Temperature
        this->currentData.temperature = hal->environment()->getTemperature();
        
        // Acceleration data
        this->currentData.acceleration[0] = hal->environment()->getAccelerationX();
        this->currentData.acceleration[1] = hal->environment()->getAccelerationY();
        this->currentData.acceleration[2] = hal->environment()->getAccelerationZ();
        
        // Steps
        this->currentData.steps = hal->environment()->getStepsToday();
        
        // Additional sensors if available
        #if OSW_PLATFORM_ENVIRONMENT_PRESSURE == 1
        this->currentData.pressure = hal->environment()->getPressure();
        #endif
        
        #if OSW_PLATFORM_ENVIRONMENT_HUMIDITY == 1
        this->currentData.humidity = hal->environment()->getHumidity();
        #endif
        
        #if OSW_PLATFORM_ENVIRONMENT_MAGNETOMETER == 1
        this->currentData.magnetometerAzimuth = hal->environment()->getMagnetometerAzimuth();
        #endif
    }
    
    // Get battery level (using raw value like debug app)
    this->currentData.batteryLevel = hal->getBatteryPercent();
    this->currentData.batteryRaw = hal->getBatteryRaw();
    this->currentData.isCharging = hal->isCharging();
    
    // Get system info
    this->currentData.ramUsed = ESP.getHeapSize() - ESP.getFreeHeap();
    this->currentData.ramTotal = ESP.getHeapSize();
    
    // Get timestamp
    this->currentData.timestamp = millis();
    
    OSW_LOG_D("Real-time sensor data updated - Temp: ", this->currentData.temperature, 
               " Steps: ", this->currentData.steps,
               " Battery: ", this->currentData.batteryLevel,
               " RAM: ", this->currentData.ramUsed, "/", this->currentData.ramTotal);
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
    OswHal* hal = OswHal::getInstance();
    
    // Title - moved down to avoid round screen cutoff
    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextCenterAligned();
    hal->gfx()->setTextCursor(120, 40);
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print("Sensor Data");
    
    // Sensor data - moved down for round screen
    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextLeftAligned();
    
    int yPos = 70; // Start lower to avoid top cutoff
    int lineHeight = 18; // Increased spacing
    
    // Temperature
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Temp: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.temperature, 1);
    hal->gfx()->print("°C");
    
    yPos += lineHeight;
    
    // Battery - emphasized for verification
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Battery: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.batteryLevel);
    hal->gfx()->print("% (");
    hal->gfx()->print(this->currentData.batteryRaw);
    hal->gfx()->print(")");
    if (this->currentData.isCharging) {
        hal->gfx()->setTextColor(ui->getSuccessColor());
        hal->gfx()->print(" CHG");
    }
    
    yPos += lineHeight;
    
    // Steps
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Steps: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.steps);
    
    yPos += lineHeight;
    
    // RAM usage
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("RAM: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.ramUsed);
    hal->gfx()->print("/");
    hal->gfx()->print(this->currentData.ramTotal);
    hal->gfx()->print("B");
    
    yPos += lineHeight;
    
    // Acceleration X
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Accel X: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.acceleration[0], 2);
    
    yPos += lineHeight;
    
    // Acceleration Y
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Accel Y: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.acceleration[1], 2);
    
    yPos += lineHeight;
    
    // Acceleration Z
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Accel Z: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->currentData.acceleration[2], 2);
    
    // Last update time at bottom
    hal->gfx()->setTextCursor(20, 220);
    hal->gfx()->setTextColor(ui->getBackgroundDimmedColor());
    hal->gfx()->print("Updated: ");
    hal->gfx()->print((millis() - this->lastUpdateTime) / 1000);
    hal->gfx()->print("s ago");
}

void OswAppSensorDataLogger::drawConnectionStatus() {
    OswHal* hal = OswHal::getInstance();
    
    // Title - moved down for round screen
    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextCenterAligned();
    hal->gfx()->setTextCursor(120, 40);
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print("Network Status");
    
    // Connection status - moved down for round screen
    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextLeftAligned();
    
    int yPos = 70;
    int lineHeight = 18;
    
    // Server status
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Server: ");
    hal->gfx()->setTextColor(this->serverConnected ? ui->getSuccessColor() : ui->getDangerColor());
    hal->gfx()->print(this->serverConnected ? "Connected" : "Disconnected");
    
    yPos += lineHeight;
    
    // WiFi status
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("WiFi: ");
    hal->gfx()->setTextColor(WiFi.status() == WL_CONNECTED ? ui->getSuccessColor() : ui->getDangerColor());
    hal->gfx()->print(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    
    yPos += lineHeight;
    
    // Auto update status
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Auto Update: ");
    hal->gfx()->setTextColor(this->autoUpdate ? ui->getSuccessColor() : ui->getDangerColor());
    hal->gfx()->print(this->autoUpdate ? "ON" : "OFF");
    
    yPos += lineHeight;
    
    // Update interval
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Interval: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print(this->updateInterval / 1000);
    hal->gfx()->print("s");
    
    yPos += lineHeight;
    
    // Last server update
    hal->gfx()->setTextCursor(20, yPos);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->print("Last Update: ");
    hal->gfx()->setTextColor(ui->getForegroundColor());
    hal->gfx()->print((millis() - this->lastServerUpdate) / 1000);
    hal->gfx()->print("s ago");
    
    // Battery level at bottom for verification
    hal->gfx()->setTextCursor(20, 220);
    hal->gfx()->setTextColor(ui->getBackgroundDimmedColor());
    hal->gfx()->print("Battery: ");
    hal->gfx()->print(this->currentData.batteryLevel);
    hal->gfx()->print("%");
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
    DynamicJsonDocument doc(2048); // Increased size for more data
    
    // Device info
    doc["device_id"] = "OSW_1";
    doc["timestamp"] = this->currentData.timestamp;
    
    // Environment sensors
    doc["temperature"] = this->currentData.temperature;
    doc["humidity"] = this->currentData.humidity;
    doc["pressure"] = this->currentData.pressure;
    doc["magnetometer_azimuth"] = this->currentData.magnetometerAzimuth;
    
    // Motion sensors
    doc["steps"] = this->currentData.steps;
    doc["activity_mode"] = this->currentData.activityMode;
    doc["acceleration"]["x"] = this->currentData.acceleration[0];
    doc["acceleration"]["y"] = this->currentData.acceleration[1];
    doc["acceleration"]["z"] = this->currentData.acceleration[2];
    
    // Battery and system
    doc["battery"]["level"] = this->currentData.batteryLevel;
    doc["battery"]["raw"] = this->currentData.batteryRaw;
    doc["battery"]["charging"] = this->currentData.isCharging;
    doc["system"]["ram_used"] = this->currentData.ramUsed;
    doc["system"]["ram_total"] = this->currentData.ramTotal;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool OswAppSensorDataLogger::connectToServer() {
    // Basic connection check - you can enhance this
    return WiFi.status() == WL_CONNECTED;
}
