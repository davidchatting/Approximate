/*
    When Active example for the Approximate Library
    -
    Report active devices on the network, optionally filtered by MAC address or OUI
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#watch-device---using-a-proximate-device-handler-and-an-active-device-handler
*/

#include <Approximate.h>
Approximate approx;

//Define for your board, not all have built-in LED:
#if defined(ESP8266)
  const int LED_PIN = 14;
#elif defined(ESP32)
  const int LED_PIN = 2;
#endif

long ledOnUntilMs = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setProximateDeviceHandler(onProximateDevice, APPROXIMATE_PERSONAL_RSSI, /*lastSeenTimeoutMs*/ 1000);
    approx.setActiveDeviceHandler(onActiveDevice, /*inclusive*/ false);
    approx.begin();
  }
}

void loop() {
  approx.loop();
  digitalWrite(LED_PIN, millis() < ledOnUntilMs);
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  switch (event) {
    case Approximate::ARRIVE:
      Serial.println("Watching:  " + device -> getMacAddressAsString());
      approx.setActiveDeviceFilter(device);
      break;
    case Approximate::DEPART:
      break;
  }
}

void onActiveDevice(Device *device, Approximate::DeviceEvent event) {
    if(event == Approximate::RECEIVE) {
      ledOnUntilMs = millis() + (device -> getPayloadSizeBytes()/10);
    }
}