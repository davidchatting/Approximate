/*
    Close By example for the Approximate Library
    -
    Find the MAC address of close by devices - and trigger ARRIVE & DEPART events
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#when-were-close-using-a-proximate-device-handler
*/

#include <Approximate.h>
Approximate approx;

//Define for your board, not all have built-in LED and/or button:
#if defined(ESP8266)
  const int LED_PIN = 14;
#elif defined(ESP32)
  const int LED_PIN = 2;
#endif

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.setProximateDeviceHandler(onProximateDevice, APPROXIMATE_PERSONAL_RSSI);
        approx.begin();
    }
}

void loop() {
    approx.loop();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
    switch(event) {
        case Approximate::ARRIVE:
            digitalWrite(LED_PIN, HIGH);
            Serial.println("ARRIVE\t" + device->getMacAddressAsString());
            break;
        case Approximate::DEPART:
            digitalWrite(LED_PIN, LOW);
            Serial.println("DEPART\t" + device->getMacAddressAsString());
            break;
    }
}