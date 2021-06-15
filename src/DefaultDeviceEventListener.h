/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#ifndef _DefaultDeviceEventListener_h
#define _DefaultDeviceEventListener_h

#include "MobiusDeviceEventListener.h"

/*!
 * @brief A MobiusDeviceEventListener which logs using ESP.
 * 
 * This implementation of MobiusDeviceEventListener shall log
 * the events to the ESP_LOG at the DEBUG level.
 */
class DefaultDeviceEventListener : public MobiusDeviceEventListener {
public:
    DefaultDeviceEventListener();
    ~DefaultDeviceEventListener();
    
    /*!
     * @brief Log the event at DEBUG level.
     * 
     * @param event MobiusDeviceEvent
     */
    void onEvent(MobiusDeviceEvent event);

private:
    /*
     * @brief Get the event name string.
     *
     * @param event MobiusDeviceEvent
     * @return std::string
     */
    std::string getEventName(MobiusDeviceEvent event);
};
#endif
