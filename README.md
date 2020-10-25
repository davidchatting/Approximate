# The Approximate Library
The Approximate library is a WiFi [Arduino](http://www.arduino.cc/download) Library for building proximate interactions with the [ESP8266](https://en.wikipedia.org/wiki/ESP8266) and [ESP32](https://en.wikipedia.org/wiki/ESP32) and your Internet of Things.

Approximate works with 2.4GHz WiFi networks, but not 5GHz networks, as neither ESP8266 or ESP32 support this technology.

# Examples
Approximate can interact with devices in proximity (using a Proximate Device Handler) or simply when they are active (using a Active Device Handler). The examples [here](examples) demonstrate combinations of these.

Every Approximate sketch requires this essential structure:

```
#include <Approximate.h>
Approximate approx;

void setup() {
    if (approx.init("MyHomeWiFi", "password")) {
        approx.begin();
    }
}

void loop() {
    approx.loop();
}
```

### When We're Close... using a Proximate Device Handler
The [CloseBy example](examples/CloseBy) identifies WiFi devices in proximity and prints out their [MAC addresses](https://en.wikipedia.org/wiki/MAC_address).

```
#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password")) {
        approx.setProximateDeviceHandler(onCloseByDevice, APPROXIMATE_PERSONAL_RSSI);
        approx.begin();
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

The Proximate Device Handler is set by `setProximateDeviceHandler()`, it takes a `DeviceHandler` callback function parameter (here `onCloseByDevice`) and a value for the `rssiThreshold` parameter that describes range that we consider to be in proximity (here `APPROXIMATE_PERSONAL_RSSI`). [RSSI](https://en.wikipedia.org/wiki/Received_signal_strength_indication) is a measure of WiFi signal strength used here to estimate proximity. It is measured in [dBm](https://en.wikipedia.org/wiki/DBm) and at close proximity (where the reception is good) its value will approach zero, as the signal degrades over distance and through objects and walls, the value will fall - for instance an RSSI of -50 represents a relatively strong signal. The library predefined four values of `rssiThreshold` for use, that borrow from the language of [proxemics](https://en.wikipedia.org/wiki/Proxemics):

* `APPROXIMATE_INTIMATE_RSSI` -20 dBm (10 centimetres)
* `APPROXIMATE_PERSONAL_RSSI` -40 dBm (1 metre)
* `APPROXIMATE_SOCIAL_RSSI`   -60 dBm (4 metres)
* `APPROXIMATE_PUBLIC_RSSI`   -80 dBm (8 metres)

These values are extremely approximate and represent the highest values that might be achieved at these ranges. `rssiThreshold` can be defined numerically and if it is not set `setProximateDeviceHandler()` defaults to a value of `APPROXIMATE_PERSONAL_RSSI`.

The callback function `onCloseByDevice()` receives both a pointer to a `Device` and a `Approximate::DeviceEvent` for each new observation - here the events `Approximate::ARRIVE` and `Approximate::DEPART` cause the device's [MAC address](https://en.wikipedia.org/wiki/MAC_address) to be printed out. MAC addresses are the primary way in which the Approximate library identifies network devices.

There are four event types that a `DeviceHandler` will encounter: 

* `Approximate::ARRIVE` once when the device first arrives in proximity (only for Proximate Device Handlers)
* `Approximate::DEPART` once when the device departs and is no longer seen in proximity (only for Proximate Device Handlers)
* `Approximate::SEND` every time the device sends (uploads) data
* `Approximate::RECEIVE` every time the device receives (downloads) data (rarely for Proximate Device Handlers, unless the router is also in proximity)

The full definition for `setProximateDeviceHandler()` is:

```
void setProximateDeviceHandler(DeviceHandler deviceHandler, int rssiThreshold = APPROXIMATE_PERSONAL_RSSI, int lastSeenTimeoutMs = 60000);
```

The parameter `lastSeenTimeoutMs` defines how quickly (in milliseconds) a device will be said to `DEPART` if it is unseen. While the `ARRIVE` event is triggered only once for a device, further observations will cause `SEND` and (sometimes) `RECEIVE` events; when these events stop and after a wait of `lastSeenTimeoutMs`, a `DEPART` event will then be generated. A suitable value will depend on the dynamics of the application and devices' use of the network. One minute (60,000 ms) is the default value.

### Find My...  using an Active Device Handler
The [FindMy example](examples/FindMy) demonstrates tracking down a device on your WiFi network using its signal strength (as measured by [RSSI](https://en.wikipedia.org/wiki/Received_signal_strength_indication)) - the LED flashes faster, the closer it is.

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
    approx.begin();
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

The callback function delivers both a pointer to a `Device` and a `Approximate::DeviceEvent` for each event. This example measures the RSSI of messages sent by the device (`event == Approximate::SEND`) to estimate its distance and renders this as a flashing LED, that speeds up as the distance decreases.

### Close By MQTT
The [CloseByMQTT example](examples/CloseByMQTT)...

### Close By Sonoff
The [CloseBySonoff example](examples/CloseBySonoff)...

### Watch Device
The [WatchDevice example](examples/WatchDevice)...

## Installation

<del>The latest stable release of Approximate is available in the Arduino IDE Library Manager. Search for "Approximate". Click install.</del>

Alternatively, Approximate can be installed by cloning the GitHub repository (https://github.com/davidchatting/Approximate), then manually copying over the contents to the `./libraries` directory used by the Arduino IDE - into a folder called `./libraries/Approximate`. The `master` branch contains the tagged stable releases, `develop` contains the current development version - pull requests and contributions are welcome.

External Dependencies... Approximate supports the ESP8266 and the ESP32, depending on which is used...