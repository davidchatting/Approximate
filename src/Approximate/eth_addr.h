/*
    eth_addr.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef eth_addr_h
#define eth_addr_h

#if defined(ESP8266)
    #include "netif/etharp.h"

#elif defined(ESP32)
    #include "lwip/etharp.h"
    
#endif

#ifndef ETHADDR16_COPY
  #define ETHADDR16_COPY(src, dst)  SMEMCPY(src, dst, ETHARP_HWADDR_LEN)
#endif

struct __attribute__((packed)) MacAddr {
  uint8_t mac[6];

  bool operator==(const MacAddr& rhs) {
    bool result = true;

    for(int n=0; n < 6 && result; ++n) result = (mac[n] == rhs.mac[n]);

    return result;
  }
};

#endif