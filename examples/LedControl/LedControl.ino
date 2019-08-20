/* AllThingsTalk Arduino SDK
 * https://github.com/allthingstalk/arduino-sdk-v2
 *
 * About this example: 
 * Control an LED connected to your board using your AllThingsTalk Maker.
 * 
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create an Actuator asset of type Boolean on your AllThingsTalk Maker. We'll use this asset to control the LED.
 * - The LED can be a built-in one in most cases (so you don't have to connect anything to your board)
 *   Please check the built-in LED pin number of your board. The LED pin defined in this example is
 *   the 2nd built-in LED of the NodeMCU board (located above the RST button)
 * - Change the following things in this example to make it work: 
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Actuator-Asset
 */

#include <AllThingsTalk.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
int  ledPin      = LED_BUILTIN;                                   // This is the built-in LED of the ESP8266 board. Define the pin of your LED if you're not using this board.

void setup() {
  Serial.begin(115200);                 // Baud rate: 115200, but you can define any baud rate you want
  pinMode(ledPin, OUTPUT);              // Set the pin of our LED to OUTPUT
  device.debugPort(Serial);             // Set AllThingsTalk library to output its debug to "Serial"
  device.setActuationCallback("Actuator-Asset", actuation);  // Your Actuator asset that you'll use to control the LED by triggering function "actuation" below
  device.init();                        // Initialize AllThingsTalk
}

void loop() {
  device.loop();                        // Keep AllThingsTalk & WiFi alive
}

void actuation(bool value) {            // Function called when message arrives on asset "actuator" defined above
  digitalWrite(ledPin, value);          // Set pin "ledPin" (defined above) to "value" (which comes from your AllThingsTalk Maker)
  Serial.print("LED Set to: ");         // Prints to serial port
  Serial.println(value);                // Prints to serial port
}