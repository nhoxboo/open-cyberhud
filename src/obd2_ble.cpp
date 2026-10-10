#include "obd2_ble.h"
#include "config_manager.h"

// Standard ELM327 BLE Service UUIDs
static NimBLEUUID OBD_SERVICE_UUID("fff0");
static NimBLEUUID OBD_CHAR_WRITE_UUID("fff2");
static NimBLEUUID OBD_CHAR_NOTIFY_UUID("fff1");

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
        OBD2BLE::instance().onScanResult(advertisedDevice);
    }
};

OBD2BLE::OBD2BLE()
    : _pClient(nullptr), _pWriteChar(nullptr), _pNotifyChar(nullptr),
      _pServerAddress(nullptr), _doConnect(false), _doScan(false),
      _lastPollTime(0), _pollState(0) {
    memset(&_telemetry, 0, sizeof(_telemetry));
}

OBD2BLE& OBD2BLE::instance() {
    static OBD2BLE inst;
    return inst;
}

void OBD2BLE::begin() {
    if (!ConfigManager::instance().config().obd_enabled) return;
    _doScan = true;
}

void OBD2BLE::onScanResult(NimBLEAdvertisedDevice* advertisedDevice) {
    String devName = advertisedDevice->getName().c_str();
    if (devName.indexOf("OBD") != -1 || devName.indexOf("Vlink") != -1 ||
        devName.indexOf("Viecar") != -1 || devName.indexOf("IOS-Vlink") != -1) {
        NimBLEDevice::getScan()->stop();
        _pServerAddress = new NimBLEAddress(advertisedDevice->getAddress());
        _doConnect = true;
        _doScan = false;
    }
}

bool OBD2BLE::connectToServer(NimBLEAddress pAddress) {
    _pClient = NimBLEDevice::createClient();
    if (!_pClient->connect(pAddress)) {
        return false;
    }

    NimBLERemoteService* pRemoteService = _pClient->getService(OBD_SERVICE_UUID);
    if (pRemoteService == nullptr) {
        // Fallback to generic FFE0 or Custom
        pRemoteService = _pClient->getService("ffe0");
        if (pRemoteService == nullptr) {
            _pClient->disconnect();
            return false;
        }
    }

    _pWriteChar = pRemoteService->getCharacteristic(OBD_CHAR_WRITE_UUID);
    if (!_pWriteChar) _pWriteChar = pRemoteService->getCharacteristic("ffe1");

    _pNotifyChar = pRemoteService->getCharacteristic(OBD_CHAR_NOTIFY_UUID);
    if (!_pNotifyChar) _pNotifyChar = pRemoteService->getCharacteristic("ffe1");

    if (_pNotifyChar && _pNotifyChar->canNotify()) {
        _pNotifyChar->subscribe(true, [](NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t len, bool isNotify) {
            String resp = "";
            for (size_t i = 0; i < len; i++) resp += (char)pData[i];
            OBD2BLE::instance().parseResponse(resp);
        });
    }

    // Initialize ELM327
    sendQuery("AT Z\r");
    delay(100);
    sendQuery("AT E0\r");
    delay(50);
    sendQuery("AT S0\r");
    delay(50);
    sendQuery("AT SP 0\r");

    _telemetry.connected = true;
    _telemetry.last_update = millis();
    return true;
}

void OBD2BLE::sendQuery(const char* cmd) {
    if (_pWriteChar && _pClient && _pClient->isConnected()) {
        _pWriteChar->writeValue((uint8_t*)cmd, strlen(cmd), false);
    }
}

void OBD2BLE::parseResponse(const String& resp) {
    String r = resp;
    r.replace(" ", "");
    r.replace("\r", "");
    r.replace("\n", "");
    r.replace(">", "");

    if (r.startsWith("410D")) { // Vehicle Speed
        long val = strtol(r.substring(4, 6).c_str(), NULL, 16);
        _telemetry.speed = (uint16_t)val;
        _telemetry.last_update = millis();
    } else if (r.startsWith("410C")) { // RPM
        long a = strtol(r.substring(4, 6).c_str(), NULL, 16);
        long b = strtol(r.substring(6, 8).c_str(), NULL, 16);
        _telemetry.rpm = (uint16_t)(((a * 256) + b) / 4);
        _telemetry.last_update = millis();
    } else if (r.startsWith("4105")) { // Coolant Temp
        long a = strtol(r.substring(4, 6).c_str(), NULL, 16);
        _telemetry.coolant_temp = (int16_t)(a - 40);
        _telemetry.last_update = millis();
    } else if (r.indexOf("V") != -1) { // ATRV battery voltage (e.g. "12.4V")
        int idx = r.indexOf("V");
        if (idx > 0) {
            String vStr = r.substring(0, idx);
            _telemetry.battery_voltage = vStr.toFloat();
            _telemetry.last_update = millis();
        }
    }
}

void OBD2BLE::update() {
    if (!ConfigManager::instance().config().obd_enabled) return;

    if (_doScan) {
        NimBLEScan* pScan = NimBLEDevice::getScan();
        pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
        pScan->setInterval(45);
        pScan->setWindow(15);
        pScan->setActiveScan(true);
        pScan->start(ConfigManager::instance().config().obd_scan_duration, false);
        _doScan = false;
    }

    if (_doConnect && _pServerAddress) {
        if (connectToServer(*_pServerAddress)) {
            _doConnect = false;
        } else {
            _doConnect = false;
            _doScan = true; // Retry scan
        }
    }

    if (_telemetry.connected && _pClient && _pClient->isConnected()) {
        if (millis() - _lastPollTime >= 200) {
            _lastPollTime = millis();
            switch (_pollState) {
                case 0: sendQuery("010D\r"); break; // Speed
                case 1: sendQuery("010C\r"); break; // RPM
                case 2: sendQuery("0105\r"); break; // Coolant
                case 3: sendQuery("ATRV\r"); break; // Voltage
            }
            _pollState = (_pollState + 1) % 4;
        }
    } else if (_telemetry.connected) {
        _telemetry.connected = false;
        _doScan = true;
    }
}
