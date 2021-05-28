/*
    Example of operating close by Sonoff switches (running in DIY mode) with the Approximate Library
    -
    Find the IP address of close by devices then send a POST to them
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020

    Example documented here > https://github.com/davidchatting/Approximate/tree/master#close-by-sonoff---interacting-with-devices
*/

#include <Approximate.h>
Approximate approx;

#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <HTTPClient.h>
#endif

#include <AceButton.h>
using namespace ace_button;

//Define for your board, not all have built-in LED and/or button:
#if defined(ESP8266)
  const int LED_PIN = 14;
  const int BUTTON_PIN = 15;
  AceButton button(BUTTON_PIN, LOW);
#elif defined(ESP32)
  const int LED_PIN = 2;
  const int BUTTON_PIN = 0;
  AceButton button(BUTTON_PIN);
#endif

Device *closeBySonoff = NULL;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, false);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(onButtonEvent);

  if (approx.init("MyHomeWiFi", "password", true)) {
    approx.setProximateDeviceHandler(onProximateDevice, APPROXIMATE_SOCIAL_RSSI, 10000);
    approx.begin();
  }
}

void loop() {
  approx.loop();
  button.check();
}

void onProximateDevice(Device *device, Approximate::DeviceEvent event) {
  switch (device->getOUI()) {
    //D8F15B Sonoff (Expressif Inc) - see: http://standards-oui.ieee.org/oui.txt
    case 0xD8F15B:
      onCloseBySonoff(device, event);
      break;
    default:
      Serial.printf("Unknown OUI:\t0x%06X\n", device->getOUI());
      break;
  }
}

void onCloseBySonoff(Device *device, Approximate::DeviceEvent event) {
  switch (event) {
    case Approximate::ARRIVE:
      closeBySonoff = device;
      digitalWrite(LED_PIN, HIGH);
      Serial.printf("SONOFF\t%s\t(%s)\tARRIVE\n", device -> getMacAddressAsString().c_str(), device -> getIPAddressAsString().c_str());
      break;
    case Approximate::DEPART:
      if(*device == *closeBySonoff) {
        closeBySonoff = NULL;
        digitalWrite(LED_PIN, LOW);
        Serial.printf("SONOFF\t%s\tDEPART\n", device -> getMacAddressAsString().c_str());
      }
      break;
  }
}

void onButtonEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if(closeBySonoff) {  
    switch (eventType) {
      case AceButton::kEventPressed:
        Serial.println("AceButton::kEventPressed");
        switchCloseBySonoff(true);
        break;
      case AceButton::kEventReleased:
        Serial.println("AceButton::kEventReleased");
        switchCloseBySonoff(false);
        break;
    }
  }
}

void switchCloseBySonoff(bool switchState) {
  if(closeBySonoff) {
    approx.onceWifiStatus(WL_CONNECTED, [](bool switchState) {
      if(closeBySonoff) {
        WiFiClient client;
        HTTPClient http;
        String url = "http://" + closeBySonoff->getIPAddressAsString() + ":8081/zeroconf/switch";
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
      
        String switchValue = switchState?"on":"off";
        String httpRequestData = "{\"deviceid\": \"\",\"data\": {\"switch\": \"" + switchValue + "\"}}";
        
        int httpResponseCode = http.POST(httpRequestData);
        Serial.printf("%s\t%s\t%i\n",url.c_str(), httpRequestData.c_str(), httpResponseCode);
        http.end();
      }
      
      #if defined(ESP8266)
        delay(20);
        approx.disconnectWiFi();
      #endif
    }, switchState);
    approx.connectWiFi();
  }
}