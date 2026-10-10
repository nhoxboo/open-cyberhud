#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct SystemConfig {
    uint8_t  display_rotation;       // 0, 1, 2, 3
    bool     show_clock;             // Show clock in idle mode
    uint8_t  led_brightness;         // 0 - 255
    uint8_t  led_color;              // 0: Off, 1: Blue, 2: Green, 3: Red, 4: White, 5: Rainbow
    int8_t   speed_limit_offset;     // e.g. -5 to +5 km/h offset
    char     welcome_text[32];       // Custom greeting
    bool     show_text_kmh;          // Display "km/h" label
    bool     show_number_marker;     // Display speed ring markers
    uint8_t  night_brightness;       // Dimmed brightness
    uint8_t  day_brightness;         // Full brightness
    bool     show_speed;             // Live speed display enabled
    uint8_t  boot_screen_duration;   // Seconds to show boot logo
    bool     beep_limit_change;      // Beep when speed limit changes
    uint16_t inactive_duration;      // Standby timeout in seconds
    bool     show_image_holder;      // Placeholder/icon mode
    uint8_t  obd_scan_duration;      // Seconds to scan for OBD
    bool     obd_enabled;            // Enable OBD2 client
    bool     enable_buzzer;          // Enable sound alerts
    uint8_t  overspeed_threshold;    // km/h over limit before alert (default 5)
};

class ConfigManager {
public:
    static ConfigManager& instance();

    void begin();
    void loadDefaults();
    bool load();
    bool save();

    void setRotation(uint8_t rot) { _config.display_rotation = rot; }

    // Parse BKHN command strings (e.g. "BKHN;display_rotation=1;show_clock=1")
    bool parseBKHNCommand(const String& cmd, String& response);

    SystemConfig& config() { return _config; }

private:
    ConfigManager();
    Preferences _prefs;
    SystemConfig _config;
    bool _isLoaded;
};
