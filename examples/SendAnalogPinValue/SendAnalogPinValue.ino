/* AllThingsTalk Arduino SDK
 * https://github.com/allthingstalk/arduino-sdk-v2
 *
 * About this example: 
 * This example is taken from the full guide at https://docs.allthingstalk.com/examples/hardware/get-started-nodemcu/
 * It reads Analog pin A0 from your board and uploads that value to your AllThingsTalk Maker.
 *
 * Notes:
 * - Create a device on your https://maker.allthingstalk.com (if you don't already have it)
 * - Create a Sensor asset of type Integer on your AllThingsTalk Maker (You'll receive your analog value here)
 * - Create a Senosr asset named "wifi-signal" of type String on your AllThingsTalk Maker
 * - Change the following things in this example to make it work: 
 *   WiFi-SSID, WiFi-Password, Device-ID, Device-Token, Sensor-Asset
 */

#include <AllThingsTalk.h>

auto wifiCreds   = WifiCredentials("WiFi-SSID", "WiFi-Password"); // Your WiFi Network Name and Password
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");     // Go to AllThingsTalk Maker > Devices > Your Device > Settings > Authentication to get your Device ID and Token
auto device      = Device(wifiCreds, deviceCreds);                // Create "device" object
String yourAsset = "Sensor-Asset";                                // Name of the asset you're going to send data to

unsigned long startMillis;          // Used to keep track of send intervals
unsigned long currentMillis;        // Used to keep track of send intervals
const long    period        = 2000; // Change this to change how many milliseconds you want between analog port readings
int           analogButton  = 0;    // Variable that will store the value of the analog port

void setup() {
  Serial.begin(115200);             // Starts the Serial port for debugging
  device.debugPort(Serial);         // Enable debug output from AllThingsTalk library
  device.init();                    // Initialize AllThingsTalk
  startMillis = millis();           // Saves the initial millis value at boot to startMillis variable
}

void analogCheck() {                                   // This is the main function that checks the value of analog port
  currentMillis = millis();                            // Saves the value of "millis()" at the time of execution of this line
  if (currentMillis - startMillis >= period) {         // If current time minus the last saved startMillis time is bigger than the period defined above, it'll run the code below
    analogButton = analogRead(A0);                     // Reads the analog port A0 of your device and saves it to "analogButton" variable
    Serial.print("Current Analog Value: ");            // Prints to Serial port
    Serial.println(analogButton);                      // Prints to Serial port
    device.send(yourAsset, analogButton);              // Sends the data to AllThingsTalk. Data is sent to "yourAsset" that you defined at the top
    startMillis = currentMillis;                       // Resets the startMillis by assigning it the value of currentMillis
  }
}

void loop() {        // Main code that'll be run in loop all the time
  device.loop();     // Keep AllThingsTalk and WiFi connection alive
  analogCheck();     // Runs our "analogCheck" function, which checks the value of analog port and publishes it
}
