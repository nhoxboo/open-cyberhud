#include "display_engine.h"
#include "config_manager.h"
#include "audio_buzzer.h"
#include "led_indicator.h"

DisplayEngine::DisplayEngine()
    : _canvas(&_tft), _bootStartTime(0), _bootFinished(false), _lastRenderTime(0), _lastTouchTime(0), _lastLDRTime(0) {}

DisplayEngine& DisplayEngine::instance() {
    static DisplayEngine inst;
    return inst;
}

void DisplayEngine::begin() {
#if defined(BOARD_ESP32_2432S024)
    // Drive both GPIO 27 and GPIO 21 HIGH to guarantee backlight on all CYD 2.4" and 2.8" revisions
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
#endif

    _tft.init();
    _tft.setRotation(ConfigManager::instance().config().display_rotation);
    setBrightness(ConfigManager::instance().config().day_brightness);
    _tft.fillScreen(TFT_BLACK);

    _canvas.setColorDepth(16);
    if (!_canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        // Fallback to 8-bit color sprite if 16-bit 153KB contiguous block is unavailable
        _canvas.setColorDepth(8);
        _canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    _canvas.setTextDatum(textdatum_t::middle_center);

    _bootStartTime = millis();
    AudioBuzzer::instance().playBootChime();
}

void DisplayEngine::setBrightness(uint8_t brightness) {
    _tft.setBrightness(brightness);
#if defined(BOARD_ESP32_2432S024)
    if (brightness > 0) {
        digitalWrite(27, HIGH);
        digitalWrite(21, HIGH);
    } else {
        digitalWrite(27, LOW);
        digitalWrite(21, LOW);
    }
#endif
}

void DisplayEngine::handleLDR() {
#if defined(PIN_LDR) && (PIN_LDR > 0)
    uint32_t now = millis();
    if (now - _lastLDRTime > 2000) { // Check ambient light every 2s
        _lastLDRTime = now;
        int ldrVal = analogRead(PIN_LDR); // 0 (dark) to 4095 (bright)
        uint8_t minB = ConfigManager::instance().config().night_brightness;
        uint8_t maxB = ConfigManager::instance().config().day_brightness;
        uint8_t targetB = map(ldrVal, 200, 3800, minB, maxB);
        if (targetB < minB) targetB = minB;
        if (targetB > maxB) targetB = maxB;
        setBrightness(targetB);
    }
#endif
}

void DisplayEngine::handleTouch() {
#if defined(HAS_TOUCHSCREEN) && HAS_TOUCHSCREEN
    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        uint32_t now = millis();
        if (now - _lastTouchTime > 400) { // Debounce touch
            _lastTouchTime = now;
            AudioBuzzer::instance().playBeep(2400, 30);

            // Cycle display rotation on screen tap
            uint8_t currentRot = ConfigManager::instance().config().display_rotation;
            uint8_t newRot = (currentRot + 1) % 4;
            ConfigManager::instance().setRotation(newRot);
            ConfigManager::instance().save();
            _tft.setRotation(newRot);
        }
    }
#endif
}

void DisplayEngine::drawBootScreen() {
    _canvas.fillScreen(TFT_BLACK);

    uint32_t elapsed = millis() - _bootStartTime;
    float progress = (float)elapsed / (ConfigManager::instance().config().boot_screen_duration * 1000.0f);
    if (progress > 1.0f) progress = 1.0f;

    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    // Outer progress ring
    _canvas.drawCircle(cx, cy, 90, 0x18E3); // Dark Gray
    _canvas.drawArc(cx, cy, 92, 88, -90, -90 + (int32_t)(progress * 360.0f), 0x07FF); // Cyan

    // Welcome title
    _canvas.setTextColor(0xFFFF, TFT_BLACK);
    _canvas.setTextSize(2);
    _canvas.drawString(ConfigManager::instance().config().welcome_text, cx, cy - 15);

    // Version & Board label
    _canvas.setTextColor(0x07FF, TFT_BLACK);
    _canvas.setTextSize(1);
    _canvas.drawString(BOARD_NAME, cx, cy + 15);

    _canvas.pushSprite(0, 0);

    if (elapsed >= (uint32_t)(ConfigManager::instance().config().boot_screen_duration * 1000)) {
        _bootFinished = true;
    }
}

void DisplayEngine::drawIdleScreen() {
    _canvas.fillScreen(TFT_BLACK);

    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    // Decorative ring
    _canvas.drawCircle(cx, cy, 110, 0x2104);

    // Clock or Standby Text
    if (ConfigManager::instance().config().show_clock) {
        _canvas.setTextColor(0xAD55, TFT_BLACK); // Slate Gray
        _canvas.setTextSize(3);
        _canvas.drawString("--:--", cx, cy - 20);
    }

    // BLE Status icon / text
    _canvas.setTextColor(0x07FF, TFT_BLACK);
    _canvas.setTextSize(1);
    _canvas.drawString("WAITING VIETMAP LIVE", cx, cy + 30);

    _canvas.setTextColor(0x7BEF, TFT_BLACK);
    _canvas.drawString("BLE: VIETMAP_HUD", cx, cy + 50);

    _canvas.pushSprite(0, 0);
}

void DisplayEngine::drawSpeedLimitSign(int x, int y, int radius, uint8_t limit) {
    if (limit == 0) return;

    // Red outer circle
    _canvas.fillCircle(x, y, radius, 0xF800); // Red
    // White inner circle
    _canvas.fillCircle(x, y, radius - 4, 0xFFFF); // White

    // Speed number
    _canvas.setTextColor(0x0000, 0xFFFF); // Black on white
    _canvas.setTextSize(radius > 25 ? 2 : 1);
    _canvas.drawString(String(limit).c_str(), x, y);
}

void DisplayEngine::drawCameraAlert(int x, int y, uint8_t alertType, uint16_t distance) {
    if (alertType == ALERT_NONE) return;

    // Alert Banner Background
    _canvas.fillRoundRect(x - 60, y - 18, 120, 36, 8, 0xFA40); // Orange-Red

    String distStr;
    if (distance >= 1000) {
        distStr = String((float)distance / 1000.0f, 1) + "km";
    } else {
        distStr = String(distance) + "m";
    }

    String label = "CAM PHAT NGUOI";
    if (alertType == ALERT_SPEED_CAMERA) label = "CAM TOC DO";
    else if (alertType == ALERT_RED_LIGHT_CAMERA) label = "CAM DEN DO";
    else if (alertType == ALERT_TOLL_BOOTH) label = "TRAM THU PHI";

    _canvas.setTextColor(0xFFFF, 0xFA40);
    _canvas.setTextSize(1);
    _canvas.drawString(label.c_str(), x, y - 6);
    _canvas.drawString(distStr.c_str(), x, y + 8);
}

void DisplayEngine::drawHUD() {
    _canvas.fillScreen(TFT_BLACK);

    const VietmapTelemetry& vm = VietmapBLE::instance().getTelemetry();
    const OBD2Telemetry& obd = OBD2BLE::instance().getTelemetry();
    const SystemConfig& cfg = ConfigManager::instance().config();

    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    // 1. Determine speed source (Vietmap or OBD)
    uint16_t currentSpeed = vm.connected ? vm.speed : (obd.connected ? obd.speed : 0);

    // Apply calibration offset
    int finalSpeed = (int)currentSpeed + cfg.speed_limit_offset;
    if (finalSpeed < 0) finalSpeed = 0;

    // 2. Speed Ring / Markers
    if (cfg.show_number_marker) {
        _canvas.drawCircle(cx, cy, 115, 0x2945); // Outer dark frame
    }

    // 3. Overspeed check & Color Coding
    bool isOverspeed = false;
    uint16_t speedColor = 0xFFFF; // Default White

    if (vm.speed_limit > 0) {
        if (finalSpeed > vm.speed_limit + cfg.overspeed_threshold) {
            isOverspeed = true;
            speedColor = 0xF800; // Red
            LEDIndicator::instance().setState(LED_OVERSPEED);
            AudioBuzzer::instance().playOverspeedAlert();
        } else if (finalSpeed >= vm.speed_limit) {
            speedColor = 0xFEA0; // Yellow/Amber
            LEDIndicator::instance().setState(LED_CONNECTED);
        } else {
            speedColor = 0x07E0; // Green
            LEDIndicator::instance().setState(LED_CONNECTED);
        }
    } else {
        LEDIndicator::instance().setState(LED_CONNECTED);
    }

    // 4. Large Digital Speed Display
    _canvas.setTextColor(speedColor, TFT_BLACK);
    _canvas.setTextSize(5);
    _canvas.drawString(String(finalSpeed).c_str(), cx, cy - 10);

    // 5. "km/h" label
    if (cfg.show_text_kmh) {
        _canvas.setTextColor(0x7BEF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString("KM/H", cx, cy + 30);
    }

    // 6. Speed Limit Circle (Top Left or Right)
    if (vm.speed_limit > 0) {
        drawSpeedLimitSign(cx + 65, cy - 65, 26, vm.speed_limit);
    }

    // 7. Camera / POI Alert (Bottom or Top)
    if (vm.alert_type != ALERT_NONE) {
        drawCameraAlert(cx, cy + 65, vm.alert_type, vm.alert_distance);
        LEDIndicator::instance().setState(LED_CAMERA_ALERT);
    }

    // 8. Road Name / Navigation instruction banner (Top)
    if (strlen(vm.road_name) > 0) {
        _canvas.setTextColor(0x07FF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString(vm.road_name, cx, cy - 80);
    }

    // 9. OBD2 Bottom Status Bar (if connected)
    if (obd.connected) {
        _canvas.setTextColor(0x07E0, TFT_BLACK);
        _canvas.setTextSize(1);
        String obdStr = String(obd.rpm) + " RPM | " + String(obd.coolant_temp) + "C | " + String(obd.battery_voltage, 1) + "V";
        _canvas.drawString(obdStr.c_str(), cx, cy + 95);
    }

    _canvas.pushSprite(0, 0);
}

void DisplayEngine::update() {
    handleTouch();
    handleLDR();

    uint32_t now = millis();
    if (now - _lastRenderTime < 33) return; // ~30 FPS refresh rate
    _lastRenderTime = now;

    if (!_bootFinished) {
        drawBootScreen();
        return;
    }

    bool isConnected = VietmapBLE::instance().isConnected() || OBD2BLE::instance().isConnected();

    if (isConnected) {
        drawHUD();
    } else {
        drawIdleScreen();
    }
}
