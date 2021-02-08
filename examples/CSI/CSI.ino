/*
    CSI example for the Approximate Library
    -
    Find the MAC address of close by devices - and trigger ARRIVE & DEPART events
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#when-were-close-using-a-proximate-device-handler
*/

#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.setChannelStateHandler(onChannelStateEvent);
        approx.begin();
    }
}

void loop() {
    approx.loop();
}

void onChannelStateEvent() {
}