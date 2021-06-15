/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#ifndef _MobiusDevice_h
#define _MobiusDevice_h

#include <cstdint>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "MobiusDeviceEventListener.h"

/*!
 * @brief Namespace containing definitions specific for Mobius communication.
 */
namespace Mobius {
    static const BLEUUID GENERAL_SERVICE("01ff0100-ba5e-f4ee-5ca1-eb1e5e4b1ce0");
    
    static const BLEUUID REQUEST_CHARACTERISTIC(   "01ff0104-ba5e-f4ee-5ca1-eb1e5e4b1ce0");//TX_FINAL
    static const BLEUUID RESPONSE_CHARACTERISTIC_2("01ff0102-ba5e-f4ee-5ca1-eb1e5e4b1ce0");//RX_FINAL
    static const BLEUUID RESPONSE_CHARACTERISTIC_1("01ff0101-ba5e-f4ee-5ca1-eb1e5e4b1ce0");//RX_DATA

    static const uint8_t OP_GROUP_REQUEST = 0xde; // C2CI_Request = -34
    static const uint8_t OP_GROUP_CONFIRM = 0xdf; // C2CI_Confirm = -33
    static const uint8_t OP_CODE_GET = 0x17;      // GetC2AttrFsciRequest
    static const uint8_t OP_CODE_SET = 0x18;      // SetC2AttrFsciRequest
    static const uint8_t ATTRIBUTE_SCENE[] =          { 0x91, 0x01, 0x00, 0x01, 0x04, 0xFF, 0xFF, 0x00, 0x00 }; // C2Attribute.CurrentScene = 401
    static const uint8_t ATTRIBUTE_CURRENT_SCENE[]  = { 0x91, 0x01, 0x00, 0x01 }; // C2Attribute.CurrentScene = 401
    static const uint8_t ATTRIBUTE_OPERATION_STATE[]= { 0x68, 0x00, 0x00, 0x01, 0x01, 0xFF }; // C2Attribute.OperationState = 104
    static const uint8_t RESPONSE_DATA_SUCCESSFUL[] = { 0xFF, 0xFF };
    static const uint8_t OPERATION_STATE_SCHEDULE = 0x03;
    static const uint16_t FEED_SCENE_ID = 1;
}

 /*!
  * @brief Class representing Mobius device.
  *
  * This class represents a Mobius device which may be controlled via BLE communication.
  */
class MobiusDevice {
public:
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
    static uint8_t scanForMobiusDevices(uint32_t scanDuration, MobiusDevice* deviceBuffer, uint8_t expectedCount = 1);

    /*!
     * @brief Prepares the MobiusDevice class for usage.
     * 
     * Prepares all internal services and utilities for handling
     * BLE communication with Mobius devices.
     *
     * @param optional MobiusDeviceEventListener to use for event listening
     */
    static void init(MobiusDeviceEventListener* listener = nullptr);



    /*!
     * Default constructor.
     */
    MobiusDevice();
    /*!
     * Main constructor to build a MobiusDevice for use.  While a public method,
     * this should only be used by the MobiusDevice itself.
     */
    MobiusDevice(BLEAdvertisedDevice* device);

    /*!
     * De-construct the class.
     */
    ~MobiusDevice();

    /*!
     * @brief Connect to the device.
     * 
     * Connect to the device corresponding to the current BLEAdvertisedDevice and
     * verify it has the required BLE characteristics.
     * 
     * @return true only if successfully connected
     */
    bool connect();
    
    /*!
     * @brief Disconnect from the device.
     *
     * Disconnect from the currently connected device.
     */
    void disconnect();

    /*!
     * @brief Get the currently running scene.
     * 
     * Query the device to determine the currently running scene.
     * Or 65535 (-1) if a failure occurs.
     * 
     * @return a uint16_t
     */
    uint16_t getCurrentScene();

    /*!
     * @brief Set a new scene.
     * 
     * Sends a set scene request with the given 'sceneId' and verify
     * the response indicates a successful set action.
     * 
     * @return true if the 'set' was successful
     */
    bool setScene(uint16_t sceneId);

    /*!
     * @brief Set the default feed scene.
     * 
     * Sends a set scene request with the default feed scene ID and 
     * verify the response indicates a successful set action.
     * 
     * @return true if the 'set' was successful
     */
    bool setFeedScene();

    /*!
     * @brief Run the schedule.
     * 
     * Sends a request to set the device into the schedule operational
     * state and verify the response indicates a successful action.
     * 
     * @return true if the action was successful
     */
    bool runSchedule();


private:
    static MobiusDeviceEventListener* _listener;
    static uint8_t* _responseData;
    static size_t   _responseDataSize;
    static bool     _responseUnread;
    static void notifyCallback(BLERemoteCharacteristic* responseCharacteristic, uint8_t* pData, size_t length, bool isNotify);

    /*!
     * A BLEAdvertisedDeviceCallbacks to count the number of Mobius devices
     * found during scanning and stops scanning early if the expected number
     * of devices is reached.
     */
    class MobiusDeviceScanCallbacks : public BLEAdvertisedDeviceCallbacks {
    public:
        /*!
         * Counts to identify when to stop scanning.early.
         */
        static uint8_t _expectedDevices;
        static uint8_t _foundDevices;
        /*!
         * Called for each advertising BLE server.
         */
        void onResult(BLEAdvertisedDevice advertisedDevice);
    };


    BLEAdvertisedDevice* _device;
    BLEClient* _client;
    BLERemoteService* _service;
    BLERemoteCharacteristic* _requestCharacteristic;  //TX_FINAL
    BLERemoteCharacteristic* _responseCharacteristic1;//RX_DATA
    BLERemoteCharacteristic* _responseCharacteristic2;//RX_FINAL
    uint16_t _messageId;


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
    bool connectToCharacteristics(BLERemoteService* service);
    
    /*!
     * Send a "set" request with the given 'data' (of size 'length').
     *
     * @return true if verification was requested and the response was valid,
     * or if verification was skipped
     */
    bool setData(uint8_t* data, uint16_t length, bool doVerification = true);
    
    /*!
     * Send a "get" request with the given 'data' (of size 'length') and parse
     * out the data portion of the response.
     * Sets the value in the given 'dataSize' address to the data's total size.
     *
     * @return the a pointer to the byte array (data)
     */
    uint8_t* getData(uint8_t* data, uint16_t length, uint16_t& dataSize);
    
    /*!
     * Build a byte array representing a Mobius request message.
     * Sets the value in the given 'requestSize' address to the request's total size.
     *
     * @return a pointer to the byte array (request)
     */
    uint8_t* buildRequest(uint8_t* data, uint16_t length, uint8_t opCode, uint16_t reserved, uint16_t& requestSize);
    
    /*!
     * Writes the given 'request' (of size 'length') to the request characteristic.
     * Sets the value in the given 'responseSize' address to the response's total size.
     * Max response size is currently 255.
     * 
     * @return a pointer to the byte array (response)
     */
    uint8_t* sendRequest(uint8_t* request, uint16_t length, uint16_t& responseSize);
    
    /*!
     * Parse the response to get extract the data.
     * Sets the value in the given 'dataSize' address to the data's total size.
     * 
     * @return a pointer to the byte array (data)
     */
    uint8_t* parseResponseData(uint8_t* response, uint16_t length, uint16_t& dataSize);
    
    /*!
     * Validate the given 'response' (of size 'resSize') for the given 'request' (of size 'reqSize').
     *
     * @return true only if the response is a success message for the request
     */
    bool responseSuccessful(uint8_t* request, uint16_t reqSize, uint8_t* response, uint16_t resSize);
};

#endif
