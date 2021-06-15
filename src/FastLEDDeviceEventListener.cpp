/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#include "FastLEDDeviceEventListener.h"

/*!
 * Default constructor.
 */
FastLEDDeviceEventListener::FastLEDDeviceEventListener() {}

/*!
 * De-construct the class.
 */
FastLEDDeviceEventListener::~FastLEDDeviceEventListener() {}

/*!
 * @brief Blink the LED.
 * 
 * @param event MobiusDeviceEvent
 */
void FastLEDDeviceEventListener::onEvent(MobiusDeviceEvent event) {
    switch (event) {
    case MobiusDeviceEvent::scanning_begin:
        FastLED.showColor(CRGB::Blue);
        break;
    case MobiusDeviceEvent::scanning_end:
        FastLED.showColor(CRGB::Black);
        break;
    case MobiusDeviceEvent::connection_begin:
        FastLED.showColor(CRGB::Green);
        break;
    case MobiusDeviceEvent::connection_failure:
        // failure, blink red a few times
        for (uint8_t i=0; i<4; i++) {
            FastLED.showColor(CRGB::Red);
            delay(850);
            FastLED.showColor(CRGB::Black);
            delay(50);
        }
        break;
    case MobiusDeviceEvent::connection_successful:
        // success, just turn off the green light
        FastLED.showColor(CRGB::Black);
        break;
    case MobiusDeviceEvent::notification_received:
        // do nothing
        break;
    case MobiusDeviceEvent::request_successful:
        // success, just turn off the light
        FastLED.showColor(CRGB::Black);
        break;
    case MobiusDeviceEvent::request_failure:
        for (uint8_t i=0; i<8; i++) {
            FastLED.showColor(CRGB::Red);
            delay(150);
            FastLED.showColor(CRGB::Black);
            delay(60);
        }
        break;
    case MobiusDeviceEvent::response_successful:
        FastLED.showColor(CRGB::White);
        delay(200);
        FastLED.showColor(CRGB::Black);
        break;
    case MobiusDeviceEvent::response_failure:
       for (uint8_t i=0; i<6; i++) {
            FastLED.showColor(CRGB::Orange);
            delay(150);
            FastLED.showColor(CRGB::Black);
            delay(60);
        }
        break;
    }
}

