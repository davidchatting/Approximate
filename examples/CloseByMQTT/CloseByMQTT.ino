/*
    Close By example with MQTT for the Approximate Library
    -
    Find the MAC address of close by devices then send an MQTT report
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#close-by-mqtt---make-a-connection-to-the-cloud
*/

#include <Approximate.h>
#include <PubSubClient.h>        //https://github.com/knolleary/pubsubclient

Approximate approx;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//Define for your board, not all have built-in LED and/or button:
#if defined(ESP8266)
  const int LED_PIN = 14;
#elif defined(ESP32)
  const int LED_PIN = 2;
#endif

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setProximateDeviceHandler(onProximateDevice);
    approx.begin([]() {
      mqttClient.setServer("192.168.XXX.XXX", 1883);
    });
  }
}

void loop() {
  approx.loop();
  mqttClient.loop();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  if(event == Approximate::ARRIVE || event == Approximate::DEPART) {
    digitalWrite(LED_PIN, event == Approximate::ARRIVE);

    String json = "{\"" + device->getMacAddressAsString() + "\":\"" + Approximate::toString(event) + "\"}";
    Serial.println(json);
    
    approx.onceWifiStatus(WL_CONNECTED, [](String payload) {
      mqttClient.connect(WiFi.macAddress().c_str());
      mqttClient.publish("closeby", payload.c_str(), false); //false = don't retain message
      
      #if defined(ESP8266)
        delay(20);
        approx.disconnectWiFi();
      #endif
    }, json);
    approx.connectWiFi();
  }
}