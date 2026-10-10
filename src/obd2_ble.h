#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>

struct OBD2Telemetry {
    bool     connected;
    uint16_t speed;           // km/h (PID 010D)
    uint16_t rpm;             // RPM (PID 010C)
    int16_t  coolant_temp;    // °C (PID 0105)
    float    battery_voltage; // V (ATRV)
    uint8_t  fuel_level;      // % (PID 012F)
    uint8_t  throttle;        // % (PID 0111)
    uint32_t last_update;
};

class OBD2BLE {
public:
    static OBD2BLE& instance();

    void begin();
    void update();

    const OBD2Telemetry& getTelemetry() const { return _telemetry; }
    bool isConnected() const { return _telemetry.connected; }

    void onScanResult(NimBLEAdvertisedDevice* advertisedDevice);

private:
    OBD2BLE();
    bool connectToServer(NimBLEAddress pAddress);
    void sendQuery(const char* cmd);
    void parseResponse(const String& resp);

    OBD2Telemetry _telemetry;
    NimBLEClient* _pClient;
    NimBLERemoteCharacteristic* _pWriteChar;
    NimBLERemoteCharacteristic* _pNotifyChar;
    NimBLEAddress* _pServerAddress;

    bool _doConnect;
    bool _doScan;
    uint32_t _lastPollTime;
    uint8_t _pollState;
};
