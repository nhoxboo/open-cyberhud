#include <Arduino.h>
#include "config_manager.h"
#include "pin_definitions.h"
#include "vietmap_ble.h"
#include "obd2_ble.h"
#include "audio_buzzer.h"
#include "led_indicator.h"
#include "display_engine.h"

// Task handles for dual core execution
TaskHandle_t DisplayTaskHandle = NULL;

void displayTask(void* parameter) {
    for (;;) {
        DisplayEngine::instance().update();
        vTaskDelay(pdMS_TO_TICKS(16)); // ~60 FPS loop
    }
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n========================================");
    Serial.printf("   OpenCyberHUD Firmware v8.09 [%s]\n", BOARD_NAME);
    Serial.println("========================================");
    Serial.printf("[System] Free Heap at Boot: %u bytes\n", ESP.getFreeHeap());

    // 1. Initialize Persistent Config (NVS)
    ConfigManager::instance().begin();

    // 2. Initialize Audio Buzzer & LED
    AudioBuzzer::instance().begin();
    LEDIndicator::instance().begin();

    // 3. Initialize Display
    Serial.println("[Display] Initializing LCD & Backlight...");
    DisplayEngine::instance().begin();
    Serial.printf("[Display] Free Heap after Display Init: %u bytes\n", ESP.getFreeHeap());

    // 4. Initialize Vietmap BLE Peripheral Server
    Serial.println("[BLE] Starting BLE Peripheral...");
    VietmapBLE::instance().begin("VIETMAP_HUD");

    // 5. Initialize OBD2 BLE Client (if enabled)
    if (ConfigManager::instance().config().obd_enabled) {
        OBD2BLE::instance().begin();
    }

    #if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
    // Run display rendering on Core 1, BLE / Network on Core 0
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        8192,
        NULL,
        1,
        &DisplayTaskHandle,
        1
    );
    #endif

    Serial.printf("[System] All systems initialized. Free Heap: %u bytes\n", ESP.getFreeHeap());
}

void loop() {
    // BLE Protocol updates & Timeouts
    VietmapBLE::instance().update();
    OBD2BLE::instance().update();

    // Sound & LED animations
    AudioBuzzer::instance().update();
    LEDIndicator::instance().update();

    #if !CONFIG_IDF_TARGET_ESP32 && !CONFIG_IDF_TARGET_ESP32S3
    // Single core ESP (e.g. ESP32-C3)
    DisplayEngine::instance().update();
    #endif

    delay(5);
}
