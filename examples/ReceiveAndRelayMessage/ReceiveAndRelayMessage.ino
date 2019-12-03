/* AllThingsTalk Arduino WiFi SDK
 * https://github.com/allthingstalk/arduino-wifi-sdk
 *
 * About this example: 
 * Waits for you to send a message from your AllThingsTalk Maker from an Actuator asset
 * and, once received, relays that message (using CBOR) back to a specified Sensor asset 
 * on your AllThingsTalk Maker.
 *
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create an Actuator asset of type String on your AllThingsTalk Maker (You'll send messages from here)
 * - Create a Sensor asset of type String on your AllThingsTalk Maker (You'll receive your messages back from here)
 *
 * These are all the things in this example that you need to change to make it work:
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Actuator-Asset, Sensor-Asset
 */

#include <AllThingsTalk_WiFi.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
char* actuator   = "Actuator-Asset";                              // Actuator asset on AllThingsTalk named "actuator"
char* sensor     = "Sensor-Asset";                                // Sensor asset on AllThingsTalk named "sensor"
CborPayload payload;                                              // Create CBOR payload object, so we can use CBOR to send data

void setup() {
  Serial.begin(115200);                   // Baud rate: 115200, but you can define any baud rate you want
  device.debugPort(Serial);               // Set AllThingsTalk library to output its debug to "Serial"
  device.setActuationCallback(actuator, actuation);  // Actuator asset defined above will trigger function "actuation" 
  device.init();                          // Initialize AllThingsTalk
}

void loop() {
  device.loop();                          // Keep AllThingsTalk & WiFi alive
}

void actuation(String value) {            // Function called when message arrives on asset "actuator" defined above
  Serial.print("Received message: ");     // Prints to serial output
  Serial.println(value);                  // Prints actual value received to serial output
  payload.reset();                        // Resets the payload
  payload.set(sensor, value);             // Adds "value" received to be sent to sensor asset (defined above) on AllThingsTalk Maker
  device.send(payload);                   // Sends the set payload(s)
}