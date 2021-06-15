/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#ifndef _ArduinoSerialDeviceEventListener_h
#define _ArduinoSerialDeviceEventListener_h

#include "MobiusDeviceEventListener.h"

/*!
 * @brief A MobiusDeviceEventListener backed by the Arduino Serial port.
 * 
 * This implementation of MobiusDeviceEventListener shall write strings
 * to the Serial stream for each MobiusDeviceEvents. Thus acting as a
 * type of logging feature.
 */
class ArduinoSerialDeviceEventListener : public MobiusDeviceEventListener {
public:
    ArduinoSerialDeviceEventListener();
    ~ArduinoSerialDeviceEventListener();
    
    /*!
     * @brief Write a message to the Serial stream.
     * 
     * @param event MobiusDeviceEvent
     */
    void onEvent(MobiusDeviceEvent event);
};
#endif
