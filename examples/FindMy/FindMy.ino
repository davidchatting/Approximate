/*
    Find My example for the Approximate Library
    -
    Report the signal strenth of a device (specificed by its MAC address)
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include <Approximate.h>
Approximate approx;

//Define for your board, not all have built-in LED:
#if defined(ESP8266)
  const int LED_PIN = 14;
#elif defined(ESP32)
  const int LED_PIN = 2;
#endif
bool ledState = LOW;
int currentRSSI = 1;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setActiveDeviceFilter("XX:XX:XX:XX:XX:XX", Filter::SENDS);
    approx.setActiveDeviceHandler(onActiveDevice);
    approx.start();
  }
}

void loop() {
  approx.loop();

  if(currentRSSI < 0) {
    int t = map(currentRSSI, -100, 0, 1000, 0);
    digitalWrite(LED_PIN, ledState);
    ledState = !ledState;
    delay(t);
  }
}

void onActiveDevice(Device *device, Approximate::DeviceEvent event) {
  currentRSSI = device->getRSSI();
}