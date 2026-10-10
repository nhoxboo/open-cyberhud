#include "led_indicator.h"
#include "config_manager.h"

LEDIndicator::LEDIndicator()
    : _pixels(nullptr), _currentState(LED_IDLE), _lastAnimTime(0), _animStep(0) {}

LEDIndicator& LEDIndicator::instance() {
    static LEDIndicator inst;
    return inst;
}

void LEDIndicator::begin() {
    #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
    _pixels = new Adafruit_NeoPixel(1, PIN_RGB_LED, NEO_GRB + NEO_KHZ800);
    _pixels->begin();
    _pixels->setBrightness(ConfigManager::instance().config().led_brightness);
    _pixels->show();
    #elif defined(PIN_LED_R) && defined(PIN_LED_G) && defined(PIN_LED_B)
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    // Active LOW -> HIGH turns OFF
    digitalWrite(PIN_LED_R, HIGH);
    digitalWrite(PIN_LED_G, HIGH);
    digitalWrite(PIN_LED_B, HIGH);
    #endif
}

void LEDIndicator::setColor(uint8_t r, uint8_t g, uint8_t b) {
    #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
    if (_pixels) {
        _pixels->setBrightness(ConfigManager::instance().config().led_brightness);
        _pixels->setPixelColor(0, _pixels->Color(r, g, b));
        _pixels->show();
    }
    #elif defined(PIN_LED_R) && defined(PIN_LED_G) && defined(PIN_LED_B)
    // Discrete active-low RGB LED
    digitalWrite(PIN_LED_R, r > 30 ? LOW : HIGH);
    digitalWrite(PIN_LED_G, g > 30 ? LOW : HIGH);
    digitalWrite(PIN_LED_B, b > 30 ? LOW : HIGH);
    #endif
}

void LEDIndicator::setState(LEDState state) {
    _currentState = state;
}

void LEDIndicator::update() {
    uint32_t now = millis();
    if (now - _lastAnimTime < 30) return;
    _lastAnimTime = now;
    _animStep++;

    uint8_t configured_color = ConfigManager::instance().config().led_color;
    uint8_t br = ConfigManager::instance().config().led_brightness;

    switch (_currentState) {
        case LED_IDLE: {
            #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
            float breath = (exp(sin(_animStep / 20.0 * PI)) - 0.36787944) * 108.0;
            uint8_t dynamic_br = (uint8_t)((breath / 255.0) * br);
            if (_pixels) _pixels->setBrightness(dynamic_br);
            #endif

            if (configured_color == 1)      setColor(0, 150, 255); // Cyan/Blue
            else if (configured_color == 2) setColor(0, 255, 50);  // Green
            else if (configured_color == 3) setColor(255, 30, 0);  // Red
            else if (configured_color == 4) setColor(255, 255, 255); // White
            else setColor(0, 150, 255);
            break;
        }

        case LED_CONNECTED: {
            #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
            if (_pixels) _pixels->setBrightness(br);
            #endif
            if (configured_color == 1)      setColor(0, 200, 255);
            else if (configured_color == 2) setColor(0, 255, 100);
            else setColor(0, 200, 255);
            break;
        }

        case LED_OVERSPEED: {
            // Fast red flashing
            if ((_animStep / 4) % 2 == 0) {
                #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
                if (_pixels) _pixels->setBrightness(255);
                #endif
                setColor(255, 0, 0);
            } else {
                setColor(0, 0, 0);
            }
            break;
        }

        case LED_CAMERA_ALERT: {
            // Amber/Yellow pulse
            if ((_animStep / 6) % 2 == 0) {
                #if defined(PIN_RGB_LED) && (PIN_RGB_LED >= 0)
                if (_pixels) _pixels->setBrightness(255);
                #endif
                setColor(255, 165, 0);
            } else {
                setColor(0, 0, 0);
            }
            break;
        }

        case LED_STANDBY: {
            setColor(0, 0, 0);
            break;
        }
    }
}
