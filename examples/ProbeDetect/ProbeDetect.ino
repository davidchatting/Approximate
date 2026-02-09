/*
    Probe Detect example for the Approximate Library
    -
    Detect nearby devices via WiFi management frames (probe requests, beacons)
    without requiring a WiFi connection or IP address resolution.
    Demonstrates the PROBE event introduced in v2.0.
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021, Updated 2026
*/

#include <Approximate.h>
Approximate approx;

void onProximateDevice(Device *device, Approximate::DeviceEvent event);

void setup() {
  Serial.begin(9600);

  // init with no IP resolution (false) - PROBE events don't need it
  if (approx.init("MyHomeWiFi", "password", false)) {
    approx.setProximateDeviceHandler(onProximateDevice, APPROXIMATE_PERSONAL_RSSI);
    approx.begin();
  }
}

void loop() {
  approx.loop();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  switch (event) {
    case Approximate::ARRIVE:
      Serial.printf("ARRIVE\t%s\tOUI: 0x%06X\tRSSI: %i\n",
        device->getMacAddressAsString().c_str(),
        device->getOUI(),
        device->getRSSI());
      break;
    case Approximate::DEPART:
      Serial.printf("DEPART\t%s\n",
        device->getMacAddressAsString().c_str());
      break;
    case Approximate::PROBE:
      Serial.printf("PROBE\t%s\tRSSI: %i\n",
        device->getMacAddressAsString().c_str(),
        device->getRSSI());
      break;
  }
}
