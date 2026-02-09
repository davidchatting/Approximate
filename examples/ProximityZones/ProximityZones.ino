/*
    Proximity Zones example for the Approximate Library
    -
    Classify nearby devices into proximity zones based on RSSI signal strength:
      INTIMATE  (< 0.5m)  RSSI > -20
      PERSONAL  (0.5-1.5m) RSSI > -40
      SOCIAL    (1.5-3m)   RSSI > -60
      PUBLIC    (3-5m)     RSSI > -80
    -
    Demonstrates setProximateRSSIThreshold, setProximateLastSeenTimeoutMs,
    and the RSSI threshold constants.
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021, Updated 2026
*/

#include <Approximate.h>
Approximate approx;

void onProximateDevice(Device *device, Approximate::DeviceEvent event);
const char* getProximityZone(int rssi);

void setup() {
  Serial.begin(9600);

  if (approx.init("MyHomeWiFi", "password", false)) {
    // Use PUBLIC threshold to detect at maximum range
    Approximate::setProximateRSSIThreshold(APPROXIMATE_PUBLIC_RSSI);
    // Devices depart after 5 seconds without a packet
    Approximate::setProximateLastSeenTimeoutMs(5000);

    approx.setProximateDeviceHandler(onProximateDevice);
    approx.begin();
  }
}

void loop() {
  approx.loop();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  int rssi = device->getRSSI();

  switch (event) {
    case Approximate::ARRIVE:
      Serial.printf("ARRIVE\t%s\t[%s]\tRSSI: %i\n",
        device->getMacAddressAsString().c_str(),
        getProximityZone(rssi),
        rssi);
      break;
    case Approximate::DEPART:
      Serial.printf("DEPART\t%s\n",
        device->getMacAddressAsString().c_str());
      break;
    case Approximate::PROBE:
      Serial.printf("PROBE\t%s\t[%s]\tRSSI: %i\n",
        device->getMacAddressAsString().c_str(),
        getProximityZone(rssi),
        rssi);
      break;
    default:
      break;
  }
}

const char* getProximityZone(int rssi) {
  if (rssi > APPROXIMATE_INTIMATE_RSSI) return "INTIMATE";
  if (rssi > APPROXIMATE_PERSONAL_RSSI) return "PERSONAL";
  if (rssi > APPROXIMATE_SOCIAL_RSSI)   return "SOCIAL";
  return "PUBLIC";
}
