/*
    eth_addr.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
    Updated 2026
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

#include <Arduino.h>

// Free functions for MAC address conversion utilities
bool MacAddr_to_eth_addr(MacAddr *in, eth_addr &out);
bool uint8_t_to_eth_addr(uint8_t *in, eth_addr &out);
bool oui_to_eth_addr(int oui, eth_addr &out);
bool c_str_to_eth_addr(const char *in, eth_addr &out);
bool c_str_to_MacAddr(const char *in, MacAddr &out);
bool String_to_eth_addr(String &in, eth_addr &out);
bool eth_addr_to_String(eth_addr &in, String &out);
bool eth_addr_to_c_str(eth_addr &in, char *out);
bool MacAddr_to_c_str(MacAddr *in, char *out);
bool MacAddr_to_oui(MacAddr *in, int &out);
bool MacAddr_to_MacAddr(MacAddr *in, MacAddr &out);

#endif