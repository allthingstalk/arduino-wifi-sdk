/* AllThingsTalk Arduino WiFi SDK
 * https://github.com/allthingstalk/arduino-wifi-sdk
 *
 * About this example: 
 * Control an externally connected LED using your AllThingsTalk Maker.
 * Try the "ControlLedBuiltIn" example if you don't have an LED.
 * 
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - [Optional] Create a string asset named "wifi-signal" on your AllThingsTalk Maker to see your device's WiFi Signal Strength
 * - Create an Actuator asset of type Boolean on your AllThingsTalk Maker. We'll use this asset to control the LED.
 * - Connect your LED's Negative (-) leg to your board's GND
 * - Connect your LED's Positive (+) leg to an adequate pin on your board
 * - Change the following things in this example to make it work: 
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Actuator-Asset, Your-Led-Pin
 */

#include <AllThingsTalk_WiFi.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
int  ledPin      = Your-Led-Pin;                                  // Change Your-Led-Pin to the pin you connected your LED to
char* asset      = "Actuator-Asset";                              // Name of your boolean actuator asset from AllThingsTalk Maker that you'll use to control the LED

void setup() {
  Serial.begin(115200);                 // Baud rate: 115200, but you can define any baud rate you want
  pinMode(ledPin, OUTPUT);              // Set the pin of our LED to OUTPUT
  device.debugPort(Serial);             // Set AllThingsTalk library to output its debug to "Serial"
  device.setActuationCallback(asset, actuation);  // "asset" - Variable we defined above, "actuation" - Name of the function below that does the work
  device.init();                        // Initialize AllThingsTalk
}

void loop() {
  device.loop();                        // Keep AllThingsTalk & WiFi alive
}

void actuation(bool value) {            // Function called when message arrives to your specified asset. "value" is the actual value received.
  if (value == true) {
    digitalWrite(ledPin, HIGH);         // Set pin "ledPin" to HIGH (ON)
    Serial.println("LED Turned ON");    // Prints to serial port
  } else {
    digitalWrite(ledPin, LOW);          // Sets pin "ledPin" to LOW (OFF)
    Serial.println("LED Turned OFF");   // Prints to serial port
  }
}