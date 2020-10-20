# Approximate Library
Approximate is a WiFi [Arduino](http://www.arduino.cc/download) Library for building proximate interactions with the ESP8266 and ESP32

The ESP8266 and ESP32 modules work only with 2.4GHz WiFi and can not interact with 5GHz networks - so the Approximate library has this limitation too.

## When We're Close...
Discover a device when it is in proximity...

```
#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.setProximateDeviceHandler(onCloseByDevice);
        approx.start();
    }
}

void loop() {
    approx.loop();
}

void onCloseByDevice(Device *device, Approximate::DeviceEvent event) {
    switch(event) {
        case Approximate::ARRIVE:
            Serial.println("ARRIVE\t" + device->getMacAddressAsString());
            break;
        case Approximate::DEPART:
            Serial.println("DEPART\t" + device->getMacAddressAsString());
            break;
    }
}
```

## Find My...
Find a device on your WiFi network using its signal strength - you can search by [MAC address](https://en.wikipedia.org/wiki/MAC_address) or by manufacter with the [OUI code](https://en.wikipedia.org/wiki/Organizationally_unique_identifier).

```
#include <Approximate.h>
Approximate approx;

const int LED_PIN = 2;
bool ledState = LOW;
int currentRSSI = 0;

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
```
