
# AllThingsTalk Arduino SDK v2

AllThingsTalk Arduino Library for WiFi Devices - makes connecting your devices with your [AllThingsTalk Maker](https://maker.allthingstalk.com/) a breeze.


Here’s a **complete** Arduino sketch that connects to WiFi and sends `Hello World!` to your AllThingsTalk Maker:

```cpp
#include <AllThingsTalk.h>
auto wifiCreds = WifiCredentials("WiFi-SSID", "WiFi-Password");
auto deviceCreds = DeviceConfig("Device-ID", "Device-Token");
auto device = Device(wifiCreds, deviceCreds);
void setup() device.init(); device.send("Sensor-Asset", "Hello World!");
void loop() device.loop();
```

That’s how easy it is!

> [AllThingsTalk](https://www.allthingstalk.com) is an accessible IoT Platform for rapid development.  
In the blink of an eye, you'll be able to extract, visualize and use the collected data.  
[Get started and connect up to 10 devices free-of-charge](https://www.allthingstalk.com/maker)

# Table of Contents
<!--ts-->  
* [Installation](#installation)  
  * [Board Support](#board-support)  
* [Connecting](#connecting)
  * [Defining Credentials](#defining-credentials)
  * [Maintaining Connection](#maintaining-connection)
  * [Connecting and Disconnecting](#connecting-and-disconnecting)
  * [Connection LED](#connection-led)
    * [Connection LED Signals](#connection-led-signals)
    * [Define Your Own Connection LED Pin](#define-your-own-connection-led-pin)
    * [Disable Connection LED](#disable-connection-led)
  * [WiFi Signal Reporting](#wifi-signal-reporting)
    * [Showing WiFi Signal Strength](#showing-wifi-signal-strength)
    * [Custom Reporting Interval](#custom-reporting-interval)
    * [WiFi Signal Strength On-Demand](#wifi-signal-strength-on-demand)
    * [Disable WiFi Signal Reporting](#disable-wifi-signal-reporting)
* [Sending Data](#sending-data)
  * [JSON](#json)
  * [CBOR](#cbor)
  * [ABCL](#abcl)
* [Receiving Data](#receiving-data)
  * [Actuation Callbacks](#actuation-callbacks)
* [Debug](#debug)
  * [Enable Debug Output](#enable-debug-output)
  * [Enable Verbose Debug Output](#enable-verbose-debug-output)
* [Notes](#notes)
<!--te-->

# Installation
- **Install AllThingsTalk SDK**
    - [Download the library as ZIP file](https://github.com/AllThingsTalk/Arduino-SDK-v2/archive/master.zip)
    - Unzip it and remove “-master” from the folder name
    - Copy the folder to your Arduino libraries folder (most likely *Documents > Arduino > libraries*)
- **Install Dependencies**
    - Open Arduino IDE and go to *Tools* > *Manage Libraries*
    - Search for and download “**ArduinoJson**” by Benoit Blanchon
- **Restart your Arduino IDE**

You can now add this library in your sketch by going to *Sketch > Include Library > AllThingsTalk* or by adding *<AllThingsTalk.h>* in your sketch.  
Make sure to play with examples included in this library by going to *File > Examples > AllThingsTalk*

## Board Support
The library automatically recognizes supported Arduino boards and uses adequate version of itself.  
Library currently has **full** support for these boards:

- **ESP8266** (this includes all ESP8266-based dev boards)


Support is planned for the following boards:

- ESP32
- Arduino MKR 1000
- Arduino MKR 1010
- Arduino Generic with Ethernet Shield


# Connecting

The library takes care about initialization and maintaining WiFi and connection towards AllThingsTalk.

## Defining Credentials

At the beginning of your sketch, make sure to include this library and define your credentials:

```cpp
#include <AllThingsTalk.h>
auto wifiCreds = WifiCredentials("Your-WiFi-SSID","Your-WiFi-Password");
auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
auto device = Device(wifiCreds, deviceCreds);
```

> To get your Device ID and Device Token, go to your [AllThingsTalk Maker](https://maker.allthingstalk.com) devices, choose your device and click “*Settings*”, then choose “*Authentication*” and copy "Device ID" and “Device Tokens” values.


> From here and on, this part of the code won’t be shown in the examples below as it’s assumed you already have it.

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
It will also show connection status using the [built-in LED](#connection-led) of your board and publish [WiFi Signal Strength](#wifi-signal-reporting) to your [AllThingsTalk Maker](https://maker.allthingstalk.com)

## Connecting and Disconnecting

Connection is automatically established once `init()` is executed.  
However, if you wish to disconnect and/or connect from either WiFi or AllThingsTalk during device operation, you can do that by using these methods anywhere in your sketch:

| **Method**                 | **Operation**                                                                 |
| -------------------------- | ----------------------------------------------------------------------------- |
| connect();                 | Connects to **both** WiFi and AllThingsTalk                                   |
| disconnect();              | Disconnects **both** WiFi and AllThingsTalk                                   |
| connectWiFi();             | Connects to WiFi                                                              |
| disconnectWiFi();          | Disconnects from AllThingsTalk first (if connected) and then disconnects WiFi |
| connectAllThingsTalk();    | Connects to WiFi first (if not connected) and then connects to WiFi           |
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

## Connection LED

The library utilizes the built-in LED of your board to show current WiFi and AllThingsTalk connection status.

### Connection LED Signals

| **LED status**          | **System status**                                  |
| ----------------------- | -------------------------------------------------- |
| Off                     | WiFi and AllThingsTalk are both connected properly |
| Breathing               | Connecting to WiFi / AllThingsTalk…                |
| Breathing               | Connection failed. Trying to reconnect…            |
| Flashes quickly 2 times | Connection to AllThingsTalk and WiFi successful                              |


### Define Your Own Connection LED Pin

The library will detect your board and use its built-in LED automatically.  
You can override this by defining your own Connection LED pin by using `connectionLed(your_led_pin)` in your `setup()` function before initialization:

```cpp
void setup() {
  device.connectionLed(your_led_pin);
  device.init();
}
```

This feature can also be defined as `connectionLed(true, your_led_pin)`

> Custom Connection LED pin needs to be defined before `init()` and cannot be changed during operation.


### Disable Connection LED

Connection LED is enabled by default, but you can optionally disable it in case you need the LED for other purposes.  
In order to disable the Connection LED, add `connectionLed(false)` in your `setup()` function:

```cpp
void setup() {
  device.connectionLed(false);
  device.init();
}
```
> Unlike defining the Connection LED Pin, you **can** run  `connectionLed(false)` and `connectionLed(true)` anywhere in your sketch, in case you need to enable/disable it during operation.

## WiFi Signal Reporting

This library automatically reports WiFi Signal Strength to your AllThingsTalk Maker every 5 minutes by default.  
The strength is presented as `Excellent`, `Good`, `Decent`, `Bad` and `Horrible`, depending on the quality of your WiFi Connection.

### Showing WiFi Signal Strength

To show WiFi Signal Strength, create a *Sensor* asset on your AllThingsTalk Maker device named `wifi-signal` of type `String`

### Custom Reporting Interval

By default, the library publishes the WiFi Signal Strength to your AllThingsTalk Maker every 5 minutes (300 seconds).  
You can override this by defining your custom interval (in seconds) using `wifiSignalReporting(seconds)` in your `setup()` function before initialization:

```cpp
void setup() {
  device.wifiSignalReporting(seconds);
  device.init();
}
```

This feature can also be defined as `wifiSignalReporting(true, seconds)`  

> You can call `wifiSignalReporting` anywhere in your sketch if you wish to change its values during operation.

### WiFi Signal Strength On-Demand

You can retrieve WiFi Signal Strength information on demand (as long as you’re connected to WiFi) using the `wifiSignal()` method.  The output is of type String.  
Example:

```cpp
void setup() {
  Serial.begin(115200);
  device.init();
}
void loop() {
  Serial.println(wifiSignal());
}
```

### Disable WiFi Signal Reporting

If you wish to disable this feature, just call `wifiSignalReporting(false)` in your setup function instead:

```cpp
void setup() {
  device.wifiSignalReporting(false);
  device.init();
}
```

> You can enable and disable WiFi Signal Reporting anywhere in your sketch by calling `wifiSignalReporting(true)` or `wifiSignalReporting(false)`


# Sending Data

You can send data to AllThingsTalk in 3 different ways using the library.  
Let’s check them out.

## JSON

*JavaScript Object Notation*  
[Read about JSON in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#json)  
JSON is a lightweight data-interchange format which is easy for humans to read and write and easy for machines to parse. It’s widely adopted on the web.

> This is the quickest and simplest way of sending data.

In order to send a JSON message, just add the following line to your code:

```cpp
device.send("asset_name", "Hello there!");
```

- `asset_name` is the name of asset on your AllThingsTalk Maker.  
  This value needs to be `char*` if you’re defining it as a variable.
- `Hello there!` is the message that’ll be sent to the specified asset.  
  This value can be of any type. (In this case it's string)

When using JSON to send data, the message is sent immediately upon execution.

## CBOR

*Concise Binary Object Representation*  
[Read more about CBOR in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#cbor)  
CBOR is a data format whose design goals include the possibility of extremely small code size, fairly small message size, and extensibility without the need for version negotiation.  


> This method uses less data. Use it if you’re working with limited data or bandwidth.  
> As opposed to JSON data sending, with CBOR, you can build a payload with multiple messages before sending them.

You’ll need to create a `CborPayload` object before being able to send data using CBOR. The beginning of your sketch should therefore contain `CborPayload payload;`

```cpp
#include <AllThingsTalk.h>
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
    -  `asset_name` is the name of asset on your AllThingsTalk Maker.
    -  `value` is the value you want to send.
    You can add multiple messages (payloads) before actually sending them to AllThingsTalk.
- `device.send(payload)` sends everything in message queue to AllThingsTalk.

## ABCL

*AllThingsTalk Binary Conversion Language*  
[Read about ABCL in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)  
In case you want to connect an *off-the-shelf* device or define your own binary payload format, you might consider using *AllThingsTalk Binary Conversion Language* (ABCL). ABCL is a data description language that allows you to support any custom binary data format.


> You don’t need to define an asset to which the payload will be sent. This is because with ABCL, you define that in device settings on your AllThingsTalk Maker.
> To get familiar with ABCL, [check our Documentation.](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)
> As opposed to JSON data sending, with ABCL, you can build a payload with multiple messages before sending them.

You’ll need to create a `BinaryPayload` object before being able to send data using ABCL. The beginning of your sketch should therefore contain `BinaryPayload payload;`

```cpp
#include <AllThingsTalk.h>
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
    -  `value` is the value you want to send.
    You can add multiple messages (payloads) before actually sending them to AllThingsTalk.
- `device.send(payload)` sends everything in message queue to AllThingsTalk.


# Receiving data
## Actuation Callbacks

Actuation Callbacks call your functions once a message arrives from your AllThingsTalk Maker to your device on a specified asset.  
For each *Actuator* asset you have on your AllThingsTalk Maker device, you can add an actuation callback in your `setup()` function by adding `setActuationCallback("Your-Actuator-Asset-Name", YourFunction)`  
To receive data, simply create functions that utilize your desired type of data.  

>Your function argument can be of any type, just make sure to match your function argument type with your *Actuator* asset type on AllThingsTalk Maker. 

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

void myActuation1(String data) {
  Serial.print("You've received a message from AllThingsTalk: ");
  Serial.println(data);        // Prints the received String data
}

void myActuation2(bool data) {
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

> You can call `setActuationCallback` anywhere in your sketch and it will add a new Actuation Callback.
> You can define up to 32 Actuation Callbacks.


# Debug

The library outputs useful information such as your WiFi details, AllThingsTalk connection details, connection status details and errors, messages going in/out, raw messages and much, much more.

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

That’s it! You should now see debug information from the library along with your serial output (if you have some).

> When enabling Debug Output, make sure to define it before anything else from this library, so you can see all output from the library.

## Enable Verbose Debug Output

By enabling the debug as shown above, you’re getting normal debug output level.  
If you wish to enable verbose debug output (which outputs a lot more useful information about behind-the-scene operations) you can do that by adding a `true` argument to the existing `debugPort(Serial)` method.


> Enabling verbose debug output instead of normal debug output can help you significantly when troubleshooting your code.

**Example:**

```cpp
void setup() {
  Serial.begin(115200);
  device.debugPort(Serial, true);
  device.init();
}
```


# Notes
- This library uses [ArduinoJson by Benoît Blanchon](https://arduinojson.org/) and [PubSubClient by Nick O](https://pubsubclient.knolleary.net/)’[Leary](https://pubsubclient.knolleary.net/)
- The PubSubClient library is included with the AllThingsTalk Arduino SDK because the library requires modification of `MQTT_MAX_PACKET_SIZE` in PubSubClient.h beforehand. The modification is required because the default maximum `128` payload size isn't enough to receive bigger messages from your AllThingsTalk Maker. By including the library, installation of AllThingsTalk Arduino SDK is made easier and the version of PubSubClient is guaranteed to be compatible.  
This does not interfere with other instances of PubSubClient you might have in your Arduino libraries.
- Connection to AllThingsTalk may break if you use the `delay()` function too often or for prolonged periods of time due to the nature of that function. If this happens, try to use `millis()` to create delays when possible.
- Due to how ESP8266 works, the WiFi Connection may break when using `AnalogRead()` way too often. In this case, it is okay to use `delay()` for about 5 to 50 milliseconds (see what works for you) in order to avoid this issue.
- Receiving **JSON Objects** or **JSON Arrays** is not currently supported. Support is planned in next release.
- This library has been tested and confirmed to work with:
    - Arduino 1.8.9
    - PubSubClient 2.7.0 (Included)
    - ArduinoJson 6.11.4
