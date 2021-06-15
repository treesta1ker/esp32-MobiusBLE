/*!
 * Discover Mobius Devices
 *
 * This example shows some debugging and discovering methods for Mobius devices.
 * First this will scans for BLE enabled Mobius devices (expecting just one). Once
 * a device is discovered this will attempt connecting to the device. After 
 * successfully connecting this will:
 *   1. read the current "scene" ID
 *   2. start the default feed "scene"
 *   3. read the feed "scene" ID (should be 1)
 *   4. start the normal schedule
 *   5. read the current "scene" ID (should be 0)
 * 
 * 
 * This example code is released into the public domain.
 */

#include <ESP32_MobiusBLE.h>
#include "ArduinoSerialDeviceEventListener.h"

// define a device buffer to hold found Mobius devices
MobiusDevice deviceBuffer[10];

/*!
 * Main Setup method
 */
void setup() {
  // connect the serial port for logs
  Serial.begin(115200);
  while (!Serial);

  // Initialize the library with a useful event listener
  MobiusDevice::init(new ArduinoSerialDeviceEventListener());

  // find nearby Mobius devices
  int count = 0;
  int scanDuration = 5; // in seconds
  while (!count) {
    count = MobiusDevice::scanForMobiusDevices(scanDuration, deviceBuffer);
  }

  // check all the devices were found
  int expectedDevices = 1;
  if (count != expectedDevices) {
    Serial.println("Failed find all the Mobius devices");
  }
}


/*!
 * Main Loop method
 */
void loop() {

  bool discovered = false;
  MobiusDevice device = deviceBuffer[0];

  Serial.println("Connect to the first device");
  while(!discovered) {
    if (device.connect()) {
      // device is now connected

      // find the current scene ID
      uint16_t sceneId = device.getCurrentScene();
      Serial.printf("Current scene:%d\n", sceneId);

      // delaying without sleeping
      unsigned long startMillis = millis();
      while (1000 > (millis() - startMillis)) {}

      // start the default feed scene (should be scene 1)
      device.setFeedScene();
      sceneId = device.getCurrentScene();
      Serial.printf("Feed scene:%d\n", sceneId);

      // delaying without sleeping
      startMillis = millis();
      while (1000 > (millis() - startMillis)) {}

      // start the normal schedule (should be scene 0)
      device.runSchedule();
      sceneId = device.getCurrentScene();
      Serial.printf("Feed scene:%d\n", sceneId);

      // disconnect from the device
      Serial.println("Disconnect from the first device");
      device.disconnect();
      discovered = true;
    }
  }

  Serial.println("Discovering complete");
  while (1) { 
    // stop discovering
  }
}
