#include "config_manager.h"
#include "pin_definitions.h"

static const char* PREFS_NAMESPACE = "openhud";

ConfigManager::ConfigManager() : _isLoaded(false) {
    loadDefaults();
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

void ConfigManager::loadDefaults() {
#if defined(DEFAULT_ROTATION)
    _config.display_rotation     = DEFAULT_ROTATION;
#elif defined(BOARD_ESP32_2432S024)
    _config.display_rotation     = 1; // Default Landscape (320x240)
#else
    _config.display_rotation     = 0;
#endif
    _config.show_clock           = true;
    _config.led_brightness       = 200;
    _config.led_color            = 1; // Blue
    _config.speed_limit_offset   = 0;
    strncpy(_config.welcome_text, "OpenCyberHUD", sizeof(_config.welcome_text) - 1);
    _config.welcome_text[sizeof(_config.welcome_text) - 1] = '\0';
    _config.show_text_kmh        = true;
    _config.show_number_marker   = true;
    _config.night_brightness     = 50;
    _config.day_brightness       = 240;
    _config.show_speed           = true;
    _config.boot_screen_duration = 2;
    _config.beep_limit_change    = true;
    _config.inactive_duration    = 45;
    _config.show_image_holder    = true;
    _config.obd_scan_duration    = 10;
    _config.obd_enabled          = false;
    _config.enable_buzzer        = true;
    _config.overspeed_threshold  = 5;
}

void ConfigManager::begin() {
    _prefs.begin(PREFS_NAMESPACE, false);
    if (!load()) {
        loadDefaults();
        save();
    }
    _isLoaded = true;
}

bool ConfigManager::load() {
    if (!_prefs.isKey("init")) {
        return false;
    }
#if defined(DEFAULT_ROTATION)
    uint8_t defRot = DEFAULT_ROTATION;
#else
    uint8_t defRot = 0;
#endif
    _config.display_rotation     = _prefs.getUChar("rot", defRot);
    _config.show_clock           = _prefs.getBool("clock", true);
    _config.led_brightness       = _prefs.getUChar("led_br", 200);
    _config.led_color            = _prefs.getUChar("led_col", 1);
    _config.speed_limit_offset   = _prefs.getChar("sp_off", 0);
    String w = _prefs.getString("welcome", "OpenCyberHUD");
    strncpy(_config.welcome_text, w.c_str(), sizeof(_config.welcome_text) - 1);
    _config.welcome_text[sizeof(_config.welcome_text) - 1] = '\0';
    _config.show_text_kmh        = _prefs.getBool("kmh", true);
    _config.show_number_marker   = _prefs.getBool("num_m", true);
    _config.night_brightness     = _prefs.getUChar("n_br", 50);
    _config.day_brightness       = _prefs.getUChar("d_br", 240);
    _config.show_speed           = _prefs.getBool("speed", true);
    _config.boot_screen_duration = _prefs.getUChar("boot_d", 2);
    _config.beep_limit_change    = _prefs.getBool("beep_l", true);
    _config.inactive_duration    = _prefs.getUShort("inact", 45);
    _config.show_image_holder    = _prefs.getBool("img_h", true);
    _config.obd_scan_duration    = _prefs.getUChar("obd_d", 10);
    _config.obd_enabled          = _prefs.getBool("obd_en", false);
    _config.enable_buzzer        = _prefs.getBool("buzz", true);
    _config.overspeed_threshold  = _prefs.getUChar("over_th", 5);
    return true;
}

bool ConfigManager::save() {
    _prefs.putBool("init", true);
    _prefs.putUChar("rot", _config.display_rotation);
    _prefs.putBool("clock", _config.show_clock);
    _prefs.putUChar("led_br", _config.led_brightness);
    _prefs.putUChar("led_col", _config.led_color);
    _prefs.putChar("sp_off", _config.speed_limit_offset);
    _prefs.putString("welcome", String(_config.welcome_text));
    _prefs.putBool("kmh", _config.show_text_kmh);
    _prefs.putBool("num_m", _config.show_number_marker);
    _prefs.putUChar("n_br", _config.night_brightness);
    _prefs.putUChar("d_br", _config.day_brightness);
    _prefs.putBool("speed", _config.show_speed);
    _prefs.putUChar("boot_d", _config.boot_screen_duration);
    _prefs.putBool("beep_l", _config.beep_limit_change);
    _prefs.putUShort("inact", _config.inactive_duration);
    _prefs.putBool("img_h", _config.show_image_holder);
    _prefs.putUChar("obd_d", _config.obd_scan_duration);
    _prefs.putBool("obd_en", _config.obd_enabled);
    _prefs.putBool("buzz", _config.enable_buzzer);
    _prefs.putUChar("over_th", _config.overspeed_threshold);
    return true;
}

bool ConfigManager::parseBKHNCommand(const String& cmd, String& response) {
    if (!cmd.startsWith("BKHN;")) {
        return false;
    }

    // Process key-value pairs separated by ';'
    int start = 5;
    bool dirty = false;

    while (start < (int)cmd.length()) {
        int end = cmd.indexOf(';', start);
        if (end == -1) end = cmd.length();
        String part = cmd.substring(start, end);
        part.trim();

        int eq = part.indexOf('=');
        if (eq != -1) {
            String key = part.substring(0, eq);
            String val = part.substring(eq + 1);
            key.trim();
            val.trim();

            if (key == "display_rotation") {
                _config.display_rotation = val.toInt() % 4;
                dirty = true;
            } else if (key == "show_clock") {
                _config.show_clock = (val.toInt() != 0);
                dirty = true;
            } else if (key == "led_brightness") {
                _config.led_brightness = constrain(val.toInt(), 0, 255);
                dirty = true;
            } else if (key == "led_color") {
                _config.led_color = val.toInt();
                dirty = true;
            } else if (key == "speed_limit_offset") {
                _config.speed_limit_offset = val.toInt();
                dirty = true;
            } else if (key == "welcome_text") {
                strncpy(_config.welcome_text, val.c_str(), sizeof(_config.welcome_text) - 1);
                _config.welcome_text[sizeof(_config.welcome_text) - 1] = '\0';
                dirty = true;
            } else if (key == "show_text_kmh") {
                _config.show_text_kmh = (val.toInt() != 0);
                dirty = true;
            } else if (key == "show_number_marker") {
                _config.show_number_marker = (val.toInt() != 0);
                dirty = true;
            } else if (key == "night_brightness") {
                _config.night_brightness = constrain(val.toInt(), 0, 255);
                dirty = true;
            } else if (key == "day_brightness") {
                _config.day_brightness = constrain(val.toInt(), 0, 255);
                dirty = true;
            } else if (key == "show_speed") {
                _config.show_speed = (val.toInt() != 0);
                dirty = true;
            } else if (key == "boot_screen_duration") {
                _config.boot_screen_duration = val.toInt();
                dirty = true;
            } else if (key == "beep_limit_change") {
                _config.beep_limit_change = (val.toInt() != 0);
                dirty = true;
            } else if (key == "inactive_duration") {
                _config.inactive_duration = val.toInt();
                dirty = true;
            } else if (key == "show_image_holder") {
                _config.show_image_holder = (val.toInt() != 0);
                dirty = true;
            } else if (key == "obd_scan_duration") {
                _config.obd_scan_duration = val.toInt();
                dirty = true;
            } else if (key == "obd_enabled") {
                _config.obd_enabled = (val.toInt() != 0);
                dirty = true;
            } else if (key == "enable_buzzer") {
                _config.enable_buzzer = (val.toInt() != 0);
                dirty = true;
            } else if (key == "overspeed_threshold") {
                _config.overspeed_threshold = val.toInt();
                dirty = true;
            }
        } else if (part == "save" || part == "SAVE") {
            save();
            dirty = false;
        } else if (part == "reset" || part == "RESET") {
            loadDefaults();
            save();
            dirty = false;
        }

        start = end + 1;
    }

    if (dirty) {
        save();
    }

    response = "BKHN;status=OK;rotation=" + String(_config.display_rotation) +
               ";led_color=" + String(_config.led_color) +
               ";led_br=" + String(_config.led_brightness) +
               ";speed_offset=" + String(_config.speed_limit_offset) + ";";
    return true;
}
