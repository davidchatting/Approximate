/*
    Close By example with MQTT for the Approximate Library
    -
    Find the MAC address of close by devices then send an MQTT report
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include <Approximate.h>
#include <PubSubClient.h>        //https://github.com/knolleary/pubsubclient

Approximate approx;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);

  if (approx.init("MyHomeWiFi", "password")) {
    approx.setProximateDeviceHandler(onCloseByDevice);
    approx.start([]() {
      mqttClient.setServer("192.168.XXX.XXX", 1883);
    });
  }
}

void loop() {
  approx.loop();
  mqttClient.loop();
}

void onCloseByDevice(Device *device, Approximate::DeviceEvent event) {
  if(event == Approximate::ARRIVE || event == Approximate::DEPART) {
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