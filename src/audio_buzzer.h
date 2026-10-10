#pragma once

#include <Arduino.h>
#include "pin_definitions.h"

class AudioBuzzer {
public:
    static AudioBuzzer& instance();

    void begin();
    void update();

    void playBeep(uint16_t freq = 2400, uint16_t duration_ms = 80);
    void playSpeedLimitTone();
    void playOverspeedAlert();
    void playCameraAlert();
    void playAlertTone(uint8_t tone_code);
    void playBootChime();

    void stop();

private:
    AudioBuzzer();
    bool _isPlaying;
    uint32_t _noteEndTime;
    uint8_t _patternStep;
    uint8_t _activePattern; // 1: speed limit, 2: overspeed, 3: camera, 4: boot
    uint32_t _stepTime;
};
