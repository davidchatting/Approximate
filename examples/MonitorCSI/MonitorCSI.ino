/*
    Monitor CSI example for the Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

#include <Approximate.h>
Approximate approx;

void setup() {
    Serial.begin(9600);

    if (approx.init("MyHomeWiFi", "password", false, true)) {
        approx.setChannelStateHandler(onChannelStateEvent);
        approx.begin();
    }
}

void loop() {
    approx.loop();
}

void onChannelStateEvent(Channel *channel) {
  int8_t a, bi; 
  for(int n = -26; n <= 26; ++n) {
    if(n!=0) {
      channel -> getSubCarrier(n, a, bi);
      Serial.printf("%i,%i\t", a, bi);
    }
  }
  Serial.printf("\n");
}