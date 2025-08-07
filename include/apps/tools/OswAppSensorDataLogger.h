#ifndef OSW_APP_SENSOR_DATA_LOGGER_H
#define OSW_APP_SENSOR_DATA_LOGGER_H

#include <OswAppV2.h>
#include <osw_hal.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class OswAppSensorDataLogger : public OswAppV2 {
public:
    OswAppSensorDataLogger();
    virtual ~OswAppSensorDataLogger() = default;

    const char* getAppId() override;
    const char* getAppName() override;
    const OswIcon& getAppIcon() override;

    void onStart() override;
    void onLoop() override;
    void onDraw() override;
    void onDrawOverlay() override;
    void onStop() override;

private:
    // Sensor data structure
    struct SensorData {
        float temperature;
        float humidity;
        float pressure;
        float acceleration[3];
        int steps;
        float batteryLevel;
        unsigned long timestamp;
    };

    // Data management
    SensorData currentData;
    bool dataUpdated;
    unsigned long lastUpdateTime;
    unsigned long lastServerUpdate;
    
    // Server communication
    String serverUrl;
    bool serverConnected;
    int updateInterval; // milliseconds
    
    // UI state
    int displayMode; // 0: sensors, 1: connection, 2: settings
    bool autoUpdate;
    
    // Methods
    void updateSensorData();
    void sendDataToServer();
    void drawSensorData();
    void drawConnectionStatus();
    void drawSettings();
    String formatSensorData();
    bool connectToServer();
};

#endif
