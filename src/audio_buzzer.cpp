#include "audio_buzzer.h"
#include "config_manager.h"

AudioBuzzer::AudioBuzzer()
    : _isPlaying(false), _noteEndTime(0), _patternStep(0), _activePattern(0), _stepTime(0) {}

AudioBuzzer& AudioBuzzer::instance() {
    static AudioBuzzer inst;
    return inst;
}

void AudioBuzzer::begin() {
    #if defined(PIN_BUZZER) && PIN_BUZZER >= 0
    ledcSetup(PWM_CHANNEL_BUZZER, 2400, 8);
    ledcAttachPin(PIN_BUZZER, PWM_CHANNEL_BUZZER);
    ledcWrite(PWM_CHANNEL_BUZZER, 0);
    #endif
}

void AudioBuzzer::playBeep(uint16_t freq, uint16_t duration_ms) {
    if (!ConfigManager::instance().config().enable_buzzer) return;
    #if defined(PIN_BUZZER) && PIN_BUZZER >= 0
    ledcWriteTone(PWM_CHANNEL_BUZZER, freq);
    _noteEndTime = millis() + duration_ms;
    _isPlaying = true;
    #endif
}

void AudioBuzzer::playSpeedLimitTone() {
    if (!ConfigManager::instance().config().enable_buzzer) return;
    _activePattern = 1;
    _patternStep = 0;
    _stepTime = millis();
}

void AudioBuzzer::playOverspeedAlert() {
    if (!ConfigManager::instance().config().enable_buzzer) return;
    _activePattern = 2;
    _patternStep = 0;
    _stepTime = millis();
}

void AudioBuzzer::playCameraAlert() {
    if (!ConfigManager::instance().config().enable_buzzer) return;
    _activePattern = 3;
    _patternStep = 0;
    _stepTime = millis();
}

void AudioBuzzer::playAlertTone(uint8_t tone_code) {
    switch (tone_code) {
        case 1: playSpeedLimitTone(); break;
        case 2: playCameraAlert(); break;
        case 3: playOverspeedAlert(); break;
        default: playBeep(2400, 100); break;
    }
}

void AudioBuzzer::playBootChime() {
    _activePattern = 4;
    _patternStep = 0;
    _stepTime = millis();
}

void AudioBuzzer::stop() {
    #if defined(PIN_BUZZER) && PIN_BUZZER >= 0
    ledcWrite(PWM_CHANNEL_BUZZER, 0);
    #endif
    _isPlaying = false;
    _activePattern = 0;
}

void AudioBuzzer::update() {
    uint32_t now = millis();

    if (_isPlaying && now >= _noteEndTime) {
        #if defined(PIN_BUZZER) && PIN_BUZZER >= 0
        ledcWrite(PWM_CHANNEL_BUZZER, 0);
        #endif
        _isPlaying = false;
    }

    if (_activePattern == 0) return;

    // Pattern 1: Speed limit chime (Double ascending tone)
    if (_activePattern == 1) {
        if (_patternStep == 0) {
            playBeep(2000, 60);
            _stepTime = now + 90;
            _patternStep = 1;
        } else if (_patternStep == 1 && now >= _stepTime) {
            playBeep(2800, 100);
            _activePattern = 0;
        }
    }
    // Pattern 2: Overspeed warning (Triple high-pitched pulse)
    else if (_activePattern == 2) {
        if (_patternStep == 0) {
            playBeep(3200, 70);
            _stepTime = now + 120;
            _patternStep = 1;
        } else if (_patternStep == 1 && now >= _stepTime) {
            playBeep(3200, 70);
            _stepTime = now + 120;
            _patternStep = 2;
        } else if (_patternStep == 2 && now >= _stepTime) {
            playBeep(3200, 100);
            _activePattern = 0;
        }
    }
    // Pattern 3: Camera alert (Two distinct pulses)
    else if (_activePattern == 3) {
        if (_patternStep == 0) {
            playBeep(2500, 80);
            _stepTime = now + 130;
            _patternStep = 1;
        } else if (_patternStep == 1 && now >= _stepTime) {
            playBeep(2500, 80);
            _activePattern = 0;
        }
    }
    // Pattern 4: Boot chime (Three-tone startup melody)
    else if (_activePattern == 4) {
        if (_patternStep == 0) {
            playBeep(1500, 60);
            _stepTime = now + 80;
            _patternStep = 1;
        } else if (_patternStep == 1 && now >= _stepTime) {
            playBeep(2000, 60);
            _stepTime = now + 80;
            _patternStep = 2;
        } else if (_patternStep == 2 && now >= _stepTime) {
            playBeep(3000, 120);
            _activePattern = 0;
        }
    }
}
