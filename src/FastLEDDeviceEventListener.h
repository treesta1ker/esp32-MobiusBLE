/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#ifndef _FastLEDDeviceEventListener_h
#define _FastLEDDeviceEventListener_h

#include <FastLED.h>
#include "MobiusDeviceEventListener.h"

/*!
 * @brief A MobiusDeviceEventListener which uses FastLED.
 * 
 * This implementation of MobiusDeviceEventListener shall use
 * FastLED to blink different colors for corresponding events.
 */
class FastLEDDeviceEventListener : public MobiusDeviceEventListener {
public:
    FastLEDDeviceEventListener();
    ~FastLEDDeviceEventListener();
    
    /*!
     * @brief Blink the LED.
     * 
     * @param event MobiusDeviceEvent
     */
    void onEvent(MobiusDeviceEvent event);
};
#endif
