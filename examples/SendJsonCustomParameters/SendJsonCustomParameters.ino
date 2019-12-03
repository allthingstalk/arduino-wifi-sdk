/* AllThingsTalk Arduino WiFi SDK
 * https://github.com/allthingstalk/arduino-wifi-sdk
 *
 * About this example: 
 * Shows various functions of this SDK.
 * It sets the Connection LED (that shows connection status) to GPIO PIN 2, 
 * sets WiFi Signal Reporting to 60 seconds and sets the WiFi hostname to "ATT-EXAMPLE".
 * Upon boot, it sends a message that it has booted and that it will disconnect soon.
 * It then disconnects and reconnects after 5 seconds, after which it sends a message
 * that it has reconnected. The process is repeated after 5 seconds.
 *
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create a Sensor asset of type String on your AllThingsTalk Maker (You'll receive messages there)
 * - Create a Sensor asset of type String on your AllThingsTalk Maker and name it "wifi-signal"
 * 
 * These are all the things in this example that you need to change to make it work:
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Sensor-Asset
 */

#include <AllThingsTalk_WiFi.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
char* asset      = "Sensor-Asset";                                // Name of your Sensor asset on AllThingsTalk (of type String)

void setup() {
  Serial.begin(115200);                    // Baud rate: 115200, but you can define any baud rate you want
  device.debugPort(Serial, true);          // Set library to output verbose debug to "Serial" port (defined above)
  device.connectionLed(true, LED_BUILTIN); // Built-in LED defined as Connection LED (LED_BUILTIN is a global variable that specifies the default onboard LED)
  device.wifiSignalReporting(true, 60);          // Report WiFi Signal every 60 seconds
  device.setHostname("ATT-EXAMPLE");       // Sets hostname of the device (Shows up on your WiFi network under this name)
  device.init();                           // Initialize AllThingsTalk
  device.send(asset, "Booted! Hello!");    // Send message "Booted! Hello!" to topic "asset_name"
}
void loop() {
  device.loop();         // Used to keep AllThingsTalk & WiFi connection alive
  device.send(asset, "Disconnecting WiFi & AllThingsTalk in 3 seconds"); // Send message to "asset" variable defined on top
  delay(3000);           // Wait 3 seconds
  device.send(asset, "Disconnecting WiFi & AllThingsTalk Now");
  device.disconnect();   // Disconnect from both AllThingsTalk and WiFi
  delay(5000);           // Wait 5 seconds
  device.connect();      // Connect to both WiFi and AllThingsTalk
  device.send(asset, "WiFi and AllThingsTalk Connection Re-Established");
  delay(5000);           // Wait 5 seconds
}