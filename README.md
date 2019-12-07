
# AllThingsTalk Arduino WiFi SDK

<img align="right" width="250" height="148" src="extras/wifi-logo.png">

AllThingsTalk Arduino Library for WiFi Devices - makes connecting your devices with your [AllThingsTalk Maker](https://maker.allthingstalk.com/) a breeze.  

Here’s a **complete** Arduino sketch that connects to WiFi and sends `Hello World!` to your AllThingsTalk Maker:

```cpp
#include <AllThingsTalk_WiFi.h>
auto wifiCreds = WifiCredentials("WiFiName", "WiFiPassword");
auto deviceCreds = DeviceConfig("DeviceID", "DeviceToken");
auto device = Device(wifiCreds, deviceCreds);
void setup() { device.init(); device.send("StringSensorAsset", "Hello World!"); }
void loop() { device.loop(); }
```

That’s how easy it is!  
If you're having issues, [check the troubleshooting section](#troubleshooting-and-notes).

> [AllThingsTalk](https://www.allthingstalk.com) is an accessible IoT Platform for rapid development.  
In the blink of an eye, you'll be able to extract, visualize and use the collected data.  
[Get started and connect up to 10 devices free-of-charge](https://www.allthingstalk.com/maker)

# Table of Contents
<!--ts-->  
* [Installation](#installation)  
  * [Board Support](#board-support)  
* [Connecting](#connecting)
  * [Defining Credentials](#defining-credentials)
    * [Separating Credentials (keys.h)](#separating-credentials)
    * [Custom AllThingsTalk Space](#custom-allthingstalk-space)
  * [Maintaining Connection](#maintaining-connection)
  * [Connecting and Disconnecting](#connecting-and-disconnecting)
  * [Setting Hostname](#setting-hostname)
  * [Connection LED](#connection-led)
    * [Connection LED Signals](#connection-led-signals)
    * [Define Your Own Connection LED Pin](#define-your-own-connection-led-pin)
    * [Disable Connection LED](#disable-connection-led)
  * [WiFi Signal Reporting](#wifi-signal-reporting)
    * [Enable WiFi Signal Reporting](#enable-wifi-signal-reporting)
    * [Custom Reporting Interval](#custom-reporting-interval)
    * [WiFi Signal Strength On-Demand](#wifi-signal-strength-on-demand)
* [Creating Assets](#creating-assets)
* [Sending Data](#sending-data)
  * [JSON](#json)
  * [CBOR](#cbor)
  * [ABCL](#abcl)
* [Receiving Data](#receiving-data)
  * [Actuation Callbacks](#actuation-callbacks)
* [Debug](#debug)
  * [Enable Debug Output](#enable-debug-output)
  * [Enable Verbose Debug Output](#enable-verbose-debug-output)
* [Troubleshooting and Notes](#troubleshooting-and-notes)
<!--te-->

# Installation

- **To install AllThingsTalk WiFi SDK:**  
    - In Arduino IDE, go to *Tools* > *Manage Libraries*
    - Search for and download "**AllThingsTalk WiFi SDK**" by AllThingsTalk
- **Install Dependencies:**  
	This library has a few dependencies, so while your Library Manager is open, install these:  
    
	| Name | Author | Required for | Version (at least) |
	|--|--|--|--|
	| **ArduinoJson** | Benoit Blanchon | All Boards | 6.13 |
	| **WiFiNINA** | Arduino | MKR1010 | 1.4.0 |
	| **Scheduler** | Arduino | MKR1010 | 0.4.4 |  

- Done!

Utilize SDK by adding ***<AllThingsTalk_WiFi.h>*** to your sketch.  
Play with **[provided examples](/examples)** by going to *File > Examples > AllThingsTalk WiFi SDK*

> If you want to install this SDK manually, [download the library as zip file](https://github.com/allthingstalk/arduino-wifi-sdk/archive/master.zip), unzip it and copy the folder to your Arduino libraries folder (most likely *Documents > Arduino > libraries*). Install dependencies normally as stated above.

## Board Support
The library automatically recognizes supported Arduino boards and uses adequate version of itself.  
Library currently has **full** support for these boards:

- **[ESP8266](http://esp8266.net/)** (includes all ESP8266-based boards)
- **[Arduino MKR WiFi 1010](https://store.arduino.cc/mkr-wifi-1010)**

Support is planned for ESP32 and MKR1000. Feel free to create pull requests any time.

# Connecting

The library takes care about initialization and maintaining WiFi and connection towards AllThingsTalk.

## Defining Credentials

At the beginning of your sketch (before `setup()`), make sure to include this library and define your credentials as shown:

```cpp
#include <AllThingsTalk_WiFi.h>
auto wifiCreds = WifiCredentials("Your-WiFi-SSID","Your-WiFi-Password");
auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
auto device = Device(wifiCreds, deviceCreds);
```

> To get your **Device ID** and **Device Token**, go to your [AllThingsTalk Maker](https://maker.allthingstalk.com) devices, choose your device and click “*Settings*”, then choose “*Authentication*” and copy "Device ID" and “Device Tokens” values.

> From here and on, this part of the code won’t be shown in the examples below as it’s assumed you already have it.

### Separating Credentials

In case you share your Arduino sketch often (either via GitHub or other means), you should consider creating a "**keys.h**" file that contains your credentials.  
This way, when you share your sketch, your credentials would remain private because you'd only share the Arduino sketch. Another advantage is that whoever downloads your sketch could have their own **keys.h** file, thus the sketch would immediately work on their computer.

> Check the *SendJsonSimplestSeparateCredentials* example included in this SDK to see this in action.

First, create a new file in the same directory as your Arduino sketch, name it **keys.h** and copy/paste the following into the file:
```cpp
#ifndef KEYS_H
#define KEYS_H

const char* WIFI_SSID     = "Your-WiFi-Network-Name";
const char* WIFI_PASSWORD = "Your-WiFi-Password";
const char* DEVICE_ID     = "Your-Device-ID";
const char* DEVICE_TOKEN  = "Your-Device-Token";

#endif
```

Then, make sure to change the top of your sketch to include **keys.h** and use the variables defined in it:

```cpp
#include <AllThingsTalk_WiFi.h>
#include "keys.h" // Include our newly created file
auto wifiCreds = WifiCredentials(WIFI_SSID, WIFI_PASSWORD);
auto deviceCreds = DeviceConfig(DEVICE_ID, DEVICE_TOKEN);
auto device = Device(wifiCreds, deviceCreds);
```
So, now you've included the "**keys.h**" file and changed the credentials to variables that will be loaded from that file. Done.

> If you're going to put your sketch to GitHub, make sure to [gitignore](https://help.github.com/en/articles/ignoring-files) the **keys.h** file

### Custom AllThingsTalk Space

> If you don't know what Spaces are, you can skip this part.

The connection defaults to [AllThingsTalk Maker](https://maker.allthingstalk.com/), but in case you're using a different [AllThingsTalk space](https://www.allthingstalk.com/spaces), you can define your API Endpoint by adding another argument to DeviceConfig, as shown below:

```cpp
auto deviceCreds = DeviceConfig("Your-Device-ID", "Your-Device-Token", "your.api.endpoint");
```

## Maintaining Connection

All you have to do to connect and to maintain a connection is add `init()` method to your `setup()` function and `loop()` method to your `loop()` function:

```cpp
void setup() {
  device.init();  // Initializes AllThingsTalk
}
void loop() {
  device.loop();  // Keeps WiFi and AllThingsTalk connection alive (if active)
}
```

This will take care of connecting to WiFi and AllThingsTalk.  
It will also show connection status using the [built-in LED](#connection-led) of your board and publish [WiFi Signal Strength](#wifi-signal-reporting) (if enabled) to your [AllThingsTalk Maker](https://maker.allthingstalk.com).

## Connecting and Disconnecting

Connection is automatically established once `init()` is executed.  
However, if you wish to disconnect and/or connect from either WiFi or AllThingsTalk during device operation, you can do that by using these methods **anywhere** in your sketch:

| **Method**                 | **Operation**                                                                 |
| -------------------------- | ----------------------------------------------------------------------------- |
| connect();                 | Connects to **both** WiFi and AllThingsTalk                                   |
| disconnect();              | Disconnects **both** WiFi and AllThingsTalk                                   |
| connectWiFi();             | Connects only to WiFi                                                              |
| disconnectWiFi();          | Disconnects from AllThingsTalk first (if connected) and then disconnects WiFi |
| connectAllThingsTalk();    | Connects to WiFi first (if not connected) and then connects to AllThingsTalk           |
| disconnectAllThingsTalk(); | Disconnects from AllThingsTalk                                                |

> Since you might want to use this to save battery (if running on battery power) or if you’re putting your device in sleep mode, the Connection LED will remain OFF even when disconnected from WiFi or AllThingsTalk because the action was intentional.
> Normal operation of the Connection LED is resumed upon connection to either WiFi or AllThingsTalk.

Example:

```cpp
void setup() {
  device.init();       // Initializes AllThingsTalk
}
void loop() {
  device.loop();       // Keeps WiFi and AllThingsTalk connection alive (if active)
  device.disconnect(); // Disconnect from both WiFi and AllThingsTalk
  delay(5000);         // Wait 5 seconds
  device.connect();    // Connects to both WiFi and AllThingsTalk
}
```

## Setting Hostname
You can set the hostname for your device (which is how it will be seen by other devices on the WiFi network) by using `setHostname("YOUR_HOSTNAME")` before `device.init()`.

Example:
```cpp
void setup() {
  device.setHostname("MyDevice");
  device.init();
}
```

## Connection LED

The library utilizes the built-in LED of your board to show current WiFi and AllThingsTalk connection status.  
This helps you see connection status at a glance and is turned on by default.  
For more detailed information, use [Debug output](#debug).

### Connection LED Signals

| **LED status** | **Connection status** |
| -- | -- |
| Fading in and out | Connecting/Reconnecting to WiFi/AllThingsTalk |
| Blinks briefly | Connection successful |
| Blinks briefly and continues fading | Connected to WiFi but still connecting to AllThingsTalk |
| Off | Connected to both WiFi and AllThingsTalk |

### Define Your Own Connection LED Pin

The library will detect your board and use its built-in LED automatically.  
You can override this by defining your own Connection LED pin by using `connectionLed(your_led_pin)` in your `setup()` function before initialization:

```cpp
void setup() {
  device.connectionLed(your_led_pin);
  device.init();
}
```

- This feature can also be defined as `connectionLed(true, your_led_pin)`
- You can change the Connection LED pin during operation by called the method above.


### Disable Connection LED

Connection LED is enabled by default, but you can optionally disable it in case you need the LED for other purposes.  
In order to disable the Connection LED, add `connectionLed(false)` in your `setup()` function:

```cpp
void setup() {
  device.connectionLed(false);
  device.init();
}
```
- You can use `connectionLed()` (without arguments) to check if the feature is enabled or not.  Method returns boolean **true** or **false**.
- You can also call  `connectionLed(false)` and `connectionLed(true)` **anywhere** in your sketch, in case you need to enable/disable it during operation.

## WiFi Signal Reporting

This library has the ability to automatically report WiFi Signal Strength to your AllThingsTalk Maker (defaults to every 5 minutes).  
The strength is presented as `Excellent`, `Good`, `Decent`, `Bad` and `Horrible`, depending on the quality of your WiFi Connection.

>If your device reports to AllThingsTalk only based on some physical state change and you don't see any updates for some time, you don't have a way of knowing if the device unexpectedly went offline, since you'd just think the state didn't change.
> Fortunately, a side-effect of WiFi Signal Reporting is that you can use it with AllThingsTalk **Watchdog**, so you can be sure your device went offline for some reason if it doesn't report back at the predefined WiFi Signal Reporting interval.
> Setup Watchdog by going to your AllThingsTalk Maker > *Your Device* > Settings > Watchdog 

### Enable WiFi Signal Reporting

This feature is off by default, so if you wish to enable it, simply call `wifiSignalReporting(true)` in your `setup()` function (preferably before initialization).

> Enabling this feature will also automatically [create the required asset](#creating-assets) on your AllThingsTalk for you to see the data. 
> (Asset Name: `wifi-signal`, Title: `WiFi Signal Strength`, Asset Type: `sensor`, Data Type: `string`)

Example:
```cpp
void setup() {
  device.wifiSignalReporting(true);
  device.init();
}
```

- You can enable and disable WiFi Signal Reporting **anywhere** in your sketch by calling `wifiSignalReporting(true)` or `wifiSignalReporting(false)`
- You can use `wifiSignalReporting()` (without arguments) to check if the feature is enabled or not.  Method returns boolean **true** or **false**.

### Custom Reporting Interval

By default, the library publishes the WiFi Signal Strength to your AllThingsTalk Maker every 5 minutes (300 seconds).  
You can override this by defining your custom interval (in seconds) using `wifiSignalReporting(true, seconds)` in your `setup()` function before initialization:

```cpp
void setup() {
  device.wifiSignalReporting(true, seconds);
  device.init();
}
```

- You can call `wifiSignalReporting(seconds)` **anywhere** in your sketch if you wish to change its value during operation.

### WiFi Signal Strength On-Demand

You can retrieve WiFi Signal Strength information on demand (as long as you’re connected to WiFi) using the `wifiSignal()` method.  The output is of type String.  

Example:

```cpp
void setup() {
  Serial.begin(115200);
  device.init();
}
void loop() {
  Serial.println(wifiSignal()); // Prints WiFi signal to serial
}
```

# Creating Assets

This library provides functionality of creating (or updating) assets on your AllThingsTalk directly from your device upon boot.  

> Your assets will be created upon device boot (when it connects to WiFi), and if they already exist, they'll be updated with properties defined in `createAsset()` method.  
> You can create up to 64 assets on AllThingsTalk from your device.

To utilize this feature, simply call `createAsset("assetName", "Asset Title", "assetType", "dataType")` method before calling `init()` in your `setup()` function.  
Method accepts 4 arguments of type String, which are:

- ***1st:*** Asset Name
	- Unique name of the asset to be created on AllThingsTalk (e.g. "door-sensor")
	- Type: String (without spaces)
- ***2nd:*** Asset Title
	- Title of the asset (e.g. "Home Door Sensor")
	- Type: String
- ***3rd:*** Asset Type
	- Type of asset on AllThingsTalk
	- Type: String (without spaces)
	- Can be: **sensor**, **actuator**, **virtual**
- ***4th:*** Data Type
	- Type of data the device will provide to the asset
	- Type: String (without spaces)
	- Can be: **boolean**, **string**, **integer**, **number**, **object**, **array**, **location**

Example:
```cpp
void setup() {
  Serial.begin(115200);
  device.debugPort(Serial);
  device.createAsset("led", "LED Light", "actuator", "boolean");
  device.createAsset("servo", "Servo Motor", "actuator", "integer");
  device.createAsset("button", "Push Button", "sensor", "boolean");
  device.createAsset("temp", "Temperature", "sensor", "number");
  device.createAsset("humidity", "Humidity", "sensor", "number");
  device.init();
}
```

# Sending Data

You can send data to AllThingsTalk in 3 different ways using the library.  

## JSON

*JavaScript Object Notation*  
[Read about JSON in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#json)  
JSON is a lightweight data-interchange format which is easy for humans to read and write and easy for machines to parse. It’s widely adopted on the web.

> This is the quickest and simplest way of sending data.

Use the following method to send a JSON message:

```cpp
device.send("asset_name", value);
```

- `asset_name` is the name of asset on your AllThingsTalk Maker.  
  This argument is of type `char*`, in case you’re defining it as a variable.
- `value` is the data that’ll be sent to the specified asset. It can be of any type.

When using JSON to send data, the message is sent immediately upon execution.  
`device.send()` returns boolean **true** or **false** depending on if the message went through or not.

## CBOR

*Concise Binary Object Representation*  
[Read more about CBOR in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#cbor)  
CBOR is a data format whose design goals include the possibility of extremely small code size, fairly small message size, and extensibility without the need for version negotiation.  

> This method uses less data. Use it if you’re working with limited data or bandwidth.  
> As opposed to JSON data sending, with CBOR, you can build a payload with multiple messages before sending them.

You’ll need to create a `CborPayload` object before being able to send data using CBOR.  
By default, the maximum CBOR payload size is **256 bytes**. If needed, you can change that by by using `CborPayload payload(payload_size_in_bytes)` when creating the object.  
The beginning of your sketch should therefore contain `CborPayload payload` or `CborPayload payload(payload_size_in_bytes)`:

```cpp
#include <AllThingsTalk_WiFi.h>
auto wifiCreds = WifiCredentials("Your-SSID","Your-WiFi-Password");
auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
auto device = Device(wifiCreds, deviceCreds);
CborPayload payload;
```

Now you can send a message to your AllThingsTalk Maker whenever you want using:

```cpp
payload.reset();
payload.set("asset_1", value);
payload.set("asset_2", value2);
...
device.send(payload);
```

- `payload.reset()` clears the message queue, so you’re sure what you’re about to send is the only thing that’s going to be sent.  
- `payload.set("asset_name", value)` adds a message to queue. 
	You can add as many messages (payloads) as you like before actually sending them to AllThingsTalk.
    -  `asset_name` is the name of asset on your AllThingsTalk Maker.  
       This argument is of type `char*`, in case you’re defining it as a variable.
    -  `value` is the data you want to send. It can be of any type.
    
- `device.send(payload)` sends everything in message queue to AllThingsTalk. It also returns boolean **true** or **false** depending on if the message went through or not.

## ABCL

*AllThingsTalk Binary Conversion Language*  
[Read about ABCL in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)  
In case you want to connect an *off-the-shelf* device or define your own binary payload format, you might consider using *AllThingsTalk Binary Conversion Language* (ABCL). ABCL is a data description language that allows you to support any custom binary data format.


> You don’t need to define an asset to which the payload will be sent. This is because with ABCL, you define that in device settings on your AllThingsTalk Maker.
> To get familiar with ABCL, [check our Documentation.](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)
> As opposed to JSON data sending, with ABCL, you can build a payload with multiple messages before sending them.

You’ll need to create a `BinaryPayload` object before being able to send data using ABCL. The beginning of your sketch should therefore contain `BinaryPayload payload;`

```cpp
#include <AllThingsTalk_WiFi.h>
auto wifiCreds = WifiCredentials("Your-SSID","Your-WiFi-Password");
auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
auto device = Device(wifiCreds, deviceCreds);
BinaryPayload payload;
```

Now you can send a message to AllThingsTalk using:

```cpp
payload.reset();
payload.add(value);
payload.add(value2);
...
device.send(payload);
```

- `payload.reset()` clears the message queue, so you’re sure what you’re about to send is the only thing that’s going to be sent.
- `payload.set(value)` adds a message to queue.  
  You can add as many messages (payloads) as you like before actually sending them to AllThingsTalk.
    -  `value` is the value you want to send. It can be of any type.  
- `device.send(payload)` sends everything in message queue to AllThingsTalk. It also returns boolean **true** or **false** depending on if the message went through or not.


# Receiving data
## Actuation Callbacks

Actuation Callbacks call your functions once a message arrives from your AllThingsTalk Maker to your device on a specified asset.  
For each *Actuator* asset you have on your AllThingsTalk Maker device, you can add an actuation callback in your `setup()` function (or anywhere else if you wish) by adding `setActuationCallback("Your-Actuator-Asset-Name", YourFunction)`  
To receive data, simply create functions that utilize your desired type of data.  

- Your function argument can be of any type (SDK automatically adjusts).  
  Just make sure to match your function argument type with your *Actuator* asset type on AllThingsTalk Maker. 
- You're able to call `setActuationCallback("asset", YourFunction)` anywhere in your sketch to add a new Actuation Callback during runtime.  
- Returns boolean **true** if it was successful and **false** if it failed.
- You can define up to 32 Actuation Callbacks.

**Example:**

```cpp
...
void setup() {
  Serial.begin(115200);        // Starts serial port on buad rate 115200
  device.debugPort(Serial);    // Tells the library to use the defined serial port
  pinMode(LED_PIN, OUTPUT);    // Initializes pin LED_PIN as output
  device.setActuationCallback("your-asset-1", myActuation1);
  device.setActuationCallback("your-asset-2", myActuation2);
  device.init();               // Initializes AllThingsTalk and WiFi Connection
}

void myActuation1(String data) { // Called when a message from "your-asset-1" is received
  Serial.print("You've received a message from AllThingsTalk: ");
  Serial.println(data);        // Prints the received String data
}

void myActuation2(bool data) { // Called when a message from "your-asset-2" is received
  if (data) {
    digitalWrite(LED_PIN, HIGH); // Turns on LED
  } else {
    digitalWrite(LED_PIN, LOW); // Turns off LED
}

void loop() {
  device.loop();  // Keeps AllThingsTalk and WiFi Connection alive
}
```

This means that each time a message arrives from your *Actuator* asset `your-asset-1` from AllThingsTalk Maker, your function `myActuation1` will be called and the message (actual data) will be forwarded to it as an argument.  
In this case, if your device receives a string value `Hello there!` on asset `your-asset-1`, the received message will be printed via Serial and if it receives value `true` on asset `your-asset-2`, the LED will be turned on. (You would change LED_PIN to a real pin on your board).

# Debug

The library outputs useful information such as your WiFi details, AllThingsTalk connection details, connection status details and errors, asset creation results, messages going in/out, raw messages and much, much more.

> When choosing a baud rate, if your board supports it, you should go for 115200 or higher, because higher speed baud rates mean less time wasted (by the CPU) outputting messages, therefore not halting your code.

## Enable Debug Output

Output of this information is not enabled by default. To enable it, you need to pass the name of your Serial interface to `debugPort()`  
For example, if you have `Serial.begin(115200)` in your `setup()` function, you'd add `debugPort(Serial)`:

```cpp
void setup() {
  Serial.begin(115200);
  device.debugPort(Serial);  // Now the library knows where to output debug information
  device.init();
}
```

That’s it! You should now see debug information from the library along with your serial output (if you have any).

> When enabling Debug Output, make sure to define it before anything else from this library, so you can see all output from the library.

## Enable Verbose Debug Output

> Enabling Verbose Debug Output can help you significantly when troubleshooting your code.
 
If you wish to see more information, you can use Verbose Debug Output which, in addition to Normal Debug Output, shows:  
- Asset Creation HTTP Responses
- HTTP Connection Status (for Asset Creation)
- Unique Generated MQTT ID
- API Endpoint
- Device ID (masked)
- Device Token (masked)
- IP Address
- WiFi Signal Strength
- Raw MQTT Topics
- Raw JSON Payloads
- Extracted message arrival times
- Extracted asset names (shows even if there's no actuation callback defined)
- Detailed information on Actuation Callbacks (when added and when called)
- Functions linked to Actuation Callbacks along with values and data types
- and more...

Enable Verbose Debug Output by adding argument `true` to the existing `debugPort(Serial)` method.

Example:

```cpp
void setup() {
  Serial.begin(115200);
  device.debugPort(Serial, true); // Verbose Debug Output is now enabled
  device.init();
}
```


# Troubleshooting and Notes

- This SDK has been tested and confirmed to work with the following software, so if you're having issues with your device/code, **make sure** you're working with at least these versions:

    | Name | Version | Used for | Type | Description |
    |--|--|--|--|--|
    | [Arduino IDE](https://www.arduino.cc/en/Main/Software) | 1.8.10 | All | Desktop Software | Main development environment. |
    | [esp8266](https://github.com/esp8266/Arduino#installing-with-boards-manager) | 2.6.2 | ESP8266 |  Arduino Board [Core](https://www.arduino.cc/en/Guide/Cores) | Enables Arduino IDE to work with *ESP8266* and *ESP8266*-based devices. |
    | [Arduino SAMD](https://www.arduino.cc/en/Guide/MKRWiFi1010#toc2) | 1.8.3 | MKR1010 |Arduino Board [Core](https://www.arduino.cc/en/Guide/Cores) | Enables Arduino IDE to work with devices based on SAMD architecture (*MKR1010*). |
     | [NINA Firmware](https://www.arduino.cc/en/Tutorial/WiFiNINA-FirmwareUpdater) | 1.2.4 | MKR1010 | Arduino MKR1010 WiFi Firmware | Firmware that drives the WiFi module present on the *Arduino MKR1010*. |
    | [WiFiNINA](https://www.arduino.cc/en/Reference/WiFiNINA) | 1.4.0 | MKR1010 | Arduino Library | Enables *Arduino MKR1010* to connect to WiFi.  |
    | [ArduinoJson](https://arduinojson.org/) | 6.13 | All | Arduino Library | Parsing and building JSON payloads to send/receive from AllThingsTalk. |
    | [PubSubClient](https://pubsubclient.knolleary.net/) (INCLUDED) | 2.7.0 | All | Arduino Library | Used by this SDK to connect to AllThingsTalk. Already included in this SDK. |
    | [Scheduler](https://www.arduino.cc/en/Reference/Scheduler) | 0.4.4 | MKR1010 | Arduino Library | Enables 'multithreading' on *Arduino MKR1010*. |
- Make sure to [enable verbose debug output](#enable-verbose-debug-output) as it could tell you a lot and thus help you resolve your problem.
- If you try to send data to a non-existent asset on AllThingsTalk, you might get disconnected. This is by design. You can make sure the asset exists by using the [create assets](#creating-assets) feature of this SDK.
- Connection to AllThingsTalk may break if you use the `delay()` function too often or for prolonged periods of time due to the nature of that function. The SDK will recover the connection automatically, but if this happens too often, try to use `millis()` to create delays when possible.
- Due to how ESP8266 works, the WiFi Connection may break when using `AnalogRead()` sometimes. This is out of our control. It will most likely fail when reading an analog pin too often. In this case, it is okay to use `delay()` for about 50 or more milliseconds (see what works for you) in order to avoid this issue.
- Receiving **JSON Objects** or **JSON Arrays** is not currently supported. Support is planned in a future release.
- If you find any bugs in this SDK, feel free to [create an issue](https://github.com/allthingstalk/arduino-wifi-sdk/issues).
