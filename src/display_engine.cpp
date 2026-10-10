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
        // Fallback to 8-bit color sprite if contiguous 16-bit buffer allocation fails
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
    _canvas.drawCircle(cx, cy, 80, 0x18E3); // Dark Gray
    _canvas.drawArc(cx, cy, 82, 78, -90, -90 + (int32_t)(progress * 360.0f), 0x07FF); // Cyan

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

#if SCREEN_WIDTH > SCREEN_HEIGHT
    // Landscape Layout (320x240)
    _canvas.drawRoundRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 12, 0x2104);

    if (ConfigManager::instance().config().show_clock) {
        _canvas.setTextColor(0xAD55, TFT_BLACK);
        _canvas.setTextSize(4);
        _canvas.drawString("--:--", cx, cy - 25);
    }

    _canvas.setTextColor(0x07FF, TFT_BLACK);
    _canvas.setTextSize(1);
    _canvas.drawString("CHO KET NOI VIETMAP LIVE", cx, cy + 30);

    _canvas.setTextColor(0x7BEF, TFT_BLACK);
    _canvas.drawString("BLE: VIETMAP_HUD (0xFFF0)", cx, cy + 50);

    _canvas.setTextColor(0x52AA, TFT_BLACK);
    _canvas.drawString("Chạm màn hình để đổi góc xoay", cx, cy + 75);

#else
    // Round / Square Layout (240x240)
    _canvas.drawCircle(cx, cy, 110, 0x2104);

    if (ConfigManager::instance().config().show_clock) {
        _canvas.setTextColor(0xAD55, TFT_BLACK);
        _canvas.setTextSize(3);
        _canvas.drawString("--:--", cx, cy - 20);
    }

    _canvas.setTextColor(0x07FF, TFT_BLACK);
    _canvas.setTextSize(1);
    _canvas.drawString("CHO KET NOI VIETMAP LIVE", cx, cy + 30);

    _canvas.setTextColor(0x7BEF, TFT_BLACK);
    _canvas.drawString("BLE: VIETMAP_HUD", cx, cy + 50);
#endif

    _canvas.pushSprite(0, 0);
}

void DisplayEngine::drawSpeedLimitSign(int x, int y, int radius, uint8_t limit) {
    if (limit == 0) return;

    // Red outer border
    _canvas.fillCircle(x, y, radius, 0xF800); // Red
    // White inner fill
    _canvas.fillCircle(x, y, radius - 4, 0xFFFF); // White

    // Bold limit number
    _canvas.setTextColor(0x0000, 0xFFFF); // Black
    _canvas.setTextSize(radius > 22 ? 2 : 1);
    _canvas.drawString(String(limit).c_str(), x, y);
}

void DisplayEngine::drawCameraAlert(int x, int y, uint8_t alertType, uint16_t distance) {
    if (alertType == ALERT_NONE) return;

    // Banner Background
    _canvas.fillRoundRect(x - 65, y - 18, 130, 36, 6, 0xFA40); // Orange-Red

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
    else if (alertType == ALERT_SPEED_CHECK_ZONE) label = "DOAN TOC DO";

    _canvas.setTextColor(0xFFFF, 0xFA40);
    _canvas.setTextSize(1);
    _canvas.drawString(label.c_str(), x, y - 6);
    _canvas.drawString(distStr.c_str(), x, y + 8);
}

void DisplayEngine::drawNavTurn(int x, int y, uint8_t icon, uint16_t distance) {
    if (icon == NAV_NONE) return;

    _canvas.fillRoundRect(x - 55, y - 25, 110, 50, 6, 0x18E3); // Dark slate

    String arrow = "^";
    if (icon == NAV_TURN_LEFT || icon == NAV_TURN_HARD_LEFT || icon == NAV_TURN_SLIGHT_LEFT) arrow = "<--";
    else if (icon == NAV_TURN_RIGHT || icon == NAV_TURN_HARD_RIGHT || icon == NAV_TURN_SLIGHT_RIGHT) arrow = "-->";
    else if (icon == NAV_UTURN) arrow = "<==";
    else if (icon == NAV_ROUNDABOUT) arrow = "(O)";

    _canvas.setTextColor(0x07FF, 0x18E3);
    _canvas.setTextSize(2);
    _canvas.drawString(arrow.c_str(), x, y - 8);

    String distStr;
    if (distance >= 1000) distStr = String((float)distance / 1000.0f, 1) + "km";
    else distStr = String(distance) + "m";

    _canvas.setTextColor(0xFFFF, 0x18E3);
    _canvas.setTextSize(1);
    _canvas.drawString(distStr.c_str(), x, y + 14);
}

void DisplayEngine::drawHUD() {
    _canvas.fillScreen(TFT_BLACK);

    const VietmapTelemetry& vm = VietmapBLE::instance().getTelemetry();
    const OBD2Telemetry& obd = OBD2BLE::instance().getTelemetry();
    const SystemConfig& cfg = ConfigManager::instance().config();

    // 1. Calculate final speed
    uint16_t currentSpeed = vm.connected ? vm.speed : (obd.connected ? obd.speed : 0);
    int finalSpeed = (int)currentSpeed + cfg.speed_limit_offset;
    if (finalSpeed < 0) finalSpeed = 0;

    // 2. Overspeed Color Coding
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

#if SCREEN_WIDTH > SCREEN_HEIGHT
    // =========================================================================
    // LANDSCAPE HUD LAYOUT (320x240) for ESP32-2432S024 CYD
    // =========================================================================

    // Top Status Line: Road Name
    if (strlen(vm.road_name) > 0) {
        _canvas.setTextColor(0x07FF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString(vm.road_name, SCREEN_WIDTH / 2, 14);
    } else {
        _canvas.setTextColor(0x4208, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString("VIETMAP LIVE NAVIGATION", SCREEN_WIDTH / 2, 14);
    }

    // Divider Line
    _canvas.drawFastHLine(10, 26, SCREEN_WIDTH - 20, 0x2104);

    // Left Column: Speedometer (x: 10 -> 155)
    int leftCx = 80;
    int leftCy = 115;

    // Speed Limit Circular Sign (Top Left)
    if (vm.speed_limit > 0) {
        drawSpeedLimitSign(32, 58, 22, vm.speed_limit);
    }

    // Big Digital Speed
    _canvas.setTextColor(speedColor, TFT_BLACK);
    _canvas.setTextSize(5);
    _canvas.drawString(String(finalSpeed).c_str(), leftCx, leftCy);

    // KM/H label
    if (cfg.show_text_kmh) {
        _canvas.setTextColor(0x7BEF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString("KM/H", leftCx, leftCy + 42);
    }

    // Vertical Divider between Left & Right panels
    _canvas.drawFastVLine(160, 32, 165, 0x2104);

    // Right Column: Navigation & Alerts (x: 165 -> 315)
    int rightCx = 240;

    if (vm.nav_icon != NAV_NONE) {
        drawNavTurn(rightCx, 70, vm.nav_icon, vm.nav_distance);
    }

    if (vm.alert_type != ALERT_NONE) {
        drawCameraAlert(rightCx, 145, vm.alert_type, vm.alert_distance);
        LEDIndicator::instance().setState(LED_CAMERA_ALERT);
    }

    // Bottom Status Bar (y: 215)
    _canvas.drawFastHLine(10, 205, SCREEN_WIDTH - 20, 0x2104);
    if (obd.connected) {
        _canvas.setTextColor(0x07E0, TFT_BLACK);
        _canvas.setTextSize(1);
        String obdStr = String(obd.rpm) + " RPM  |  " + String(obd.coolant_temp) + "C  |  " + String(obd.battery_voltage, 1) + "V";
        _canvas.drawString(obdStr.c_str(), SCREEN_WIDTH / 2, 222);
    } else {
        _canvas.setTextColor(0x52AA, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString("LIVE GPS HUD  *  CONNECTED", SCREEN_WIDTH / 2, 222);
    }

#else
    // =========================================================================
    // ROUND / SQUARE HUD LAYOUT (240x240) for ESP32-2424S012
    // =========================================================================
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;

    if (cfg.show_number_marker) {
        _canvas.drawCircle(cx, cy, 115, 0x2945);
    }

    // Digital Speed
    _canvas.setTextColor(speedColor, TFT_BLACK);
    _canvas.setTextSize(5);
    _canvas.drawString(String(finalSpeed).c_str(), cx, cy - 10);

    // "KM/H"
    if (cfg.show_text_kmh) {
        _canvas.setTextColor(0x7BEF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString("KM/H", cx, cy + 30);
    }

    // Speed Limit Sign
    if (vm.speed_limit > 0) {
        drawSpeedLimitSign(cx + 65, cy - 65, 24, vm.speed_limit);
    }

    // Camera Alert
    if (vm.alert_type != ALERT_NONE) {
        drawCameraAlert(cx, cy + 65, vm.alert_type, vm.alert_distance);
        LEDIndicator::instance().setState(LED_CAMERA_ALERT);
    }

    // Road Name
    if (strlen(vm.road_name) > 0) {
        _canvas.setTextColor(0x07FF, TFT_BLACK);
        _canvas.setTextSize(1);
        _canvas.drawString(vm.road_name, cx, cy - 80);
    }

    // OBD2 Status
    if (obd.connected) {
        _canvas.setTextColor(0x07E0, TFT_BLACK);
        _canvas.setTextSize(1);
        String obdStr = String(obd.rpm) + " RPM | " + String(obd.coolant_temp) + "C | " + String(obd.battery_voltage, 1) + "V";
        _canvas.drawString(obdStr.c_str(), cx, cy + 95);
    }
#endif

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
