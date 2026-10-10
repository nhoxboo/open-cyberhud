#include "vietmap_ble.h"
#include "config_manager.h"
#include "audio_buzzer.h"

// Vietmap BLE UUIDs
static const char* SERVICE_UUID        = "0000fff0-0000-1000-8000-00805f9b34fb";
static const char* CHAR_NOTIFY_UUID    = "0000fff1-0000-1000-8000-00805f9b34fb";
static const char* CHAR_WRITE_UUID     = "0000fff2-0000-1000-8000-00805f9b34fb";
static const char* CHAR_CONFIG_UUID    = "0000fff3-0000-1000-8000-00805f9b34fb";

// Device Info Service
static const char* DEVINFO_SVC_UUID    = "0000180a-0000-1000-8000-00805f9b34fb";
static const char* DEVINFO_VER_UUID    = "00002a26-0000-1000-8000-00805f9b34fb";

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        VietmapBLE::instance().handleConnect();
    }
    void onDisconnect(NimBLEServer* pServer) override {
        VietmapBLE::instance().handleDisconnect();
        NimBLEDevice::startAdvertising();
    }
};

class WriteCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            VietmapBLE::instance().handlePacket(reinterpret_cast<const uint8_t*>(rxValue.data()), rxValue.length());
        }
    }
};

VietmapBLE::VietmapBLE() : _pServer(nullptr), _pNotifyChar(nullptr), _pWriteChar(nullptr), _pConfigChar(nullptr) {
    memset(&_telemetry, 0, sizeof(_telemetry));
}

VietmapBLE& VietmapBLE::instance() {
    static VietmapBLE inst;
    return inst;
}

void VietmapBLE::begin(const char* deviceName) {
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(false, false, false);

    _pServer = NimBLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks());

    // 1. Device Info Service
    NimBLEService* pDevInfoSvc = _pServer->createService(DEVINFO_SVC_UUID);
    NimBLECharacteristic* pVerChar = pDevInfoSvc->createCharacteristic(DEVINFO_VER_UUID, NIMBLE_PROPERTY::READ);
    pVerChar->setValue("v@8.09");
    pDevInfoSvc->start();

    // 2. Vietmap Custom Service
    NimBLEService* pCustomSvc = _pServer->createService(SERVICE_UUID);

    _pNotifyChar = pCustomSvc->createCharacteristic(CHAR_NOTIFY_UUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    _pWriteChar  = pCustomSvc->createCharacteristic(CHAR_WRITE_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    _pConfigChar = pCustomSvc->createCharacteristic(CHAR_CONFIG_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    _pWriteChar->setCallbacks(new WriteCallbacks());
    _pConfigChar->setCallbacks(new WriteCallbacks());

    pCustomSvc->start();

    // 3. BLE Advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    NimBLEDevice::startAdvertising();
}

void VietmapBLE::handleConnect() {
    _telemetry.connected = true;
    _telemetry.last_packet_time = millis();
}

void VietmapBLE::handleDisconnect() {
    _telemetry.connected = false;
    _telemetry.speed = 0;
    _telemetry.speed_limit = 0;
    _telemetry.alert_type = ALERT_NONE;
}

void VietmapBLE::sendHandshakeResponse() {
    // 92-byte handshake response matching official H1V / CyberHUD response
    uint8_t handshakeResp[92];
    memset(handshakeResp, 0, sizeof(handshakeResp));

    handshakeResp[0] = 0xA5;
    handshakeResp[1] = 0x5A;
    handshakeResp[2] = 0x37;
    handshakeResp[3] = 0xC3;
    handshakeResp[7] = 0x5C; // 92 bytes length

    handshakeResp[8]  = 0x64;
    handshakeResp[9]  = 0xE2;
    handshakeResp[10] = 0x2D;
    handshakeResp[11] = 0xCA;
    handshakeResp[12] = 0x0E;

    const char* infoStr = "MODEL:H1V,HW:1.6.4,FW:1.3.6,PROTOCOL:2.2.2, OBDV:,COMPILE:Feb 27 2021-12:13:56";
    size_t infoLen = strlen(infoStr);
    if (infoLen > 67) infoLen = 67;
    memcpy(&handshakeResp[16], infoStr, infoLen);

    // Trailing signature bytes
    uint8_t sig[] = { 0xC0, 0xFE, 0xB7, 0x7E, 0x6B, 0x94, 0x21, 0x5F };
    memcpy(&handshakeResp[84], sig, sizeof(sig));

    if (_pNotifyChar) {
        _pNotifyChar->setValue(handshakeResp, sizeof(handshakeResp));
        _pNotifyChar->notify();
    }
}

void VietmapBLE::handlePacket(const uint8_t* data, size_t length) {
    if (length == 0) return;
    _telemetry.last_packet_time = millis();

    // Check if it's a BKHN text config command
    if (length >= 5 && data[0] == 'B' && data[1] == 'K' && data[2] == 'H' && data[3] == 'N' && data[4] == ';') {
        String cmd = "";
        for (size_t i = 0; i < length; i++) cmd += (char)data[i];
        String resp = "";
        ConfigManager::instance().parseBKHNCommand(cmd, resp);
        if (_pConfigChar && resp.length() > 0) {
            _pConfigChar->setValue(reinterpret_cast<const uint8_t*>(resp.c_str()), resp.length());
            _pConfigChar->notify();
        }
        return;
    }

    // Check for Vietmap binary protocol header 0xA5 0x5A
    if (length >= 4 && data[0] == 0xA5 && data[1] == 0x5A) {
        // Handshake request: 0xA5 0x5A 0x37 0xC3
        if (data[2] == 0x37 && data[3] == 0xC3) {
            sendHandshakeResponse();
            return;
        }

        parsePayload(data, length);
    }
}

void VietmapBLE::parsePayload(const uint8_t* data, size_t len) {
    if (len < 13) return;

    // In Vietmap packets, opcode is at byte offset 12
    uint8_t opcode = data[12];

    switch (opcode) {
        case 0x01: // Live Speed update
            if (len >= 15) {
                uint16_t raw_speed = (data[13] << 8) | data[14];
                _telemetry.speed = raw_speed;
            }
            break;

        case 0x10: // Speed limit update
            if (len >= 14) {
                uint8_t old_limit = _telemetry.speed_limit;
                _telemetry.speed_limit = data[13];
                if (ConfigManager::instance().config().beep_limit_change &&
                    _telemetry.speed_limit != old_limit && _telemetry.speed_limit > 0) {
                    AudioBuzzer::instance().playSpeedLimitTone();
                }
            }
            break;

        case 0x11: // Camera / POI Alert
            if (len >= 17) {
                _telemetry.alert_type = data[13];
                _telemetry.alert_speed = data[14];
                _telemetry.alert_distance = (data[15] << 8) | data[16];
            }
            break;

        case 0x13: // Sound / Beep Trigger from App
            if (len >= 14) {
                uint8_t sound_code = data[13];
                AudioBuzzer::instance().playAlertTone(sound_code);
            }
            break;

        case 0x40: // Turn-by-turn Navigation Instruction
            if (len >= 18) {
                _telemetry.nav_icon = data[13];
                _telemetry.nav_distance = (data[14] << 8) | data[15];

                // Check for road name text
                if (len >= 23) {
                    uint8_t text_len = data[22];
                    if (text_len > 0 && len >= (size_t)(23 + text_len)) {
                        size_t copy_len = min((size_t)text_len, sizeof(_telemetry.road_name) - 1);
                        memcpy(_telemetry.road_name, &data[23], copy_len);
                        _telemetry.road_name[copy_len] = '\0';
                    }
                }
            }
            break;

        default:
            break;
    }
}

void VietmapBLE::update() {
    // Timeout check: If no packet received in 15 seconds, mark disconnected
    if (_telemetry.connected && (millis() - _telemetry.last_packet_time > 15000)) {
        _telemetry.connected = false;
        _telemetry.speed = 0;
        _telemetry.speed_limit = 0;
        _telemetry.alert_type = ALERT_NONE;
    }
}
