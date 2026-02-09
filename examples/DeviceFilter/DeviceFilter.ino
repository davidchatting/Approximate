/*
    Device Filter example for the Approximate Library
    -
    Filter active devices by OUI (Organizationally Unique Identifier) to detect
    specific manufacturer devices on the network. Demonstrates addActiveDeviceFilter
    and removeActiveDeviceFilter with OUI-based filtering.
    -
    Common OUIs (see http://standards-oui.ieee.org/oui.txt):
      0xD8F15B  Sonoff (Espressif)
      0xA4CF12  Espressif
      0x3C71BF  Espressif
      0xDCA632  Apple
      0x98E743  Apple
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021, Updated 2026
*/

#include <Approximate.h>
Approximate approx;

// Define for your board, not all have built-in LED:
#if defined(ESP8266)
  const int LED_PIN = 14;
#elif defined(ESP32)
  const int LED_PIN = 2;
#endif

void onActiveDevice(Device *device, Approximate::DeviceEvent event);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    // Filter for Espressif devices by OUI
    approx.addActiveDeviceFilter(0xA4CF12);
    approx.addActiveDeviceFilter(0x3C71BF);
    approx.addActiveDeviceFilter(0xD8F15B);

    approx.setActiveDeviceHandler(onActiveDevice);
    approx.begin();
  }
}

void loop() {
  approx.loop();
}

void onActiveDevice(Device *device, Approximate::DeviceEvent event) {
  switch (event) {
    case Approximate::SEND:
      digitalWrite(LED_PIN, HIGH);
      Serial.printf("SEND\t%s\tOUI: 0x%06X\tRSSI: %i\t%i bytes\n",
        device->getMacAddressAsString().c_str(),
        device->getOUI(),
        device->getRSSI(),
        device->getPayloadSizeBytes());
      break;
    case Approximate::RECEIVE:
      digitalWrite(LED_PIN, LOW);
      Serial.printf("RECV\t%s\tOUI: 0x%06X\tRSSI: %i\t%i bytes\n",
        device->getMacAddressAsString().c_str(),
        device->getOUI(),
        device->getRSSI(),
        device->getPayloadSizeBytes());
      break;
    case Approximate::PROBE:
      Serial.printf("PROBE\t%s\tOUI: 0x%06X\tRSSI: %i\n",
        device->getMacAddressAsString().c_str(),
        device->getOUI(),
        device->getRSSI());
      break;
    default:
      break;
  }
}
