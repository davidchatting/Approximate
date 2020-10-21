# The Approximate Library
The Approximate library is a WiFi [Arduino](http://www.arduino.cc/download) Library for building proximate interactions with the ESP8266 and ESP32.

Approximate works 2.4GHz WiFi networks, but not 5GHz networks, as neither ESP8266 or ESP32 support this technology.

Every Approximate sketch requires this essential structure - if the specified WiFi network can not be found or the authentication fails `init()` will return false.

```
#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.start();
    }
}

void loop() {
    approx.loop();
}
```

Approximate can interact with devices on your home network in proximity (using a Proximate Device Handler) or simply when they are active (using a Active Device Handler). The examples on this page demonstrate both and combinations of the two.

## When We're Close... using a Proximate Device Handler
Identify a WiFi device in proximity and print out its [MAC address](https://en.wikipedia.org/wiki/MAC_address).

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

The Proximate Device Handler is set by `setProximateDeviceHandler()` that takes a `DeviceHandler` callback function parameter (here `onCloseByDevice`) that will be called in the case that one of these events occurs for a device in proximity:

* `Approximate::ARRIVE` once when the device first arrives in proximity
* `Approximate::DEPART` once when the device departs and is no longer seen in proximity
* `Approximate::SEND` every time the device sends (uploads) data

* `Approximate::RECEIVE` every time the device receives (downloads) data - however unless the router is also in proximity, RECEIVE events will not be seen

`setProximateDeviceHandler()` has two further optional parameters that define how proximity is managed, the full definition is:

```
void Approximate::setProximateDeviceHandler(DeviceHandler deviceHandler, int rssiThreshold = APPROXIMATE_PERSONAL_RSSI, int lastSeenTimeoutMs = 60000);
```

The parameter `rssiThreshold` defines the RSSI threshold value that is considered to be in proximity. [RSSI](https://en.wikipedia.org/wiki/Received_signal_strength_indication) is a measure of WiFi signal strength used to estimate proximity. While `rssiThreshold` can be defined numerically, four predefined vales are available for use, borrowing from the language of [proxemics](https://en.wikipedia.org/wiki/Proxemics):

* `APPROXIMATE_INTIMATE_RSSI`
* `APPROXIMATE_PERSONAL_RSSI`
* `APPROXIMATE_SOCIAL_RSSI`
* `APPROXIMATE_PUBLIC_RSSI`

`APPROXIMATE_PERSONAL_RSSI` is the default value.

The parameter `lastSeenTimeoutMs` defines how quickly (in milliseconds) a device will be said to `DEPART` if it is unseen. While the `ARRIVE` event is triggered only once for a device, further observations will cause `SEND` and (sometimes) `RECEIVE` events; when these events stop and after a wait of `lastSeenTimeoutMs`, a `DEPART` event will then be generated. A suitable value will depend on the dynamics of the application and devices' use of the network. One minute (60,000 ms) is the default value.

The callback function delivers both a pointer to a `Device` and a `Approximate::DeviceEvent` for each event. This example identifies the device by its [MAC address](https://en.wikipedia.org/wiki/MAC_address) and demonstrates the `Device::getMacAddressAsString()` function. MAC addresses are the primary way in which the Approximate library identifies network devices.

## Find My...  using an Active Device Handler
Track down a device on your WiFi network using its signal strength ([RSSI](https://en.wikipedia.org/wiki/Received_signal_strength_indication)) and flash an LED faster the closer it is.

```
#include <Approximate.h>
Approximate approx;

const int LED_PIN = 2;
bool ledState = LOW;
long ledToggleAtMs = 0;
int ledToggleIntervalMs = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setActiveDeviceFilter("XX:XX:XX:XX:XX:XX");
    approx.setActiveDeviceHandler(onActiveDevice);
    approx.start();
  }
}

void loop() {
  approx.loop();

  digitalWrite(LED_PIN, ledState);
  
  if(ledToggleAtMs > 0 && millis() > ledToggleAtMs) {
    ledState = !ledState;
    ledToggleAtMs = millis() + ledToggleIntervalMs;
  }
}

void onActiveDevice(Device *device, Approximate::DeviceEvent event) {
  if(event == Approximate::SEND) {  
    ledToggleIntervalMs = map(device->getRSSI(), -100, 0, 1000, 0);
  }
}
```

The Active Device Handler is set by `Approximate::setActiveDeviceHandler()` that takes a `DeviceHandler` callback function parameter (here `onActiveDevice`), which generates `Approximate::SEND` and `Approximate::RECEIVE` events for active devices. An optional parameter `inclusive` defines whether all devices on the network should initially be included (`true`) or excluded (`false`), by default it is inclusive.

```
void setActiveDeviceHandler(DeviceHandler activeDeviceHandler, bool inclusive = true);
```

Unlike the Proximate Device Handler the Active Device Handler is not filtered by signal strength, but instead can be filtered by [MAC address](https://en.wikipedia.org/wiki/MAC_address) or by device manufacturer with an [OUI code](https://en.wikipedia.org/wiki/Organizationally_unique_identifier). To observe a specific device, as with this example, `setActiveDeviceFilter()` takes a MAC address formatted as an String (XX:XX:XX:XX:XX:XX). Similarly, the OUI code of a specific manufacturer can be used as a filter and these can be found [here](http://standards-oui.ieee.org/oui.txt). Alterantively, a list of filters can be maintained using the `addActiveDeviceFilter()` variant. Once a filter is added the initially inclusive or exclusive behaviour is overwritten.

```
void setActiveDeviceFilter(String macAddress);
void setActiveDeviceFilter(int oui);

void addActiveDeviceFilter(String macAddress);
void addActiveDeviceFilter(int oui);
```

The callback function delivers both a pointer to a `Device` and a `Approximate::DeviceEvent` for each event. This example measures the RSSI of messages sent by the device (`event == Approximate::SEND`) to estimate its distance and renders this as a flashing LED that speeds up as the distance decreases.