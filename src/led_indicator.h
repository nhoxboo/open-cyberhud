#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "pin_definitions.h"

enum LEDState {
    LED_IDLE,
    LED_CONNECTED,
    LED_OVERSPEED,
    LED_CAMERA_ALERT,
    LED_STANDBY
};

class LEDIndicator {
public:
    static LEDIndicator& instance();

    void begin();
    void update();
    void setState(LEDState state);
    void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
    LEDIndicator();
    Adafruit_NeoPixel* _pixels;
    LEDState _currentState;
    uint32_t _lastAnimTime;
    uint8_t _animStep;
};
