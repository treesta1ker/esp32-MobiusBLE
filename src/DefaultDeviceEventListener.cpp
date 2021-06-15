/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#include <string>
#include <sdkconfig.h>
#include "DefaultDeviceEventListener.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include <esp32-hal-log.h>
#define LOG_TAG ""
#else
#include <esp_log.h>
static const char* LOG_TAG = "DefaultDeviceEventListener";
#endif

/*!
 * Default constructor.
 */
DefaultDeviceEventListener::DefaultDeviceEventListener() {}

/*!
 * De-construct the class.
 */
DefaultDeviceEventListener::~DefaultDeviceEventListener() {}

/*!
 * @brief Log the event at DEBUG level.
 * 
 * @param event MobiusDeviceEvent
 */
void DefaultDeviceEventListener::onEvent(MobiusDeviceEvent event) {
    ESP_LOGD(LOG_TAG, "- %s", getEventName(event).c_str());
}

/*
 * @brief Get the event name string.
 *
 * @param event MobiusDeviceEvent
 * @return std::string
 */
std::string DefaultDeviceEventListener::getEventName(MobiusDeviceEvent event) {
    switch (event) {
    case MobiusDeviceEvent::scanning_begin:
        return "scanning_begin";
    case MobiusDeviceEvent::scanning_end:
        return "scanning_end";
    case MobiusDeviceEvent::connection_begin:
        return "connection_begin";
    case MobiusDeviceEvent::connection_failure:
        return "connection_failure";
    case MobiusDeviceEvent::connection_successful:
        return "connection_successful";
    case MobiusDeviceEvent::notification_received:
        return "notification_received";
    case MobiusDeviceEvent::request_successful:
        return "request_successful";
    case MobiusDeviceEvent::request_failure:
        return "request_failure";
    case MobiusDeviceEvent::response_successful:
        return "response_successful";
    case MobiusDeviceEvent::response_failure:
        return "response_failure";
    }
    return "unknown";
}

