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
 *   the 2nd built-in LED of the NodeMCU board (located above the RST button), and its function is inverted, 
 *   which means when you set it to HIGH, it's going to be OFF.
 * - Change the following things in this example to make it work: 
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Actuator-Asset, LED_BUILTIN (Change only if this doesn't work for you)
 */

#include <AllThingsTalk.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
int  ledPin      = LED_BUILTIN;                                   // This is the built-in LED of the ESP8266 board. Define the pin of your LED if this doesn't work for you.
char* asset      = "Actuator-Asset";                              // Name of your actuator asset from AllThingsTalk Maker that you'll use to control the LED

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

void actuation(bool value) {            // Function called when message arrives on asset "actuator" defined above. "value" is the value received on that asset
  
  // The Built-In LED of the ESP8266 is inverted. Therefore, setting it to "HIGH" would set it to OFF. 
  // To compensate, we're going to set the statement below to "value == false" instead of "value == true".
  // You can change "false" to "true" if you board is different or you've setup an external LED.
  
  if (value == false) {
    digitalWrite(ledPin, HIGH);         // Set pin "ledPin" to HIGH
    Serial.println("LED Turned ON");    // Prints to serial port
  } else {
    digitalWrite(ledPin, LOW);          // Sets pin "ledPin" to LOW
    Serial.println("LED Turned OFF");   // Prints to serial port
  }
}