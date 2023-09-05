/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2019 AllThingsTalk
 * Author: Vanja
 * https://allthingstalk.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This library makes connecting your Arduino devices with your AllThingsTalk Maker
 * a breeze, but it has quite a few more tricks up its sleeve.
 * Detailed instructions for this library can be found: https://github.com/allthingstalk/arduino-wifi-sdk
 */

#include "AllThingsTalk_WiFi.h"
#include "Arduino.h"
#include "CborPayload.h"
#include "GeoLocation.h"
#include "BinaryPayload.h"
#include "PubSubClient.h"
#include <ArduinoJson.h>

#ifdef ARDUINO_SAMD_MKRWIFI1010
#include <Scheduler.h>
#include <WiFiNINA.h>
#define SUPPORTS // Prevents error if no devices are supported by SDK
#endif

#ifdef ESP8266
#include <Ticker.h>
#include <ESP8266WiFi.h>
#define SUPPORTS // Prevents error if no devices are supported by SDK
#endif

#ifdef ESP32
#include <Ticker.h>
#include <WiFi.h>
//#include <analogWrite.h> // External library, required only for ESP32, https://github.com/ERROPiX/ESP32_AnalogWrite
#define SUPPORTS
#endif


#if defined(ESP8266) || defined(ESP32)
Ticker fader;
#endif
WiFiClient networkClient;
PubSubClient mqtt(networkClient);

#ifndef ESP8266
// #ifdef ARDUINO_SAMD_MKRWIFI1010 || ESP32 // Doesn't compile if this is enabled
Device *Device::instance = nullptr;
#endif
// #endif

// Constructor
Device::Device(WifiCredentials &wifiCreds, DeviceConfig &deviceCreds) {
    #ifndef ESP8266
    if (Device::instance != nullptr) {
        //#error "This device doesn't support more than 1 object";
    }
    Device::instance = this;
    #endif
    this->deviceCreds = &deviceCreds;
    this->wifiCreds = &wifiCreds;
}

// Serial print (debugging)
template<typename T> void Device::debug(T message, char separator) {
    if (debugSerial) {
        debugSerial->print(message);
        if (separator) {
            debugSerial->print(separator);
        }
    }
}

// Serial print (verbose debugging)
template<typename T> void Device::debugVerbose(T message, char separator) {
    if (debugVerboseEnabled) {
        if (debugSerial) {
            debugSerial->print(message);
            if (separator) {
                debugSerial->print(separator);
            }
        }
    }
}

// Start fading the Connection LED
void Device::connectionLedFadeStart() {
    #if defined(ESP8266) //|| defined(ESP32)
    if (ledEnabled == true) {
        if (fader.active() == false) {
            fader.attach_ms(1, std::bind(&Device::connectionLedFadeStart, this));
        } else {
            unsigned long thisMillis = millis();
            if (thisMillis - previousFadeMillis >= fadeInterval) {
                if (fadeDirection == UP) {
                    fadeValue = fadeValue + fadeIncrement;
                    if (fadeValue >= maxPWM) {
                        fadeValue = maxPWM;
                        fadeDirection = DOWN;
                    }
            } else {
                fadeValue = fadeValue - fadeIncrement;
                if (fadeValue <= minPWM) {
                    fadeValue = minPWM;
                    fadeDirection = UP;
                }
            }
            analogWrite(connectionLedPin, fadeValue);
            previousFadeMillis = thisMillis;
            }
        }
    }
    #endif
    #ifdef ARDUINO_SAMD_MKRWIFI1010
    if (ledEnabled) {
        supposedToFade = true;
        if (!schedulerActive) {
            Scheduler.startLoop(Device::connectionLedFade);
            schedulerActive = true;
        }
    }
    #endif
}

// Stop the Connection LED
void Device::connectionLedFadeStop() {
    #if defined(ESP8266) || defined(ESP32)
    if (ledEnabled == true) {
        fader.detach();
        while (fadeValue <= maxPWM) {
            delay(fadeInterval);
            fadeValue = fadeValue + fadeIncrement;
            analogWrite(connectionLedPin, fadeValue);
        }
        fadeValue = maxPWM;
        for (int i=0; i<=1; i++) {
            delay(100);
            analogWrite(connectionLedPin, minPWM);
            delay(50);
            analogWrite(connectionLedPin, maxPWM);
        }
    }
    #endif
    #ifdef ARDUINO_SAMD_MKRWIFI1010
    supposedToFade = false;
    supposedToStop = true;
    fadeOut = true;
    #endif
}

#ifdef ARDUINO_SAMD_MKRWIFI1010
// Actual logic for fading, fade-out and post-fade-out blinking of Connection LED
void Device::connectionLedFade() {
    if (instance->ledEnabled) {
        unsigned long thisMillis = millis();
        if (instance->supposedToStop) {
            if (thisMillis - instance->previousFadeOutMillis >= instance->fadeInterval && instance->fadeOut) {
                instance->fadeValue = instance->fadeValue - instance->fadeIncrement;
                if (instance->fadeValue <= instance->minPWM) {
                    instance->fadeValue = instance->minPWM;
                    instance->fadeDirection = UP;
                    instance->fadeOutBlinkIteration = 0;
                    instance->fadeOut = false;
                    instance->fadeOutBlink = true;
                    yield();
                }
                analogWrite(instance->connectionLedPin, instance->fadeValue);
                instance->previousFadeOutMillis = thisMillis;
                yield();
            }
            if (thisMillis - instance->previousFadeOutBlinkMillis >= instance->blinkInterval && instance->fadeOutBlink) {
                switch(instance->fadeOutBlinkIteration) {
                    case 0:
                        instance->fadeValue = instance->minPWM;
                        break;
                    case 1:
                        instance->fadeValue = instance->maxPWM;
                        break;
                    case 2:
                        instance->fadeValue = instance->minPWM;
                        break;
                    case 3:
                        instance->fadeValue = instance->maxPWM;
                        break;
                    case 4:
                        instance->fadeValue = instance->minPWM;
                        break;
                    case 5:
                        instance->fadeOutBlink = false;
                        instance->supposedToStop = false;
                        break;
                }
                analogWrite(instance->connectionLedPin, instance->fadeValue);
                instance->fadeOutBlinkIteration++;
                instance->previousFadeOutBlinkMillis = thisMillis;
                yield();
            }
        }
        if (instance->supposedToFade) {
            if (thisMillis - instance->previousFadeMillis >= instance->fadeInterval) {
                if (instance->fadeDirection == UP) {
                    instance->fadeValue = instance->fadeValue + instance->fadeIncrement;
                    if (instance->fadeValue >= instance->maxPWM) {
                        instance->fadeValue = maxPWM;
                        instance->fadeDirection = DOWN;
                    }
                    yield();
                } else {
                    instance->fadeValue = instance->fadeValue - instance->fadeIncrement;
                    if (instance->fadeValue <= instance->minPWM) {
                        instance->fadeValue = instance->minPWM;
                        instance->fadeDirection = UP;
                    }
                    yield();
                }
                analogWrite(instance->connectionLedPin, instance->fadeValue);
                instance->previousFadeMillis = thisMillis;
                yield();
            }
            yield();
        }
        yield();
    }
    yield();
}
#endif


// Used to check if wifiSignalReporting is enabled
bool Device::wifiSignalReporting() {
    if (rssiReporting) {
        return true;
    } else {
        return false;
    }
}

// Used to set wifiSignalReporting on/off
bool Device::wifiSignalReporting(bool state) {
    rssiReporting = state;
    return true;
}

// Used to set wifiSignalReporting interval
bool Device::wifiSignalReporting(int time) {
    rssiReportInterval = time;
    return true;
}

// Used to set wifiSignalReporting on/off and interval in one go
bool Device::wifiSignalReporting(bool state, int time) {
    rssiReportInterval = time;
    rssiReporting = state;
    return true;
}

// Used to check if connectionLed is enabled or disabled
bool Device::connectionLed() {
    if (ledEnabled) {
        return true;
    } else {
        return false;
    }
}

// Used to set connectionLed on/off
bool Device::connectionLed(bool state) {
    ledEnabled = state;
    return true;
}

// Used to set a custom pin for connectionLed
bool Device::connectionLed(int ledPin) {
    connectionLedPin = ledPin;
    return true;
}

// Used to set connectionLed on/off and custom pin in one go
bool Device::connectionLed(bool state, int ledPin) {
    connectionLedPin = ledPin;
    ledEnabled = state;
    return true;
}

// Used to enable debug output
void Device::debugPort(Stream &debugSerial) {
    this->debugSerial = &debugSerial;
#ifdef ARDUINO_SAMD_MKRWIFI1010
    delay(5000);
#endif
    debug("");
    debug("------------- AllThingsTalk WiFi SDK Serial Begin -------------");
    debug("Debug Level: Normal");
}

// Used to enable verbose debug output
void Device::debugPort(Stream &debugSerial, bool verbose) {
    debugVerboseEnabled = verbose;
    this->debugSerial = &debugSerial;
#ifdef ARDUINO_SAMD_MKRWIFI1010
    delay(5000); // SAMD supports direct USB communication which causes the COM port to drop when reset. This gives the user time for the serial monitor to pick it up
#endif
    debug("");
    debug("------------- AllThingsTalk WiFi SDK Serial Begin -------------");
    if (!verbose) debug("Debug Level: Normal");
    debugVerbose("Debug Level: Verbose");
    
}

// Generate Unique MQTT ID
void Device::generateRandomID() {
#ifdef ESP8266
    sprintf(mqttId, "%s%i", "arduino-", ESP.getChipId());
#endif
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ESP32)
    byte mac[6];
    WiFi.macAddress(mac);
    sprintf(mqttId, "arduino-%2X%2X%2X%2X", mac[3], mac[2], mac[1], mac[0]);
#endif
    debugVerbose("Unique MQTT ID of Device:", ' ');
    debugVerbose(mqttId);
}

// Shows Device ID and Device Token via Serial in a hidden way (for visual verification)
void Device::showMaskedCredentials() {
    if (debugVerboseEnabled) {
        String hiddenDeviceId = deviceCreds->getDeviceId();
        String hiddenDeviceToken = deviceCreds->getDeviceToken();
        String lastFourDeviceId = hiddenDeviceId.substring(20);
        String lastFourDeviceToken = hiddenDeviceToken.substring(41);
        hiddenDeviceId = hiddenDeviceId.substring(0, 4);
        hiddenDeviceToken = hiddenDeviceToken.substring(0, 10);
        hiddenDeviceId += "****************";
        hiddenDeviceId += lastFourDeviceId;
        hiddenDeviceToken += "*******************************";
        hiddenDeviceToken += lastFourDeviceToken;
        debugVerbose("API Endpoint:", ' ');
        debugVerbose(deviceCreds->getHostname());
        debugVerbose("Device ID:", ' ');
        debugVerbose(hiddenDeviceId);
        debugVerbose("Device Token:", ' ');
        debugVerbose(hiddenDeviceToken);
    }
}

// Initialization of everything. Run in setup(), only after defining everything else.
void Device::init() {
    // Start flashing the Connection LED
    connectionLedFadeStart();
    
    #ifdef ARDUINO_SAMD_MKRWIFI1010
    // Check the WiFiNina firmware
    String fv = WiFi.firmwareVersion();
    if (fv < "1.2.4") {
      debug("WARNING: Your Arduino MKR WiFi 1010 is running old WiFiNINA Firmware", ' ');
      debug(fv);
      debug("INFO: To avoid connection issues, upgrade your firmware to the newest version:");
      debug("INFO: https://www.arduino.cc/en/Tutorial/WiFiNINA-FirmwareUpdater");
    }
    #endif

    // Print out information about Connection LED
    if (ledEnabled == false) {
        debug("Connection LED: Disabled");
    } else {
        debug("Connection LED: Enabled - GPIO", ' ');
        debug(connectionLedPin);
        debugVerbose("Please don't use GPIO", ' ');
        debugVerbose(connectionLedPin, ' ');
        debugVerbose("as it's used for Connection LED");
    }
    
    // Print out information about WiFi Signal Reporting
    if (rssiReporting == false) {
        debug("WiFi Signal Reporting: Disabled");
    } else {
        debug("WiFi Signal Reporting: Enabled");
        debugVerbose("WiFi Signal Strength will be published every", ' ');
        debugVerbose(rssiReportInterval, ' ');
        debugVerbose("seconds to 'wifi-signal' asset (will be created automatically)");
        createAsset("wifi-signal", "WiFi Signal Strength", "sensor", "string");
    }

    // Generate MQTT ID
    generateRandomID();

    // Print out the Device ID and Device Token in a hidden way (for visual verification)
    showMaskedCredentials();

    // Set MQTT Connection Parameters
    mqtt.setServer(deviceCreds->getHostname(), 1883);
    if (callbackEnabled == true) {
        #if defined(ESP8266) || defined(ESP32)
        mqtt.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->mqttCallback(topic, payload, length); });
        #else
        mqtt.setCallback(Device::mqttCallback);
        #endif
    }
    
    // Connect to WiFi and AllThingsTalk
    connectWiFi();
    createAssets();
    connectAllThingsTalk();
}

// Needs to be run in program loop in order to keep connections alive
void Device::loop() {
    maintainWiFi();
    mqtt.loop();
    reportWiFiSignal();
    maintainAllThingsTalk();
    yield();
}

// Connect to both WiFi and ATT
void Device::connect() {
    debug("Connecting to both WiFi and AllThingsTalk...");
    connectWiFi();
    connectAllThingsTalk();
}

// Disconnect both WiFi and ATT
void Device::disconnect() {
    debug("Disconnecting from both AllThingsTalk and WiFi...");
    disconnectAllThingsTalk();
    disconnectWiFi();
}

// Main method to connect to WiFi
void Device::connectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        connectionLedFadeStart();
        #if defined(ESP8266) || defined(ESP32)
        WiFi.mode(WIFI_STA);
        #endif
        if (wifiHostnameSet) {
            #ifdef ESP8266
            WiFi.hostname(wifiHostname);
            #endif
            #if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ESP32)
            WiFi.setHostname(wifiHostname);
            #endif
            debugVerbose("WiFi Hostname:", ' ');
            debugVerbose(wifiHostname);
        }
        debug("Connecting to WiFi:", ' ');
        debug(wifiCreds->getSsid(), '.');
        WiFi.begin(wifiCreds->getSsid(), wifiCreds->getPassword());
        while (WiFi.status() != WL_CONNECTED) {
            debug("", '.');
            delay(1500);
        }
        debug("");
        debug("Connected to WiFi!");
        connectionLedFadeStop();
        debugVerbose("IP Address:", ' ');
        debugVerbose(WiFi.localIP());
        debugVerbose("WiFi Signal:", ' ');
        debugVerbose(wifiSignal());
        disconnectedWiFi = false;
    }
}

// Checks and recovers WiFi if lost
void Device::maintainWiFi() {
    if (!disconnectedWiFi) {
        if (WiFi.status() != WL_CONNECTED) {
            connectionLedFadeStart();
            debug("WiFi Connection Dropped! Reason:", ' ');
            switch(WiFi.status()) {
                case WL_CONNECTED:
                    debug("Seems like WiFi is connected, but it's giving an error (WL_CONNECTED)");
                    break;
                case WL_NO_SHIELD:
                    debug("No WiFi Shield Present (WL_NO_SHIELD)");
                    break;
                case WL_IDLE_STATUS:
                    debug("WiFi.begin is currently trying to connect... (WL_IDLE_STATUS)");
                    break;
                case WL_NO_SSID_AVAIL:
                    debug("SSID Not Available (WL_NO_SSID_AVAIL)");
                    break;
                case WL_SCAN_COMPLETED:
                    debug("WiFi Scan Completed (WL_SCAN_COMPLETED)");
                    break;
                case WL_CONNECT_FAILED:
                    debug("Connection lost and failed to connect after many attempts (WL_CONNECT_FAILED)");
                    break;
                case WL_CONNECTION_LOST:
                    debug("Connection Lost (WL_CONNECTION_LOST)");
                    break;
                case WL_DISCONNECTED:
                    debug("Intentionally disconnected from the network (WL_DISCONNECTED)");
                    break;
                default:
                    debug("Unknown");
                    break;
            }
            connectWiFi();
            if (!disconnectedAllThingsTalk) {
                connectAllThingsTalk();
            }
        }
    }
}

// Main method for disconnecting from WiFi
void Device::disconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        disconnectAllThingsTalk();
        WiFi.disconnect();
        disconnectedWiFi = true;
        while (WiFi.status() == WL_CONNECTED) {
            delay(1000);
        }
        debug("Successfully Disconnected from WiFi");
    }
}

// Used to set hostname that the device will present itself as on the network
#ifdef ESP8266
bool Device::setHostname(String hostname) {
    wifiHostname = hostname;
    wifiHostnameSet = true;
    return true;
}
#else
// This one is for ESP32 and MKR1010
bool Device::setHostname(const char* hostname) {
    wifiHostname = hostname;
    wifiHostnameSet = true;
    return true;
}
#endif


// Used by the user to create a new asset on AllThingsTalk
bool Device::createAsset(String name, String title, String assetType, String dataType) {
    if (assetType == "sensor" || assetType == "actuator" || assetType == "virtual") {
    } else {
        String output = "Asset '" + name + "' (" + title + ") will not be created on AllThingTalk because it has an invalid asset type '" + assetType + "'.";
        debug(output);
        return false;
    }
    if (dataType == "boolean" || dataType == "string" || dataType == "integer" || dataType == "number" || dataType == "object" ||dataType == "array" ||  dataType == "location") {
    } else {
        String output = "Asset '" + name + "' (" + title + ") will not be created on AllThingTalk because it has an invalid data type '" + dataType + "'.";
        debug(output);
        return false;
    }
    
    assetsToCreate = true;
    assetProperties[assetsToCreateCount].name = name;
    assetProperties[assetsToCreateCount].title = title;
    assetProperties[assetsToCreateCount].assetType = assetType;
    assetProperties[assetsToCreateCount].dataType = dataType;
    ++assetsToCreateCount;
    return true;
}

// Used by the SDK to actually create all the assets requested by user
void Device::createAssets() {
    if (assetsToCreate) {
        WiFiClient httpNetworkClient;
        connectionLedFadeStart();
        for (int i=0; i < assetsToCreateCount; i++) {
            if (httpNetworkClient.connect(deviceCreds->getHostname(), 80)) {
                httpNetworkClient.println("PUT /device/" + String(deviceCreds->getDeviceId()) + "/asset/" + assetProperties[i].name  + " HTTP/1.1");
                httpNetworkClient.print(F("Host: "));
                httpNetworkClient.println(deviceCreds->getHostname());
                httpNetworkClient.println(F("Content-Type: application/json"));
                httpNetworkClient.print(F("Authorization: Bearer "));
                httpNetworkClient.println(deviceCreds->getDeviceToken());
                httpNetworkClient.print(F("Content-Length: ")); 
                String contentBody = "{\"title\":\"" + assetProperties[i].title + "\", \"is\":\"" + assetProperties[i].assetType + "\",\"profile\":{\"type\":\"" + assetProperties[i].dataType + "\"}}";
                httpNetworkClient.println(contentBody.length());
                httpNetworkClient.println();
                httpNetworkClient.println(contentBody);
                httpNetworkClient.println();
                
                unsigned long maxTime = millis() + 2000; // Wait 2 seconds at most before quitting
                while (millis() < maxTime) {
                    if (httpNetworkClient.available()) {
                        break;
                    } else {
                        delay(10);
                    }
                }
                if (debugVerboseEnabled) {
                    if (httpNetworkClient.available()) {
                        String response;
                        debugVerbose("---------------- HTTP Response from AllThingsTalk (Begin) ----------------");
                        while (httpNetworkClient.available()) {
                            char c = httpNetworkClient.read();
                            response += c;
                        }
                        debugVerbose(response);
                        debugVerbose("----------------- HTTP Response from AllThingsTalk (End) -----------------");
                    }
                } else {
                    if (httpNetworkClient.available()) {
                        String output;
                        while (httpNetworkClient.available()) { // This is of dubious value.
                            if (!httpNetworkClient.find("HTTP/1.1")) {
                                break;
                            }
                            int responseCode = httpNetworkClient.parseInt();
                            switch (responseCode) {
                                case 200:
                                    output = "Updated existing " + assetProperties[i].dataType + " " + assetProperties[i].assetType + " asset '" + assetProperties[i].name + "' (" + assetProperties[i].title + ") on AllThingsTalk";
                                    debug(output);
                                    break;
                                case 201:
                                    output = "Created a " + assetProperties[i].dataType + " " + assetProperties[i].assetType + " asset '" + assetProperties[i].name + "' (" + assetProperties[i].title + ") on AllThingsTalk";
                                    debug(output);
                                    break;
                                default:
                                    output = "Failed to create a " + assetProperties[i].dataType + " " + assetProperties[i].assetType + " asset '" + assetProperties[i].name + "' (" + assetProperties[i].title + ") on AllThingsTalk. HTTP Response Code: " + responseCode;
                                    debug(output);
                                    break;
                            }
                            break;
                        }
                    }
                }
                httpNetworkClient.stop();
            } else {
                debug("Asset couldn't be created on AllThingsTalk because the HTTP connection failed.");
            }
        }
    }
}

// Connect to AllThingsTalk (and WiFi if it's disconnected)
void Device::connectAllThingsTalk() {
    if (!mqtt.connected()) {
        connectionLedFadeStart();
        connectWiFi(); // WiFi needs to be present of course
        debug("Connecting to AllThingsTalk", '.');
        while (!mqtt.connected()) {
            yield();
            if (WiFi.status() != WL_CONNECTED) {
                debug(" "); // Cosmetic only.
                maintainWiFi(); // In case WiFi connection is lost while connecting to ATT
            }
            if (!mqtt.connected()) { // Double check while running to avoid double debug output (because maintainWiFi() also calls this method if ATT isn't connected)
                if (mqtt.connect(mqttId, deviceCreds->getDeviceToken(), "arbitrary")) {
                    if (callbackEnabled == true) {
                        // Build the subscribe topic
                        char command_topic[256];
                        snprintf(command_topic, sizeof command_topic, "%s%s%s", "device/", deviceCreds->getDeviceId(), "/asset/+/command");
                        mqtt.subscribe(command_topic); // Subscribe to it
                    }
                    disconnectedAllThingsTalk = false;
                    debug("");
                    debug("Connected to AllThingsTalk!");
                    connectionLedFadeStop();
                    if (rssiReporting) send(wifiSignalAsset, wifiSignal()); // Send WiFi Signal Strength upon connecting
                } else {
                    debug("", '.');
                    delay(1000);
                }
            }
            yield();     
        }
    }
}

// Used to monitor AllThingsTalk connection and reconnect if dropped
void Device::maintainAllThingsTalk() {
    if (!disconnectedAllThingsTalk) {
        if (!mqtt.connected()) {
            connectionLedFadeStart();
            debug("AllThingsTalk Connection Dropped! Reason:", ' ');
            switch (mqtt.state()) {
                case -4:
                    debug("Server didn't respond within the keepalive time");
                    break;
                case -3:
                    debug("Network connection was broken");
                    break;
                case -2:
                    debug("Network connection failed.");
                    debugVerbose("This is a general error. Check if the asset you're publishing to exists on AllThingsTalk.");
                    break;
                case -1:
                    debug("Client disconnected cleanly (intentionally)");
                    break;
                case 0:
                    debug("Seems like client is connected. Restart device.");
                    break;
                case 1:
                    debug("Server doesn't support the requested protocol version");
                    break;
                case 2:
                    debug("Server rejected the client identifier");
                    break;
                case 3:
                    debug("Server was unable to accept the connection");
                    break;
                case 4:
                    debug("Bad username or password");
                    break;
                case 5:
                    debug("Client not authorized to connect");
                    break;
                default:
                    debug("Unknown");
                    break;
            }
            connectAllThingsTalk();
        }
    }
}

// Used to disconnect from AllThingsTalk
void Device::disconnectAllThingsTalk() {
    if (mqtt.connected()) {
        mqtt.disconnect();
        disconnectedAllThingsTalk = true;
        while (mqtt.connected()) {}
        if (!mqtt.connected()) {
            debug("Successfully Disconnected from AllThingsTalk");
        } else {
            debug("Failed to disconnect from AllThingsTalk");
        }
    }
}

// Called from loop; Reports wifi signal to ATTalk Maker at specified interval
void Device::reportWiFiSignal() {
    if (rssiReporting && WiFi.status() == WL_CONNECTED) {
        if (millis() - rssiPrevTime >= rssiReportInterval*1000) {
            send(wifiSignalAsset, wifiSignal());
            rssiPrevTime = millis();
        }
    }
}


// Reads wifi signal in RSSI and converts it to string
String Device::wifiSignal() {
    if (WiFi.status() == WL_CONNECTED) {
        int signal = WiFi.RSSI();
        String signalString;
        if (signal < -87) {
            signalString = "Horrible";
        } else if (signal >= -87 && signal <= -80) {
            signalString = "Bad";
        } else if (signal > -80 && signal <= -70) {
            signalString = "Decent";
        } else if (signal > -70 && signal <= -55) {
            signalString = "Good";
        } else if (signal > -55) {
            signalString = "Excellent";
        } else {
            signalString = "Error converting RSSI Values";
            debugVerbose("Error converting RSSI Values to WiFi Strength String");
        }
        return signalString;
    } else {
        debug("Can't read WiFi Signal Strength because you're not connected to WiFi");
        return "Can't read WiFi Signal Strength because you're not connected to WiFi";
    }
    return "Error getting WiFi Signal Strength";
}

// Add boolean callback (0)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(bool payload)) {
    debugVerbose("Adding a Boolean Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 0);
}

// Add integer callback (1)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(int payload)) {
    debugVerbose("Adding an Integer Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 1);
}

// Add double callback (2)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(double payload)) {
    debugVerbose("Adding a Double Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 2);
}

// Add float callback (3)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(float payload)) {
    debugVerbose("Beware that the maximum value of float in 32-bit systems is 2,147,483,647");
    debugVerbose("Adding a Float Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 3);
}

// Add char callback (4)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(const char* payload)) {
    debugVerbose("Adding a Char Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 4);
}

// Add String callback (5)
bool Device::setActuationCallback(String asset, void (*actuationCallback)(String payload)) {
    debugVerbose("Adding a String Actuation Callback for Asset:", ' ');
    return tryAddActuationCallback(asset, (void*) actuationCallback, 5);
}

// Actual saving of added callbacks
bool Device::tryAddActuationCallback(String asset, void (*actuationCallback), int actuationCallbackArgumentType) {
   if (actuationCallbackCount >= maximumActuations) {
       debug("");
       debug("You've added too many actuations. The maximum is", ' ');
       debug(maximumActuations);
       return false;
   }
    callbackEnabled = true;
    actuationCallbacks[actuationCallbackCount].asset = asset;
    actuationCallbacks[actuationCallbackCount].actuationCallback = actuationCallback;
    actuationCallbacks[actuationCallbackCount].actuationCallbackArgumentType = actuationCallbackArgumentType;
    actuationCallbackCount++;
    debugVerbose(asset);
    return true;
}

// Retrieve a specific callback based on asset
ActuationCallback *Device::getActuationCallbackForAsset(String asset) {
    for (int i = 0; i < actuationCallbackCount; i++) {
        if (asset == actuationCallbacks[i].asset) {
            debugVerbose("Found Actuation Callback for Asset:", ' ');
            debugVerbose(actuationCallbacks[i].asset);
            return &actuationCallbacks[i];
        }
    }
    return nullptr;
}

// Asset name extraction from MQTT topic
String extractAssetNameFromTopic(String topic) {
    // Topic is formed as: device/ID/asset/NAME/state
    const int devicePrefixLength = 38;  // "device/ID/asset/"
    const int stateSuffixLength = 8;  // "/state"
    return topic.substring(devicePrefixLength, topic.length()-stateSuffixLength);
}

/* void ActuationCallback::execute(JsonVariant variant) {
}
*/


#ifdef ESP8266
// MQTT Callback for receiving messages
void Device::mqttCallback(char* p_topic, byte* p_payload, unsigned int p_length) {
    debugVerbose("--------------------------------------");
    debug("< Message Received from AllThingsTalk");
    String payload;
    String topic(p_topic);
    
    debugVerbose("Raw Topic:", ' ');
    debugVerbose(p_topic);

    // Whole JSON Payload
    for (uint8_t i = 0; i < p_length; i++) payload.concat((char)p_payload[i]);
    debugVerbose("Raw JSON Payload:", ' ');
    debugVerbose(payload);
    
    // Deserialize JSON
    DynamicJsonDocument doc(256);
    char json[256];
    for (int i = 0; i < p_length; i++) {
        json[i] = (char)p_payload[i];
    }
    auto error = deserializeJson(doc, json);
    if (error) {
        debug("Parsing JSON failed. Code:", ' ');
        debug(error.c_str());
        return;
    }

    // Extract time from JSON Document
    debugVerbose("Message Time:", ' ');
    const char* time = doc["at"];
    debugVerbose(time);

    String asset = extractAssetNameFromTopic(topic);
    debugVerbose("Asset Name:", ' ');
    debugVerbose(asset);

    // Call actuation callback for this specific asset
    ActuationCallback *actuationCallback = getActuationCallbackForAsset(asset);
    if (actuationCallback == nullptr) {
        debug("Error: There's no actuation callback for this asset.");
        return;
    }

    // Create JsonVariant which we'll use to to check data type and convert if necessary
    JsonVariant variant = doc["value"].as<JsonVariant>();

    //actuationCallback->execute(variant);


/*
    debug("------------------------------------------");
    if (variant.is<bool>()) debug("Value type: BOOLEAN");
    if (variant.is<int>()) debug("Value type: INTEGER");
    if (variant.is<double>()) debug("Value type: DOUBLE");
    if (variant.is<float>()) debug("Value type: FLOAT");
    if (variant.is<const char*>()) debug("Value type: CONST CHAR*");
    debug("------------------------------------------");
*/

    // BOOLEAN
    if (actuationCallback->actuationCallbackArgumentType == 0 && variant.is<bool>()) {
        bool value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: Boolean, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(bool payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // INTEGER
    if (actuationCallback->actuationCallbackArgumentType == 1 && variant.is<int>() && variant.is<double>()) {
        int value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: Integer, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(int payload)>(actuationCallback->actuationCallback)(value);
        return;
    }

    // DOUBLE
    if (actuationCallback->actuationCallbackArgumentType == 2 && variant.is<double>()) {
        double value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: Double, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(double payload)>(actuationCallback->actuationCallback)(value);
        return;
    }

    // FLOAT
    if (actuationCallback->actuationCallbackArgumentType == 3 && variant.is<float>()) {
        float value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: Float, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(float payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // CONST CHAR*
    if (actuationCallback->actuationCallbackArgumentType == 4 && variant.is<const char*>()) {
        const char* value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: const char*, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(const char* payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // STRING
    if (actuationCallback->actuationCallbackArgumentType == 5 && variant.is<const char*>()) {
        String value = doc["value"];
        debugVerbose("Called Actuation for Asset:", ' ');
        debugVerbose(actuationCallback->asset, ',');
        debugVerbose(" Payload Type: String, Value:", ' ');
        debugVerbose(value);
        reinterpret_cast<void (*)(String payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // JSON ARRAY
    if (variant.is<JsonArray>()) {
        debug("Receiving Arrays is not yet supported!");
        return;
    }
    
    // JSON OBJECT
    if (variant.is<JsonObject>()) {
        debug("Receiving Objects is not yet supported!");
        return;
    }
}
#else
// MQTT Callback for receiving messages
void Device::mqttCallback(char* p_topic, byte* p_payload, unsigned int p_length) {
    instance->debugVerbose("--------------------------------------");
    instance->debug("< Message Received from AllThingsTalk");
    String payload;
    String topic(p_topic);
    
    instance->debugVerbose("Raw Topic:", ' ');
    instance->debugVerbose(p_topic);

    // Whole JSON Payload
    for (uint8_t i = 0; i < p_length; i++) payload.concat((char)p_payload[i]);
    instance->debugVerbose("Raw JSON Payload:", ' ');
    instance->debugVerbose(payload);
    
    // Deserialize JSON
    DynamicJsonDocument doc(256);
    char json[256];
    for (int i = 0; i < p_length; i++) {
        json[i] = (char)p_payload[i];
    }
    auto error = deserializeJson(doc, json);
    if (error) {
        instance->debug("Parsing JSON failed. Code:", ' ');
        instance->debug(error.c_str());
        return;
    }

    // Extract time from JSON Document
    instance->debugVerbose("Message Time:", ' ');
    const char* time = doc["at"];
    instance->debugVerbose(time);

    String asset = extractAssetNameFromTopic(topic);
    instance->debugVerbose("Asset Name:", ' ');
    instance->debugVerbose(asset);

    // Call actuation callback for this specific asset
    ActuationCallback *actuationCallback = instance->getActuationCallbackForAsset(asset);
    if (actuationCallback == nullptr) {
        instance->debug("Error: There's no actuation callback for this asset.");
        return;
    }

    // Create JsonVariant which we'll use to to check data type and convert if necessary
    JsonVariant variant = doc["value"].as<JsonVariant>();

    //actuationCallback->execute(variant);


/*
    debug("------------------------------------------");
    if (variant.is<bool>()) debug("Value type: BOOLEAN");
    if (variant.is<int>()) debug("Value type: INTEGER");
    if (variant.is<double>()) debug("Value type: DOUBLE");
    if (variant.is<float>()) debug("Value type: FLOAT");
    if (variant.is<const char*>()) debug("Value type: CHAR*");
    debug("------------------------------------------");
*/

    // BOOLEAN
    if (actuationCallback->actuationCallbackArgumentType == 0 && variant.is<bool>()) {
        bool value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: Boolean, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(bool payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // INTEGER
    if (actuationCallback->actuationCallbackArgumentType == 1 && variant.is<int>() && variant.is<double>()) {
        int value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: Integer, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(int payload)>(actuationCallback->actuationCallback)(value);
        return;
    }

    // DOUBLE
    if (actuationCallback->actuationCallbackArgumentType == 2 && variant.is<double>()) {
        double value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: Double, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(double payload)>(actuationCallback->actuationCallback)(value);
        return;
    }

    // FLOAT
    if (actuationCallback->actuationCallbackArgumentType == 3 && variant.is<float>()) {
        float value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: Float, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(float payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // CONST CHAR*
    if (actuationCallback->actuationCallbackArgumentType == 4 && variant.is<const char*>()) {
        const char* value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: const char*, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(const char* payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // STRING
    if (actuationCallback->actuationCallbackArgumentType == 5 && variant.is<const char*>()) {
        String value = doc["value"];
        instance->debugVerbose("Called Actuation for Asset:", ' ');
        instance->debugVerbose(actuationCallback->asset, ',');
        instance->debugVerbose(" Payload Type: String, Value:", ' ');
        instance->debugVerbose(value);
        reinterpret_cast<void (*)(String payload)>(actuationCallback->actuationCallback)(value);
        return;
    }
    
    // JSON ARRAY
    if (variant.is<JsonArray>()) {
        instance->debug("Receiving Arrays is not yet supported!");
        return;
    }
    
    // JSON OBJECT
    if (variant.is<JsonObject>()) {
        instance->debug("Receiving Objects is not yet supported!");
        return;
    }
}
#endif

// Send data as CBOR
bool Device::send(CborPayload &payload) {
    if (WiFi.status() == WL_CONNECTED) {
        if (mqtt.connected()) {
            char topic[128];
            snprintf(topic, sizeof topic, "%s%s%s", "device/", deviceCreds->getDeviceId(), "/state");
            mqtt.publish(topic, payload.getBytes(), payload.getSize());
            debug("> Message Published to AllThingsTalk (CBOR)");
            return true;
        } else {
            debug("Can't publish message because you're not connected to AllThingsTalk");
            return false;
        }
    } else {
        debug("Can't publish message because you're not connected to WiFi");
        return false;
    }
}

// Send data as Binary Payload
bool Device::send(BinaryPayload &payload) {
    if (WiFi.status() == WL_CONNECTED) {
        if (mqtt.connected()) {
            char topic[128];
            snprintf(topic, sizeof topic, "%s%s%s", "device/", deviceCreds->getDeviceId(), "/state");
            mqtt.publish(topic, payload.getBytes(), payload.getSize());
            debug("> Message Published to AllThingsTalk (Binary Payload)");
            return true;
        } else {
            debug("Can't publish message because you're not connected to AllThingsTalk");
            return false;
        }
    } else {
        debug("Can't publish message because you're not connected to WiFi");
        return false;
    }
}

template<typename T> bool Device::send(char *asset, T payload) {
    if (WiFi.status() == WL_CONNECTED) {
        if (mqtt.connected()) {
            char topic[128];
            snprintf(topic, sizeof topic, "%s%s%s%s%s", "device/", deviceCreds->getDeviceId(), "/asset/", asset, "/state");
            DynamicJsonDocument doc(256);
            char JSONmessageBuffer[256];
            doc["value"] = payload;
            serializeJson(doc, JSONmessageBuffer);
            mqtt.publish(topic, JSONmessageBuffer, false);
            debug("> Message Published to AllThingsTalk (JSON)");
            debugVerbose("Asset:", ' ');
            debugVerbose(asset, ',');
            debugVerbose(" Value:", ' ');
            debugVerbose(payload);
            return true;
        } else {
            debug("Can't publish message because you're not connected to AllThingsTalk");
            return false;
        }
    } else {
        debug("Can't publish message because you're not connected to WiFi");
        return false;
    }
}

template bool Device::send(char *asset, bool payload);
template bool Device::send(char *asset, char *payload);
template bool Device::send(char *asset, const char *payload);
template bool Device::send(char *asset, String payload);
template bool Device::send(char *asset, int payload);
template bool Device::send(char *asset, byte payload);
template bool Device::send(char *asset, short payload);
template bool Device::send(char *asset, long payload);
template bool Device::send(char *asset, float payload);
template bool Device::send(char *asset, double payload);

#ifndef SUPPORTS
Device::Device(WifiCredentials &wifiCreds, DeviceConfig &deviceCreds) {
    #error "Currently, ESP8266 (all ESP8266-based devices) and MKR1010 are supported. Open up an issue on GitHub if you'd like us to support your device."
}
#undef SUPPORTS
#endif
