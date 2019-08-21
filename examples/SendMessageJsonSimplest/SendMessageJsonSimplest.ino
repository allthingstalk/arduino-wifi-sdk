/* AllThingsTalk Arduino SDK
 * https://github.com/allthingstalk/arduino-sdk-v2
 *
 * About this example: 
 * Shows the minimal code required to connect to Wi-Fi and send a message to your AllThingsTalk Maker
 * Sends a "Hello World!" message upon boot.
 *
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create a Sensor asset of type String on your AllThingsTalk Maker
 * - Change the following things in this example to make it work: 
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Sensor-Asset
 */

#include <AllThingsTalk.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object

void setup() {
  device.init();                                 // Initialize AllThingsTalk
  device.send("Sensor-Asset", "Hello World!");   // Send "Hello World!" to asset named "Sensor-Asset" (change this to fit your asset name)
}

void loop() {
  device.loop();    // Keeps the WiFi and AllThingsTalk connection alive (optional in this example)
}