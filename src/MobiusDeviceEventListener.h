/*!
 * This file is part of the ESP32_MobiusBLE library.
 */

#ifndef _MobiusDeviceEventListener_h
#define _MobiusDeviceEventListener_h

/*!
 * @brief enum for possible MobiusDevice events.
 */
enum class MobiusDeviceEvent { scanning_begin, // scanning is about to start
                               scanning_end,   // scanning has ended
                               connection_begin,      // connecting is about to start
                               connection_failure,    // connecting failed to complete
                               connection_successful, // connecting was successful
                               notification_received, // a notification/message/response was received from a device
                               request_successful,// request was sent to a device
                               request_failure,   // request failed to be sent
                               response_successful,// response indicated a success
                               response_failure    // response indicated a failure
                               };

/*!
 * @brief Mobius interface for listening to MobiusDeviceEvents.
 * 
 * Implementations of this interface must handle MobiusDeviceEvents
 * when they are fired/triggered. Any blocking or errors will
 * affect the source MobiusDevice.
 */
class MobiusDeviceEventListener {
public:
    MobiusDeviceEventListener(){}
    virtual ~MobiusDeviceEventListener(){}

    /*!
     * @brief Handle a MobiusDeviceEvent.
     * 
     * @param event MobiusDeviceEvent
     */
    virtual void onEvent(MobiusDeviceEvent event) = 0;
};
#endif
