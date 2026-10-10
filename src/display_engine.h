#pragma once

#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "pin_definitions.h"
#include "vietmap_ble.h"
#include "obd2_ble.h"

// LovyanGFX display configuration for ESP32 / ESP32-S3 / ESP32-C3 / ESP32-2432S024 / ESP32-2424S012
class LGFX : public lgfx::LGFX_Device {
#if defined(BOARD_ESP32_2424S012) || defined(BOARD_ESP32S3_ROUND_LCD_128)
    lgfx::Panel_GC9A01  _panel_instance;
#elif defined(BOARD_ESP32_2432S024)
    lgfx::Panel_ILI9341 _panel_instance;
#else
    lgfx::Panel_ST7789  _panel_instance;
#endif

    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;

#if defined(TOUCH_TYPE_XPT2046) && TOUCH_TYPE_XPT2046
    lgfx::Touch_XPT2046 _touch_instance;
#elif defined(HAS_TOUCHSCREEN) && HAS_TOUCHSCREEN
    lgfx::Touch_CST816S _touch_instance;
#endif

public:
    LGFX() {
        {
            auto cfg = _bus_instance.config();
#if defined(BOARD_ESP32_2432S024)
            cfg.spi_host = HSPI_HOST; // SPI2_HOST on ESP32
            cfg.pin_miso = TFT_MISO;
#else
            cfg.spi_host = SPI2_HOST;
            cfg.pin_miso = -1;
#endif
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read  = 16000000;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_dc   = TFT_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = TFT_CS;
            cfg.pin_rst          = TFT_RST;
            cfg.pin_busy         = -1;
#if defined(BOARD_ESP32_2432S024)
            cfg.panel_width      = 240;
            cfg.panel_height     = 320;
#else
            cfg.panel_width      = SCREEN_WIDTH;
            cfg.panel_height     = SCREEN_HEIGHT;
#endif
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = false;
            cfg.invert           = TFT_INVERT;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = TFT_BL;
            cfg.invert = false;
            cfg.freq   = PWM_FREQ_BL;
            cfg.pwm_channel = PWM_CHANNEL_BL;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

#if defined(TOUCH_TYPE_XPT2046) && TOUCH_TYPE_XPT2046
        {
            auto cfg = _touch_instance.config();
            cfg.x_min = 300;
            cfg.x_max = 3900;
            cfg.y_min = 200;
            cfg.y_max = 3700;
            cfg.pin_int = -1; // Polling mode (avoid GPIO36 floating interrupt freeze)
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.spi_host = VSPI_HOST; // SPI3_HOST on ESP32
            cfg.pin_sclk = TOUCH_CLK;
            cfg.pin_mosi = TOUCH_MOSI;
            cfg.pin_miso = TOUCH_MISO;
            cfg.pin_cs   = TOUCH_CS;
            cfg.freq = 2000000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
#elif defined(HAS_TOUCHSCREEN) && HAS_TOUCHSCREEN
        {
            auto cfg = _touch_instance.config();
            cfg.x_min = 0;
            cfg.x_max = SCREEN_WIDTH - 1;
            cfg.y_min = 0;
            cfg.y_max = SCREEN_HEIGHT - 1;
            cfg.pin_int = TOUCH_INT;
            cfg.pin_rst = TOUCH_RST;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.i2c_port = 0;
            cfg.i2c_addr = TOUCH_I2C_ADDR;
            cfg.pin_sda = TOUCH_SDA;
            cfg.pin_scl = TOUCH_SCL;
            cfg.freq = 400000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
#endif

        setPanel(&_panel_instance);
    }
};

class DisplayEngine {
public:
    static DisplayEngine& instance();

    void begin();
    void update();
    void setBrightness(uint8_t brightness);

private:
    DisplayEngine();

    void handleTouch();
    void handleLDR();
    void drawBootScreen();
    void drawIdleScreen();
    void drawHUD();
    void drawSpeedLimitSign(int x, int y, int radius, uint8_t limit);
    void drawCameraAlert(int x, int y, uint8_t alertType, uint16_t distance);
    void drawNavTurn(int x, int y, uint8_t icon, uint16_t distance);
    void drawOBDRibbon(int y);

    LGFX _tft;
    LGFX_Sprite _canvas;
    uint32_t _bootStartTime;
    bool _bootFinished;
    uint32_t _lastRenderTime;
    uint32_t _lastTouchTime;
    uint32_t _lastLDRTime;
};
