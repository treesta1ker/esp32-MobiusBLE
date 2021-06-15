/*!
 * This file is part of the MobiusBLE library.
 */

#include "MobiusDevice.h"
#include "MobiusCRC.h"
#include "DefaultDeviceEventListener.h"
#include <mutex>

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include <esp32-hal-log.h>
#define LOG_TAG ""
#else
#include <esp_log.h>
static const char* LOG_TAG = "MobiusDevice";
#endif

// from Arduino
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))


/**!
 * Implement BLEAdvertisedDeviceCallbacks to count the number of
 * Mobius devices found during scanning and stop scanning early
 * if the expected number of devices is reached.
 */
uint8_t MobiusDevice::MobiusDeviceScanCallbacks::_expectedDevices = 0;
uint8_t MobiusDevice::MobiusDeviceScanCallbacks::_foundDevices = 0;
/**!
 * Called for each advertising BLE server.
 */
void MobiusDevice::MobiusDeviceScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    std::string deviceString = advertisedDevice.toString();
    ESP_LOGD(LOG_TAG, "- BLE Advertised Device found: %s", deviceString.c_str());
    // found a device, so check for the service
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(Mobius::GENERAL_SERVICE)) {
        // update the number of Mobius devices found
        _foundDevices++;
        ESP_LOGD(LOG_TAG, "- Mobius BLE device found: %s", deviceString.c_str());
        if (_foundDevices >= _expectedDevices) {
            ESP_LOGD(LOG_TAG, "- Stopping scanner early");
            BLEDevice::getScan()->stop();
        }
    }
}

// static MobiusDevice variables
MobiusDeviceEventListener* MobiusDevice::_listener = nullptr;
uint8_t* MobiusDevice::_responseData = nullptr;
size_t   MobiusDevice::_responseDataSize = 0;
bool MobiusDevice::_responseUnread = false;
/*
 * Mutex for performing a call and reading the response
 */
std::mutex _callMutex;
/*
 * Mutex for reading/writing to the (static) response data
 */
std::mutex _responseDataMutex;


/*!
 * @brief Scan for Mobius devices
 * 
 * Performs a scan for nearby BLEDevices which have are advertising
 * the GENERAL_SERVICE (i.e. the "MOBIUS" service). Any found devices
 * will be added the give 'deviceBuffer'. Once the 'expectedCount' is
 * reached, scanning will cease regardless of how much time is left
 * until 'scanDuration'.
 * 
 * @param scanDuration maximum scan time (in seconds)
 * @param deviceBuffer buffer to hold all found devices
 * @param expectedCount number of devices expected to be found (default 1)
 * @return number of found devices (number of MobiusDevice added)
 */
uint8_t MobiusDevice::scanForMobiusDevices(uint32_t scanDuration, MobiusDevice* deviceBuffer, uint8_t expectedCount) {
    MobiusDevice::_listener->onEvent(MobiusDeviceEvent::scanning_begin);
    // reset the scanning counts
    MobiusDevice::MobiusDeviceScanCallbacks::_expectedDevices = expectedCount;
    MobiusDevice::MobiusDeviceScanCallbacks::_foundDevices = 0;
    uint8_t count = 0;
    ESP_LOGI(LOG_TAG, "- Scanning for BLE devices");
    // get the singleton BLEScan object
    BLEScan* scanner = BLEDevice::getScan();
    BLEScanResults results = scanner->start(scanDuration, false);
    int deviceCount = results.getCount();
    for (int i = 0; i < deviceCount; i++) {
        BLEAdvertisedDevice advertisedDevice = results.getDevice(i);
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(Mobius::GENERAL_SERVICE)) {
            deviceBuffer[count++] = MobiusDevice(new BLEAdvertisedDevice(advertisedDevice));
            ESP_LOGD(LOG_TAG, "- Updated deviceBuffer with: %s", advertisedDevice.toString().c_str());
        }
    }
    // delete any results fromBLEScan buffer to release memory
    scanner->clearResults();
    MobiusDevice::_listener->onEvent(MobiusDeviceEvent::scanning_end);
    ESP_LOGD(LOG_TAG, "- Expecting to find %d devices; found %d", expectedCount, count);
    return count;
}

/*!
 * @brief Prepares the MobiusDevice class for usage.
 * 
 * Prepares all internal services and utilities for handling
 * BLE communication with Mobius devices.
 *
 * @param optional MobiusDeviceEventListener to use for event listening
 */
void MobiusDevice::init(MobiusDeviceEventListener* listener) {
    // initialize the BLE library
    BLEDevice::init("");
    
    // initialize the singleton BLEScan object
    BLEScan* scanner = BLEDevice::getScan();
    scanner->setInterval(1349);
    scanner->setWindow(449);
    scanner->setActiveScan(true);
    scanner->setAdvertisedDeviceCallbacks(new MobiusDeviceScanCallbacks());
    
    // initialize the handler
    if (nullptr == listener) {
        MobiusDevice::_listener = new DefaultDeviceEventListener();
    } else {
        MobiusDevice::_listener = listener;
    }
}


/*!
 * WARNING: Due to the BLERemoteCharacteristic API, this static function will handle ALL
 *          received notifications regardless of which MobiusDevice instance the message
 *          is intended for. To avoid messages from being read by the wrong MobiusDevice
 *          instance, it is recommended to limit the number of instance actively
 *          connected at any given time.
 *
 * Receives notify messages from devices and writes data from 'RESPONSE_CHARACTERISTIC_2'
 * to '_responseData' for other threads to read.
 */
void MobiusDevice::notifyCallback(BLERemoteCharacteristic* responseCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    MobiusDevice::_listener->onEvent(MobiusDeviceEvent::notification_received);
    BLEUUID uuid = responseCharacteristic->getUUID();
    ESP_LOGD(LOG_TAG, "- Received response from characteristic %s", uuid.toString().c_str());
    
    ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, pData, length, ESP_LOG_DEBUG);
    if (uuid.equals(Mobius::RESPONSE_CHARACTERISTIC_2)) {
        // get mutex lock to update response data safely
        _responseDataMutex.lock();
        // data has been read, so start fresh
        delete MobiusDevice::_responseData;
        // set new response data
        MobiusDevice::_responseData = new uint8_t[length];
        memcpy(MobiusDevice::_responseData, pData, length);
        MobiusDevice::_responseDataSize = length;
        MobiusDevice::_responseUnread = true;
        _responseDataMutex.unlock();
    } else {
        ESP_LOGW(LOG_TAG, "- Received unexpected response on %s", uuid.toString().c_str());
        ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, pData, length, ESP_LOG_DEBUG);
    }
}




/*!
 * Default constructor.
 */
MobiusDevice::MobiusDevice() : MobiusDevice::MobiusDevice(nullptr) {}
/*!
 * Main constructor to build a MobiusDevice for use.  While a public method, 
 * this should only be used by the MobiusDevice itself.
 */
MobiusDevice::MobiusDevice(BLEAdvertisedDevice* device) {
    _device = device;
    _client = nullptr;
    _requestCharacteristic   = nullptr;//TX_FINAL
    _responseCharacteristic1 = nullptr;//RX_DATA
    _responseCharacteristic2 = nullptr;//RX_FINAL
    _messageId = 0;
}
/*!
 * De-construct the class.
 */
MobiusDevice::~MobiusDevice() {
    disconnect();
}

/*!
 * @brief Connect to the device.
 * 
 * Connect to the device corresponding to the current BLEAdvertisedDevice and
 * verify it has the required BLE characteristics.
 * 
 * @return true only if successfully connected
 */
bool MobiusDevice::connect() {
    // rest the message count/ID
    // starting with 2, because why not?
    _messageId = 2;
    MobiusDevice::_listener->onEvent(MobiusDeviceEvent::connection_begin);
    BLEClient* client  = BLEDevice::createClient();
    std::string addressString = _device->getAddress().toString();
    ESP_LOGD(LOG_TAG, "- Connecting to %s", addressString.c_str());
    client->connect(_device);
    
    BLERemoteService* remoteService = client->getService(Mobius::GENERAL_SERVICE);
    
    if (nullptr == remoteService) {
        ESP_LOGW(LOG_TAG, "- Failed to find service on %s", addressString.c_str());
        client->disconnect();
        delete(client);
        MobiusDevice::_listener->onEvent(MobiusDeviceEvent::connection_failure);
    } else {
        // connected to the device with general service
        if (connectToCharacteristics(remoteService)) {
            _client = client;
            _service = remoteService;
            ESP_LOGD(LOG_TAG, "- Connected successfully to %s", addressString.c_str());
            MobiusDevice::_listener->onEvent(MobiusDeviceEvent::connection_successful);
        } else {
            ESP_LOGW(LOG_TAG, "- Failed to connect to characteristics");
            MobiusDevice::_listener->onEvent(MobiusDeviceEvent::connection_failure);
            delete(client);
        }
    }
    return (nullptr != _client);
}
/*!
 * @brief Disconnect from the device.
 *
 * Disconnect from the currently connected device.
 */
void MobiusDevice::disconnect() {
    if (_client) {
        ESP_LOGD(LOG_TAG, "- Disconnecting from client");
        _client->disconnect();
        delete(_client);
        // destroying a client destroys the services
        // and destroying a service destroys the characteristics
        _client = nullptr;
        _service = nullptr;
        _requestCharacteristic = nullptr;
        _responseCharacteristic1 = nullptr;
        _responseCharacteristic2 = nullptr;
    }
}
/*!
 * @brief Get the currently running scene.
 *
 * Query the device to determine the currently running scene.
 *
 * @return an unsigned short
 */
uint16_t MobiusDevice::getCurrentScene() {
    uint16_t scene = -1;
    uint16_t bodySize;
    uint16_t attSize = sizeof Mobius::ATTRIBUTE_CURRENT_SCENE;
    uint8_t attributes[attSize];
    memcpy(attributes, Mobius::ATTRIBUTE_CURRENT_SCENE, attSize);

    uint8_t* body = getData(attributes, attSize, bodySize);
    if (0 < bodySize) {
        // data was retrieved
        scene = body[7];
        scene = (scene << 8) + (body[6]);
    }
    delete[] body;
    return scene;
}
/*!
 * @brief Set a new scene.
 *
 * Sends a set scene request with the given 'sceneId' and verify
 * the response indicates a successful set action.
 *
 * @return true if the 'set' was successful
 */
bool MobiusDevice::setScene(uint16_t sceneId) {
    uint16_t attSize = sizeof Mobius::ATTRIBUTE_SCENE;
    uint8_t attributes[attSize];
    memcpy(attributes, Mobius::ATTRIBUTE_SCENE, attSize);
    // update scene ID portion of the attribute
    attributes[5] = lowByte(sceneId); // little endian
    attributes[6] = highByte(sceneId);// little endian

    return setData(attributes, attSize);
}
/*!
 * @brief Set the default feed scene.
 *
 * Sends a set scene request with the default feed scene ID and
 * verify the response indicates a successful set action.
 *
 * @return true if the 'set' was successful
 */
bool MobiusDevice::setFeedScene() {
    return setScene(Mobius::FEED_SCENE_ID);
}
/*!
 * @brief Run the schedule.
 *
 * Sends a request to set the device into the schedule operational
 * state and verify the response indicates a successful action.
 *
 * @return true if the action was successful
 */
bool MobiusDevice::runSchedule() {
    uint16_t attSize = sizeof Mobius::ATTRIBUTE_OPERATION_STATE;
    uint8_t attributes[attSize];
    memcpy(attributes, Mobius::ATTRIBUTE_OPERATION_STATE, attSize);
    // update which state to set
    attributes[attSize-1] = Mobius::OPERATION_STATE_SCHEDULE;
    
    return setData(attributes, attSize);
}



/*!
 * @brief Connect to relevant characteristics
 * 
 * Connect to the relevant characteristics on the given BLE service for sending
 * and receiving messages.
 * - REQUEST_CHARACTERISTIC must be found and writable
 * - RESPONSE_CHARACTERISTIC_1 must be found and subscribed to
 * - RESPONSE_CHARACTERISTIC_2 must be found and subscribed to
 *
 * @return true only if all the required characteristics are connected/ready
 */
bool MobiusDevice::connectToCharacteristics(BLERemoteService* service) {
    // setup the request characteristic
    _requestCharacteristic = service->getCharacteristic(Mobius::REQUEST_CHARACTERISTIC);
    bool hasRequestChar = _requestCharacteristic && _requestCharacteristic->canWriteNoResponse();
    ESP_LOGD(LOG_TAG, "- hasRequestChar:%s", (hasRequestChar ? "true" : "false"));
    // setup the first response characteristic (the one which is used)
    _responseCharacteristic1 = service->getCharacteristic(Mobius::RESPONSE_CHARACTERISTIC_1);
    bool hasResponseChar1 = _responseCharacteristic1 && _responseCharacteristic1->canNotify();
    ESP_LOGD(LOG_TAG, "- hasResponseChar1:%s", (hasResponseChar1 ? "true" : "false"));
    if (hasResponseChar1) {
        // setup the notify callback so full responses can be read
        _responseCharacteristic1->registerForNotify(notifyCallback);
        //addressing issue in BLERemoteCharacteristic (missing response true)
        // similar to https://github.com/nkolban/esp32-snippets/issues/397
        uint8_t notificationOn[]={0x01, 0x00};
        _responseCharacteristic1->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue(notificationOn, 2, true);
    }
    // setup the second response characteristic
    _responseCharacteristic2 = service->getCharacteristic(Mobius::RESPONSE_CHARACTERISTIC_2);
    bool hasResponseChar2 = _responseCharacteristic2 && _responseCharacteristic2->canNotify();
    ESP_LOGD(LOG_TAG, "- hasResponseChar2:%s", (hasResponseChar2 ? "true" : "false"));
    if (hasResponseChar2) {
        // setup the notify callback so full responses can be read
        _responseCharacteristic2->registerForNotify(notifyCallback);
        //addressing issue in BLERemoteCharacteristic (missing response true)
        // similar to https://github.com/nkolban/esp32-snippets/issues/397
        uint8_t notificationOn[]={0x01, 0x00};
        _responseCharacteristic2->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue(notificationOn, 2, true);
    }
    
    return hasRequestChar && hasResponseChar1 && hasResponseChar2;
}
/*!
 * Send a "set" request with the given 'data' (of size 'length').
 *
 * @return true if verification was requested and the response was valid,
 * or if verification was skipped
 */
bool MobiusDevice::setData(uint8_t* data, uint16_t length, bool doVerification) {
    // build a request to SET data on a device
    uint16_t reqSize;
    uint8_t* req = buildRequest(data, length, Mobius::OP_CODE_SET, 0x0800, reqSize);
    uint16_t resSize;
    _callMutex.lock();
    uint8_t* res = sendRequest(req, reqSize, resSize);

    bool verified = false;
    // verify the response
    if (0 < resSize && doVerification) {
        verified = responseSuccessful(req, reqSize, res, resSize);
    }
    // cleanup sent request & returned response from memory
    delete[] req;
    delete[] res;
    _callMutex.unlock();
    return verified || !doVerification;
}
/*!
 * Send a "get" request with the given 'data' (of size 'length') and parse
 * out the data portion of the response.
 * Sets the value in the given 'dataSize' address to the data's total size.
 *
 * @return the a pointer to the byte array (data)
 */
uint8_t* MobiusDevice::getData(uint8_t* data, uint16_t length, uint16_t& dataSize) {
    // build a request to GET data on a device
    uint16_t reqSize;
    uint8_t* req = buildRequest(data, length, Mobius::OP_CODE_GET, 0x0000, reqSize);
    uint16_t resSize;
    _callMutex.lock();
    uint8_t* res = sendRequest(req, reqSize, resSize);
    // current assumes the response is for the current request
    uint8_t* responseData = parseResponseData(res, resSize, dataSize);
    // cleanup sent request & returned response from memory
    delete[] req;
    delete[] res;
    _callMutex.unlock();
    return responseData;
}
/*!
 * Build a byte array representing a Mobius request message.
 * Sets the value in the given 'requestSize' address to the request's total size.
 *
 * @return a pointer to the byte array (request)
 */
uint8_t* MobiusDevice::buildRequest(uint8_t* data, uint16_t length, uint8_t opCode, uint16_t reserved, uint16_t& requestSize) {
    requestSize = length + 11;
    uint8_t* request = new uint8_t[requestSize];

    // first byte is always 02
    request[0] = 0x02;
    // opGroup
    request[1] = Mobius::OP_GROUP_REQUEST;  // C2CI_Request
    // opCode
    request[2] = opCode;
    // mMessageId
    request[3] = lowByte(_messageId); // little endian
    request[4] = highByte(_messageId);// little endian
    _messageId++;
    // mReserved
    request[5] = highByte(reserved);
    request[6] = lowByte(reserved);
    // data size
    request[7] = lowByte(length); // little endian
    request[8] = highByte(length);// little endian
    // data
    for (uint16_t i = 0; i < length; i++) {
        request[9 + i] = data[i];
    }

    short crc = MobiusCRC::crc16(&request[1], requestSize - 3);
    request[requestSize - 2] = (uint8_t)crc;// lowByte(length)
    request[requestSize - 1] = (uint8_t)(crc >> 8); // highByte(length)

    ESP_LOGW(LOG_TAG, "- built request is:");
    ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, request, requestSize, ESP_LOG_DEBUG);
    return request;
}
/*!
 * Writes the given 'request' (of size 'length') to the request characteristic.
 * Sets the value in the given 'responseSize' address to the response's total size.
 * Max response size is currently 255.
 *
 * @return a pointer to the byte array (response)
 */
uint8_t* MobiusDevice::sendRequest(uint8_t* request, uint16_t length, uint16_t& responseSize) {
    ESP_LOGD(LOG_TAG, "- data being sent:");
    ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, request, length, ESP_LOG_DEBUG);
    // setup response info
    responseSize = 0;
    uint8_t* response = new uint8_t[0];
    // get a copy of the request to write
    uint8_t toSend[length];
    memcpy(toSend, request, length);
    
    // reset the unread flag to avoid reading old data
    _responseDataMutex.lock();
    MobiusDevice::_responseUnread = false;
    _responseDataMutex.unlock();
    
    // do the actual writing to the characteristic
    if (_requestCharacteristic->writeValue(toSend, length)) {
        ESP_LOGD(LOG_TAG, "- data sent successfully");
        MobiusDevice::_listener->onEvent(MobiusDeviceEvent::request_successful);
        // look for a response
        bool received = false;
        ESP_LOGD(LOG_TAG, "- waiting for response");
        // wait for at most 1 second, with 100 ms read/check delay
        uint32_t oneSecond = 1000000;
        uint32_t readDelay =  100000;
        int64_t startMicro = esp_timer_get_time();
        while (!received && oneSecond > (esp_timer_get_time() - startMicro)) {
            // busy wait to avoid flooding with read requests and constant locking
            int64_t delayStartMicro = esp_timer_get_time();
            while(readDelay > (esp_timer_get_time() - delayStartMicro)) {}
            
            _responseDataMutex.lock();
            if (MobiusDevice::_responseUnread) {
                received = true;
                delete[] response;
                // new data to read
                responseSize = _responseDataSize;
                response = new uint8_t[responseSize];
                memcpy(response, _responseData, responseSize);
                // update as read
                MobiusDevice::_responseUnread = false;
                
                ESP_LOGD(LOG_TAG, "- response data was received:");
                ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, response, responseSize, ESP_LOG_DEBUG);
            }
            _responseDataMutex.unlock();
        }
    } else {
        ESP_LOGW(LOG_TAG, "- Failed to send the request");
        MobiusDevice::_listener->onEvent(MobiusDeviceEvent::request_failure);
    }
    return response;
}
/*!
 * Parse the response to get extract the data.
 * Sets the value in the given 'dataSize' address to the data's total size.
 *
 * @return a pointer to the byte array (data)
 */
uint8_t* MobiusDevice::parseResponseData(uint8_t* response, uint16_t length, uint16_t& dataSize) {
    // setup default data info
    dataSize = 0;
    uint8_t* data = new uint8_t[0];
    // check response info
    bool isValid = (length > 11);
    isValid = isValid && (0x02 == response[0]);
    isValid = isValid && (Mobius::OP_GROUP_CONFIRM == response[1]);
    if (isValid) {
        // get the data 
        dataSize = (response[8] << 8) + (response[7]);
        // cleanup data and copy the data to be returned
        delete[] data;
        data = new uint8_t[dataSize];
        memcpy(data, &response[9], dataSize);
        ESP_LOGD(LOG_TAG, "- response data was valid and parsed into:");
        ESP_LOG_BUFFER_HEXDUMP(LOG_TAG, data, dataSize, ESP_LOG_DEBUG);
    } else {
        ESP_LOGW(LOG_TAG, "- response data was invalid");
    }
    return data;
}
/*!
 * Validate the given 'response' (of size 'resSize') for the given 'request' (of size 'reqSize').
 *
 * @return true only if the response is a success message for the request
 */
bool MobiusDevice::responseSuccessful(uint8_t* request, uint16_t reqSize, uint8_t* response, uint16_t resSize) {
    bool idValid = false;
    bool dataSuccess = false;
    bool lengthsValid = (reqSize > 11) && (resSize > 11);
    if (lengthsValid) {
        // check first 5 bytes (which should match)
        idValid = (request[0] == response[0]);
        idValid = idValid && (response[1] == Mobius::OP_GROUP_CONFIRM); // C2CI_Confirm
        idValid = idValid && (request[2] == response[2]);
        idValid = idValid && (request[3] == response[3]);
        idValid = idValid && (request[4] == response[4]);
        // check the data
        int dataSize = (response[8] << 8) + (response[7]);
        uint8_t resData[dataSize];
        dataSuccess = (3 == dataSize);
        dataSuccess = dataSuccess && (0x00 == response[9]); // all response data starts with 0x00
        for (int i = 0; dataSuccess && i < dataSize - 1; i++) {
            resData[i] = response[10 + i];
            dataSuccess = dataSuccess && response[10 + i] == Mobius::RESPONSE_DATA_SUCCESSFUL[i];
        }
    }
    // check CRC of the response
    // skipping CRC validation for now because it seems to be different
    // Mobius app doesn't seem to be checking either
    //  short crc = crc16(&response[1], resSize-3);
    //  bool crcValid = (response[resSize-2] == (byte) crc) && (response[resSize-1] = (byte) (crc >> 8));
    ESP_LOGD(LOG_TAG, "- lengthsValid: %s", (lengthsValid ? "true" : "false"));
    ESP_LOGD(LOG_TAG, "- idValid: %s", (idValid ? "true" : "false"));
    ESP_LOGD(LOG_TAG, "- idValiddataSuccess: %s", (dataSuccess ? "true" : "false"));
    // skipping the CRC validation for now
    bool responseSuccessful = lengthsValid && idValid /*&& crcValid*/ && dataSuccess;
    if (responseSuccessful) {
        MobiusDevice::_listener->onEvent(MobiusDeviceEvent::response_successful);
    } else {
        MobiusDevice::_listener->onEvent(MobiusDeviceEvent::response_failure);
    }
    return responseSuccessful;
}
