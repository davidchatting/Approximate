/*
    Find My example for the Approximate Library
    -
    Report the signal strenth of a device (specificed by its MAC address)
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#find-my--using-an-active-device-handler
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
long ledToggleAtMs = 0;
int ledToggleIntervalMs = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setActiveDeviceFilter("XX:XX:XX:XX:XX:XX");
    approx.setActiveDeviceHandler(onActiveDevice);
    approx.begin();
  }
}

void loop() {
  approx.loop();

  digitalWrite(LED_PIN, ledState);
  
  if(ledToggleIntervalMs > 0 && millis() > ledToggleAtMs) {
    ledState = !ledState;
    ledToggleAtMs = millis() + ledToggleIntervalMs;
  }
}

void onActiveDevice(Device *device, Approximate::DeviceEvent event) {
  if(event == Approximate::SEND) {  
    ledToggleIntervalMs = map(device->getRSSI(), -100, 0, 1000, 0);
  }
}