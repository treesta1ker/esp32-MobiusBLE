/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#include "ArduinoSerialDeviceEventListener.h"
#include <Arduino.h>

/*!
 * Default constructor.
 */
ArduinoSerialDeviceEventListener::ArduinoSerialDeviceEventListener() {}

/*!
 * De-construct the class.
 */
ArduinoSerialDeviceEventListener::~ArduinoSerialDeviceEventListener() {}

/*!
 * @brief Write a message to the Serial stream.
 * 
 * @param event MobiusDeviceEvent
 */
void ArduinoSerialDeviceEventListener::onEvent(MobiusDeviceEvent event) {
    switch (event) {
    case MobiusDeviceEvent::scanning_begin:
        Serial.println("Starting scan for Mobius devices");
        break;
    case MobiusDeviceEvent::scanning_end:
        Serial.println("Finished scan for Mobius devices");
        break;
    case MobiusDeviceEvent::connection_begin:
        Serial.println("Starting connection to the device");
        break;
    case MobiusDeviceEvent::connection_failure:
        Serial.println("Failed to connect to the device");
        break;
    case MobiusDeviceEvent::connection_successful:
        Serial.println("Successfully connected to the device");
        break;
    case MobiusDeviceEvent::notification_received:
        Serial.println("A notification was received from the device");
        break;
    case MobiusDeviceEvent::request_successful:
        Serial.println("Successfully sent the request to the device");
        break;
    case MobiusDeviceEvent::request_failure:
        Serial.println("Failed to send the request to the device");
        break;
    case MobiusDeviceEvent::response_successful:
        Serial.println("Received a successful response from the device");
        break;
    case MobiusDeviceEvent::response_failure:
        Serial.println("Received an unsuccessful response from the device");
        break;
    }
}

