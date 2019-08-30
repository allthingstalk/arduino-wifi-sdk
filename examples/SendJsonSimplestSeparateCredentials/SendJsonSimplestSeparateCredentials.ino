/* AllThingsTalk Arduino SDK
 * https://github.com/allthingstalk/arduino-sdk-v2
 *
 * About this example:
 * Shows the minimal code required to connect to Wi-Fi and send a message to your AllThingsTalk Maker.
 * Sends a "Hello World!" message upon boot.
 * Uses the "keys.h" file that separates credentials from the sketch.
 * 
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create a Sensor asset of type String on your AllThingsTalk Maker
 * - Change the following things in this example: 
 *   Sensor-Asset
 * - Edit the "keys.h" file (second tab of your Arduino IDE) and enter your credentials there.
 */
 
#include <AllThingsTalk.h>
#include "keys.h"

auto wifiCreds   = WifiCredentials(WIFI_SSID, WIFI_PASSWORD); // Edit these in the "keys.h" file
auto deviceCreds = DeviceConfig(DEVICE_ID, DEVICE_TOKEN);     // Edit these in the "keys.h" file
auto device      = Device(wifiCreds, deviceCreds);            // Create "device" object

void setup() {
  device.init();                                 // Initialize AllThingsTalk
  device.send("Sensor-Asset", "Hello World!");   // Send "Hello World!" to asset named "Sensor-Asset" (change this to fit your asset name)
}

void loop() {
  device.loop();    // Keeps the WiFi and AllThingsTalk connection alive (optional in this example)
}