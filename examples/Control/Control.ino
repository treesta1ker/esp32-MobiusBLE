/*!
 * Control a Mobius Device
 *
 * This example shows how a Mobius device may be controlled with an analog signal.
 * First this will scans for BLE enabled Mobius devices (expecting just one). Once
 * the device is discovered this checks the analog PIN (A0) every 2 seconds for the
 * current state. When a new state is detected this will connect to the Mobius
 * device and set the scene corresponding to the state.
 * 
 * The circuit:
 * - M5Atom
 * 
 * This example code is released into the public domain.
 */
#include <FastLED.h>
#include <ESP32_MobiusBLE.h>
#include "FastLEDDeviceEventListener.h"

// define LED configuration
#define NUM_LEDS 1
#define LED_PIN 27
#define BRIGHTNESS  40
// define array of LEDs
CRGB leds[NUM_LEDS];

// define analog pin which will get the divided 0-10V signal
const int SENSOR_PIN = GPIO_NUM_33;
const gpio_num_t DISABLE_PIN = GPIO_NUM_23;

// define a variable for holding the state, initialized to neutral/normal
byte currentState = 0;
// define a variable for the MobiusDevice to be controlled
MobiusDevice pump;

/*!
 * Main Setup method
 */
void setup() {
  // connect the serial port for logs
  Serial.begin(115200);
  while (!Serial);

  // setup the pins to read analog inputs
  pinMode(SENSOR_PIN, INPUT);
  gpio_pulldown_dis(DISABLE_PIN);
  gpio_pullup_dis(DISABLE_PIN);

  // setup the LED
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(  BRIGHTNESS );
  
  // Initialize the library with a useful event listener
  MobiusDevice::init(new FastLEDDeviceEventListener());

  // define a device buffer to hold found Mobius devices
  MobiusDevice deviceBuffer[5];

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
  
  pump = deviceBuffer[0];
}


/*!
 * Main Loop method
 */
void loop() {
  // get the analog state
  byte newState = analogState();

  if (currentState != newState) {
    // now in a different state, update a maybe do something
    if (1 == newState && pump.connect()) {
      Serial.println("Feed Mode");
      // new state is the feed state AND the device is connected
      if(pump.setFeedScene()) {
        // update the current state so not to re-enter feed state next loop
        currentState = newState;
      }
      // disconnect from the device
      pump.disconnect();
    } else if (2 == newState && pump.connect()) {
      Serial.println("Maintenance Mode");
      // new state is the maintenance state AND the device is connected
      // set the sceneId to the custom/unique ID
      uint16_t sceneId = 1234;
      if(pump.setScene(sceneId)) {
        // update the current state so not to re-enter maintenance state next loop
        currentState = newState;
      }
      // disconnect from the device
      pump.disconnect();
    } else {
      // update the current state
      currentState = newState;
    }
  }
  
  // wait/sleep for 2 seconds before checking the state again
  delay(2000);
}

/*!
 * Determines the current "state" from the monitored SENSOR_PIN.
 * Possible state values:
 *   0 : neutral or normal
 *   1 : feed mode state (~5V)
 *   2 : maintenance mode state (~2V)
 */
byte analogState() {
   // read the ADC value (0 - 4095)
  int sensorValue = analogRead(SENSOR_PIN);
  // calculate voltage, assume a voltage divider is in place
  // dropping a 10V max to a board acceptable 3.3V 
  float voltage = sensorValue * (10.0 / 4095.0);
  Serial.printf("Input Voltage: %.5f", voltage);

  // default state is "neutral"
  byte state = 0;
  if (4.75 <= voltage && voltage <= 5.26) {
    // voltage is within the 5V range
    state = 1;
  } else if (1.75 <= voltage && voltage <= 2.26) {
    // voltage is within the 2V range
    state = 2;
  }
  Serial.printf("\tstate: %d\n", state);
  return state;
}

