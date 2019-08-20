# Arduino SDK v2
AllThingsTalk Arduino library for WiFi Devices.
This library will help you connect your Arduino devices to your [AllThingsTalk Maker](https://maker.allthingstalk.com/)
to send/receive data with ease.

# Board support & recognition

The library will automatically recognize supported Arduino boards and use adequate version of itself that was tailored specifically for that board.

**Currently supported boards**
Library currently has full support for these boards:

- NodeMCU

**Planned support**
Support is planned for the following boards:

- ESP8266
- Arduino MKR 1000
- Arduino MKR 1010
- Arduino Generic with Ethernet Shield



# Connecting

The library takes care about initialization and maintaining WiFi and connection towards AllThingsTalk.

## Defining credentials

At the beginning of your sketch, make sure to define your credentials:

    #include <AllThingsTalk.h>
    auto wifiCreds = WifiCredentials("Your-SSID","Your-WiFi-Password");
    auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
    auto device = Device(wifiCreds, deviceCreds);
## Maintaining connection

And all you have to do to connect and to maintain a connection is add `device.init()` to your `setup()` function and `device.loop()` to your `loop()` function:

    void setup() {
      device.init();
    }
    void loop() {
      device.loop();
    }
## Connection LED

The library utilizes the built-in LED of your board to show current WiFi and AllThingsTalk connection status.

**Connection signals**

| **LED Status**  | **System Status**                          |
| ----------------------- | -------------------------------------------------- |
| Off                     | WiFi and AllThingsTalk are both connected properly |
| Breathing               | Connecting to WiFi / AllThingsTalk…                |
| Breathing               | Connection failed. Trying to reconnect…            |
| Flashes quickly 2 times | Connection successful                              |

**Disabling connection LED**
Connection LED is enabled by default, but you can optionally disable it in case you need the LED for your own purposes.
In order to disable the Connection LED, add `false` when initializing your device in `setup()` function:

    void setup() {
      device.init(false);
    }


> You can also combine these two commands: debug Serial and Connection LED off by using: `device.init(Serial, false)` 



# Sending Data

You can send data to AllThingsTalk in 3 different ways using the library. Let’s check them out.

## CBOR

*Concise Binary Object Representation*
[Read more about CBOR in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#cbor)
CBOR is a data format whose design goals include the possibility of extremely small code size, fairly small message size, and extensibility without the need for version negotiation.


> This the recommended way of sending data unless you have a specific need to use ABCL or JSON

You’ll need to create a `CborPayload` object before being able to send data using CBOR. The beginning of your sketch should look something like this:

    #include <AllThingsTalk.h>
    auto wifiCreds = WifiCredentials("Your-SSID","Your-WiFi-Password");
    auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
    auto device = Device(wifiCreds, deviceCreds);
    CborPayload payload;

 
 Now you can send a message to AllThingsTalk whenever you want using:

    payload.reset();
    payload.set("asset_1", value);
    payload.set("asset_2", value2);
    ...
    device.send(payload);
- `payload.reset()` - Clears the message queue, so you’re sure what you’re about to send is the only thing that’s going to be sent.
- `payload.set(``"``asset_name``"``, value)` adds a message to queue. 
    -  `asset_name` is the name of asset on your AllThingsTalk Maker.
    -  `value` is the value you want to send.
    You can add multiple messages (payloads) before actually sending them to AllThingsTalk.
- `device.send(payload)` sends everything in message queue to AllThingsTalk.


## JSON

*JavaScript Object Notation*
[Read about JSON in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#json)
JSON is a lightweight data-interchange format which is easy for humans to read and write and easy for machines to parse. It’s widely adopted on the web.


> This is the simplest way of sending data.

In order to send a JSON message, just add the following line to your code:

    device.send("asset_name", "Hello there!");
- `asset_name` is the name of asset on your AllThingsTalk Maker.
- `Hello there!` is the message that’ll be sent to the specified asset.

You don’t need to create any objects for sending data in this case. There is also no need of payload resetting because each command builds a message and sends it right away.


## ABCL

*AllThingsTalk Binary Conversion Language*
[Read about ABCL in our Documentation](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)
In case you want to connect an *off-the-shelf* device or define your own binary payload format, you might consider using **AllThingsTalk Binary Conversion Language** (ABCL). ABCL is a data description language that allows you to support any custom binary data format.


> You don’t need to define an asset to which the payload will be sent. This is because with ABCL, you define that in device settings on your AllThingsTalk Maker.
> To get familiar with ABCL, [check our Documentation.](https://docs.allthingstalk.com/developers/data-formats/#custom-binary)

You’ll need to create a `BinaryPayload` object before being able to send data using ABCL. The beginning of your sketch should look something like this:

    #include <AllThingsTalk.h>
    auto wifiCreds = WifiCredentials("Your-SSID","Your-WiFi-Password");
    auto deviceCreds = DeviceConfig("Your-Device-ID","Your-Device-Token");
    auto device = Device(wifiCreds, deviceCreds);
    BinaryPayload payload;

 
 Now you can send a message to AllThingsTalk using:

    payload.reset();
    payload.add(value);
    payload.add(value2);
    ...
    device.send(payload);
- `payload.reset()` - Clears the message queue, so you’re sure what you’re about to send is the only thing that’s going to be sent.
- `payload.set(value)` adds a message to queue. 
    -  `value` is the value you want to send.
    You can add multiple messages (payloads) before actually sending them to AllThingsTalk.
- `device.send(payload)` sends everything in message queue to AllThingsTalk.


## 
# Receiving data
## Actuations

Actuations are functions that will be called once a message arrives from AllThingsTalk to your device on a specified asset. For each “Actuator” asset you have on your AllThingsTalk Maker, you can add an actuation in your `setup()` function:

    void setup() {
      Serial.begin(115200);
      device.init();
      device.setActuation("your-asset-1", myActuation);
      device.setActuation("your-asset-2", myActuation2);
    ...
    }

This means that each time a message arrives from `your-asset-1`, function `myActuation` will be called and the message (the actual data) will be forwarded to it.


> Make sure to set new actuations after you initialized the device with `device.init()`

To receive data, simply create function(s) that utilizes your desired type of data, for example:

    void myActuation(String data) {;
      Serial.print("Youve received a message from AllThingsTalk: ");
      Serial.println(data);
    }
    void myActuation2(bool data) {
      digitalWrite(D4, data); // Turns on/off built-in LED on NodeMCU depending on value received from AllThingsTalk Maker
    }
    ...

You can define as many actuations as you want. You’re only limited by your hardware.


# Debug

The library outputs useful information such as your WiFi details, AllThingsTalk connection details, connection status details and errors, messages going in/out, raw messages and more.
Output of this information is not enabled by default. To enable it, you just need to pass the name of your Serial interface to the AllThingsTalk device initialization function.

For example, if you have `Serial.begin(115200)` in your `setup()` function, you would initialize your device like this:

    void setup() {
      Serial.begin(115200);
      device.init(Serial);
    }

That’s it! You should now see debug information from the library along with your serial output (if you have some).


> You can choose any baud rate you want, but higher speed baud rates mean less time wasted outputting messages.


> If you do not want debug information on your Serial, just use `device.init()`.
> If you wish to enable debug and disable the connection LED, use `device.init(Serial, false);`



# Code examples

**Send a message using JSON**

    #include <AllThingstTalk.h>
    auto wifiCreds = WifiCredentials("SSID", "Password");
    auto deviceCreds = DeviceConfig("device_id", "device_token");
    auto device = Device(wifiCreds, deviceCreds);
    
    void setup() {
      device.init();
      device.send("asset_name", "Hello World!");
    }
    
    void loop() {
    }

**Send a message using CBOR**

    #include <AllThingstTalk.h>
    
    auto wifiCreds = WifiCredentials("SSID", "Password");
    auto deviceCreds = DeviceConfig("device_id", "device_token");
    auto device = Device(wifiCreds, deviceCreds);
    CborPayload = payload;
    
    int value = 0;
    
    void setup() {
      Serial.begin(115200);
      device.init(Serial);
    }
    
    void loop() {
      device.loop();
      ++value;
      payload.reset();
      payload.set("counter", value);
      device.send(payload);
      delay(2000);
    }

**Receive a message**
… and print it out via Serial and relay it back to an AllThingsTalk asset via CBOR:

    #include <AllThingstTalk.h>
    auto wifiCreds = WifiCredentials("SSID", "Password");
    auto deviceCreds = DeviceConfig("device_id", "device_token");
    auto device = Device(wifiCreds, deviceCreds);
    CborPayload payload;
    
    void setup() {
      Serial.begin(115200);
      device.init(Serial);
      device.setActuation("actuator", actuation);
    }
    
    void loop() {
      device.loop();
    }
    
    void actuation(String value) {
      Serial.print("Received message: ");
      Serial.println(value);
      payload.reset();
      payload.set("sensor", value);
      device.send(payload)
    }

