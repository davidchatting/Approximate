/*
    Close By example for the Approximate Library
    -
    Find the MAC address of close by devices - and trigger ARRIVE & DEPART events
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.setProximateDeviceHandler(onCloseByDevice, APPROXIMATE_PERSONAL_RSSI);
        approx.start();
    }
}

void loop() {
    approx.loop();
}

void onCloseByDevice(Device *device, Approximate::DeviceEvent event) {
    switch(event) {
        case Approximate::ARRIVE:   Serial.println("ARRIVE\t" + device->getMacAddressAsString());    break;
        case Approximate::DEPART:   Serial.println("DEPART\t" + device->getMacAddressAsString());    break;
    }
}