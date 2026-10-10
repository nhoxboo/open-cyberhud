#pragma once

#include <Arduino.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS & BOARD PROFILES
// =========================================================================

#if defined(BOARD_ESP32_2432S024)
    // ---------------------------------------------------------------------
    // Profile: ESP32-2432S024 (CYD 2.4" TFT ILI9341 320x240 + XPT2046 Touch)
    // MCU: ESP32-WROOM-32 (Xtensa Dual-Core 240MHz)
    // ---------------------------------------------------------------------
    #define BOARD_NAME          "ESP32-2432S024"
    #define SCREEN_WIDTH        320     // Landscape Width
    #define SCREEN_HEIGHT       240     // Landscape Height
    #define DEFAULT_ROTATION    1       // Landscape Mode (0: Portrait, 1: Landscape, 2: Inv Portrait, 3: Inv Landscape)
    #define SCREEN_IS_ROUND     false
    #define HAS_TOUCHSCREEN     true
    #define TOUCH_TYPE_XPT2046  true

    // ILI9341 SPI Display Pins (HSPI)
    #define TFT_MISO            12
    #define TFT_MOSI            13
    #define TFT_SCLK            14
    #define TFT_CS              15
    #define TFT_DC              2
    #define TFT_RST             -1      // Tied to EN/Reset
    #define TFT_BL              27      // Backlight PWM on 2.4" (GPIO 27)
    #define TFT_BL_ALT          21      // Alternate Backlight pin on some CYD revisions (GPIO 21)
    #define TFT_INVERT          false   // Standard ILI9341

    // XPT2046 Resistive Touchscreen (VSPI)
    #define TOUCH_CS            33
    #define TOUCH_INT           -1      // Polling mode (avoid GPIO36 floating IRQ)
    #define TOUCH_CLK           25
    #define TOUCH_MOSI          32
    #define TOUCH_MISO          39

    // Discrete 3-Pin RGB LED on CYD (Active LOW: LOW=ON, HIGH=OFF)
    #define PIN_LED_R           4
    #define PIN_LED_G           16
    #define PIN_LED_B           17
    #define PIN_RGB_LED         -1      // Discrete pins instead of WS2812B

    // Peripheral Pins
    #define PIN_BUZZER          26      // Speaker / Buzzer amp
    #define PIN_LDR             34      // Light Dependent Resistor (ADC1_CH6)
    #define PIN_BUTTON          0       // BOOT button

    // PWM Settings
    #define PWM_CHANNEL_BL      0
    #define PWM_FREQ_BL         5000
    #define PWM_RES_BL          8
    #define PWM_CHANNEL_BUZZER  1

#elif defined(BOARD_ESP32_2424S012)
    // ---------------------------------------------------------------------
    // Profile: ESP32-2424S012 (CYD 1.28" Round IPS GC9A01 + CST816D Touch)
    // MCU: ESP32-C3 RISC-V (160MHz)
    // ---------------------------------------------------------------------
    #define BOARD_NAME          "ESP32-2424S012"
    #define SCREEN_WIDTH        240
    #define SCREEN_HEIGHT       240
    #define DEFAULT_ROTATION    0
    #define SCREEN_IS_ROUND     true
    #define HAS_TOUCHSCREEN     true

    // GC9A01 SPI Display Pins
    #define TFT_MISO            -1
    #define TFT_MOSI            7
    #define TFT_SCLK            6
    #define TFT_CS              10
    #define TFT_DC              2
    #define TFT_RST             -1      // Tied to EN/Reset
    #define TFT_BL              3       // Backlight PWM active HIGH
    #define TFT_INVERT          true    // GC9A01 color inversion

    // CST816D Capacitive Touchscreen (I2C)
    #define TOUCH_SDA           4
    #define TOUCH_SCL           5
    #define TOUCH_INT           0
    #define TOUCH_RST           1
    #define TOUCH_I2C_ADDR      0x15

    // Peripheral Pins
    #define PIN_BUZZER          8       // Pad S1 (IO8)
    #define PIN_RGB_LED         -1      // No onboard WS2812B
    #define PIN_BUTTON          9       // BOOT button

    // PWM Settings
    #define PWM_CHANNEL_BL      0
    #define PWM_FREQ_BL         44100
    #define PWM_RES_BL          8
    #define PWM_CHANNEL_BUZZER  1

#elif defined(BOARD_ESP32S3_ROUND_LCD_128)
    // ---------------------------------------------------------------------
    // Profile: Waveshare / LilyGO ESP32-S3 1.28" Round LCD (GC9A01 + CST816S)
    // ---------------------------------------------------------------------
    #define BOARD_NAME          "ESP32-S3-Round-1.28"
    #define SCREEN_WIDTH        240
    #define SCREEN_HEIGHT       240
    #define DEFAULT_ROTATION    0
    #define SCREEN_IS_ROUND     true
    #define HAS_TOUCHSCREEN     true

    #define TFT_MISO            -1
    #define TFT_MOSI            11
    #define TFT_SCLK            10
    #define TFT_CS              9
    #define TFT_DC              8
    #define TFT_RST             12
    #define TFT_BL              40
    #define TFT_INVERT          true

    #define TOUCH_SDA           6
    #define TOUCH_SCL           7
    #define TOUCH_INT           5
    #define TOUCH_RST           13
    #define TOUCH_I2C_ADDR      0x15

    #define PIN_BUZZER          14
    #define PIN_RGB_LED         38
    #define PIN_BUTTON          0

    #define PWM_CHANNEL_BL      0
    #define PWM_FREQ_BL         5000
    #define PWM_RES_BL          8
    #define PWM_CHANNEL_BUZZER  1

#elif defined(BOARD_ESP32C3_SUPERMINI)
    // ---------------------------------------------------------------------
    // Profile: ESP32-C3 SuperMini + ST7789 240x240
    // ---------------------------------------------------------------------
    #define BOARD_NAME          "ESP32-C3-SuperMini"
    #define SCREEN_WIDTH        240
    #define SCREEN_HEIGHT       240
    #define DEFAULT_ROTATION    0
    #define SCREEN_IS_ROUND     false
    #define HAS_TOUCHSCREEN     false

    #define TFT_MISO            -1
    #define TFT_MOSI            6
    #define TFT_SCLK            4
    #define TFT_CS              7
    #define TFT_DC              2
    #define TFT_RST             3
    #define TFT_BL              1
    #define TFT_INVERT          true

    #define PIN_BUZZER          5
    #define PIN_RGB_LED         8   // Built-in Addressable LED
    #define PIN_BUTTON          9

    #define PWM_CHANNEL_BL      0
    #define PWM_FREQ_BL         5000
    #define PWM_RES_BL          8
    #define PWM_CHANNEL_BUZZER  1

#else
    // ---------------------------------------------------------------------
    // Profile: Original CyberHUD (ESP32-WROOM-32 + ST7789/GC9A01)
    // ---------------------------------------------------------------------
    #define BOARD_NAME          "CyberHUD-ESP32"
    #define SCREEN_WIDTH        240
    #define SCREEN_HEIGHT       240
    #define DEFAULT_ROTATION    0
    #define SCREEN_IS_ROUND     true
    #define HAS_TOUCHSCREEN     false

    #define TFT_MISO            -1
    #define TFT_MOSI            23
    #define TFT_SCLK            18
    #define TFT_CS              5
    #define TFT_DC              2
    #define TFT_RST             4
    #define TFT_BL              15
    #define TFT_INVERT          true

    #define PIN_BUZZER          25
    #define PIN_RGB_LED         27
    #define PIN_BUTTON          0

    #define PWM_CHANNEL_BL      0
    #define PWM_FREQ_BL         5000
    #define PWM_RES_BL          8
    #define PWM_CHANNEL_BUZZER  1
#endif
