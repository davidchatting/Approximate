# Approximate Library
Approximate is a WiFi [Arduino](http://www.arduino.cc/download) Library for building proximate interactions with the ESP8266 and ESP32.

The Approximate library works 2.4GHz WiFi networks, but not with 5GHz networks as neither ESP8266 or ESP32 support this technology.

## When We're Close...
Discover a device in proximity...

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

`setProximateDeviceHandler()` takes a `DeviceHandler` callback function parameter (here `onCloseByDevice`) that will be called in the case that one of these events occurs for a device in proximity:

* `Approximate::ARRIVE` once when the device first arrives in proximity

* `Approximate::DEPART` once when the device departs and is no longer seen in proximity

* `Approximate::UPLOAD` everytime the device uploads (sends) data

* `Approximate::DOWNLOAD` everytime the device downloads (receives) data

`setProximateDeviceHandler()` has two further optional parameters that define how proximity is managed, the full definition is:

```
void Approximate::setProximateDeviceHandler(DeviceHandler deviceHandler, int rssiThreshold = APPROXIMATE_PERSONAL_RSSI, int lastSeenTimeoutMs = 60000);
```

The parameter `rssiThreshold` defines the RSSI threshold value that is considered to be in proximity. ([RSSI](https://en.wikipedia.org/wiki/Received_signal_strength_indication)) is a measure of WiFi signal strength used to estimate proximity. While `rssiThreshold` can be defined numerically, four predefined vales are available for use, borrowing from the language of [proxemics](https://en.wikipedia.org/wiki/Proxemics):

* `APPROXIMATE_INTIMATE_RSSI`
* `APPROXIMATE_PERSONAL_RSSI`
* `APPROXIMATE_SOCIAL_RSSI`
* `APPROXIMATE_PUBLIC_RSSI`

`APPROXIMATE_PERSONAL_RSSI` is the default value.

The parameter `lastSeenTimeoutMs` defines how quickly (in milliseconds) a device will be said to `DEPART` if it is unseen. While the `ARRIVE` event is triggered only once for a device, further observations will cause `UPLOAD` and `DOWNLOAD` events; when these events stop and after a wait of `lastSeenTimeoutMs`, a `DEPART` event will then be generated. A suitable value will depend on the dynamics of the application and devices' use of the network. One minute (60,000 ms) is the default value.

The callback function delivers both a pointer to a `Device` and a `Approximate::DeviceEvent` for each event. This example identifies the device by its [MAC address](https://en.wikipedia.org/wiki/MAC_address) and demonstrates the `Device::getMacAddressAsString()` function. [MAC addresses](https://en.wikipedia.org/wiki/MAC_address) are the primary way in which the Approximate library identifies devices.

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
