# ESP32 MobiusBLE
Enables connectivity with and control of Mobius devices.

This library supports BLE communication with BLE enabled Mobius devices using the [ESP32_BLE_Arduino](https://www.arduino.cc/reference/en/libraries/esp32-ble-arduino) library (see [Dependencies](#dependencies) section for more inforation). Thus ESP32-MobiusBLE can be used on ESP32 boards developed with the Arduino IDE.


## Dependencies
ESP32-MobiusBLE (specifically `MobiusDevice`) is heavily dependent upon the [ESP32_BLE_Arduino](https://www.arduino.cc/reference/en/libraries/esp32-ble-arduino) library. However, the "official" Arduino library isn't up to date so a different version was needed. Follow the steps to [build a new version](https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/ArduinoBLE.md) of the library.

The `ArduinoSerialDeviceEventListener` is the only class which has an Arduino dependency.

The `FastLEDDeviceEventListener` depends on the [FastLED](https://www.arduino.cc/reference/en/libraries/fastled) library.


## Listeners
| Event                  | DefaultDeviceEventListener | ArduinoSerialDeviceEventListener | FastLEDDeviceEventListener |
|------------------------|----------------------------|----------------------------------|----------------------------|
| `scanning_begin`       | logged with `ESP_LOGD`     | logged with `Serial.println`     | solid blue                 |
| `scanning_end`         | logged with `ESP_LOGD`     | logged with `Serial.println`     | turns off LED(s)           |
| `connection_begin`     | logged with `ESP_LOGD`     | logged with `Serial.println`     | solid green                |
| `connection_failure`   | logged with `ESP_LOGD`     | logged with `Serial.println`     | blink red slow             |
| `connection_successful`| logged with `ESP_LOGD`     | logged with `Serial.println`     | turns off LED(s)           |
| `notification_received`| logged with `ESP_LOGD`     | logged with `Serial.println`     | nothing                    |
| `request_successful`   | logged with `ESP_LOGD`     | logged with `Serial.println`     | turns off LED(s)           |
| `request_failure`      | logged with `ESP_LOGD`     | logged with `Serial.println`     | blink red fast             |
| `response_successful`  | logged with `ESP_LOGD`     | logged with `Serial.println`     | blink white                |
| `response_failure`     | logged with `ESP_LOGD`     | logged with `Serial.println`     | blink orange               |


## Examples
#### Discover
This example shows some debugging and discovering methods for Mobius devices. First it will scan for BLE enabled Mobius devices (expecting just one). Once a device is discovered it will attempt connecting to the device. After successfully connecting it will:
1. read the current "scene" ID
2. start the default feed "scene"
3. read the feed "scene" ID (should be 1)
4. start the normal schedule
5. read the current "scene" ID (should be 0)
#### Control
This example shows how a Mobius device may be controlled with an analog signal. First it will scan for BLE enabled Mobius devices (expecting just one). Once the device is discovered it will check the analog PIN (GPIO_NUM_33) every 2 seconds for the current state. When a new state is detected it will connect to the Mobius device and set the scene corresponding to the state.

## Troubleshooting
To help troubleshoot [switch on debugging](https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/ArduinoBLE.md#switching-on-debugging) within the IDE.

If the board seems to get stuck while trying to connect, it may be due to [this issue](https://github.com/nkolban/esp32-snippets/issues/874). The workaround for this is to manually update the FreeRTOS.cpp file as described in the issue (i.e. replace all usages of `portMAX_DELAY` used in `xSemaphoreTake` calls with a reasonable value such as `15000UL`).


## License
```
MIT License

Copyright (c) 2021 treesta1ker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

