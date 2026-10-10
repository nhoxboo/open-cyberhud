#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>

enum CameraAlertType {
    ALERT_NONE = 0,
    ALERT_SPEED_CAMERA = 1,
    ALERT_RED_LIGHT_CAMERA = 2,
    ALERT_HIGHWAY_ENTRY = 3,
    ALERT_HIGHWAY_EXIT = 4,
    ALERT_TOLL_BOOTH = 5,
    ALERT_SPEED_CHECK_ZONE = 6,
    ALERT_DANGER_ZONE = 7,
    ALERT_OVERTAKING_FORBIDDEN = 8,
    ALERT_RAILWAY_CROSSING = 9,
    ALERT_TRAFFIC_CHECK = 10
};

enum NavTurnIcon {
    NAV_NONE = 0,
    NAV_STRAIGHT = 1,
    NAV_TURN_SLIGHT_LEFT = 2,
    NAV_TURN_LEFT = 3,
    NAV_TURN_HARD_LEFT = 4,
    NAV_TURN_SLIGHT_RIGHT = 5,
    NAV_TURN_RIGHT = 6,
    NAV_TURN_HARD_RIGHT = 7,
    NAV_UTURN = 8,
    NAV_ROUNDABOUT = 9,
    NAV_DESTINATION = 10
};

struct VietmapTelemetry {
    bool     connected;
    uint16_t speed;              // km/h
    uint8_t  speed_limit;        // km/h (0 = no limit / unknown)
    uint8_t  alert_type;         // CameraAlertType
    uint16_t alert_distance;     // meters
    uint8_t  alert_speed;        // speed limit for camera (km/h)
    uint8_t  nav_icon;           // NavTurnIcon
    uint16_t nav_distance;       // meters to next turn
    char     road_name[128];     // Current street or instruction text
    bool     beep_triggered;     // Beep flag
    uint32_t last_packet_time;   // millis()
};

class VietmapBLE {
public:
    static VietmapBLE& instance();

    void begin(const char* deviceName = "VIETMAP_HUD");
    void update();

    const VietmapTelemetry& getTelemetry() const { return _telemetry; }
    bool isConnected() const { return _telemetry.connected; }

    // Internal callbacks for NimBLE
    void handleConnect();
    void handleDisconnect();
    void handlePacket(const uint8_t* data, size_t length);

private:
    VietmapBLE();
    void sendHandshakeResponse();
    void parsePayload(const uint8_t* payload, size_t len);

    VietmapTelemetry _telemetry;
    NimBLEServer* _pServer;
    NimBLECharacteristic* _pNotifyChar;
    NimBLECharacteristic* _pWriteChar;
    NimBLECharacteristic* _pConfigChar;
};
