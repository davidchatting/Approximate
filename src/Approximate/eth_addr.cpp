/*
    eth_addr.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
    Updated 2026
*/

#include "eth_addr.h"

bool MacAddr_to_eth_addr(MacAddr *in, eth_addr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.addr[n] = in->mac[n];

  return(success);
}

bool uint8_t_to_eth_addr(uint8_t *in, eth_addr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.addr[n] = in[n];

  return(success);
}

bool oui_to_eth_addr(int oui, eth_addr &out) {
  bool success = true;

  out.addr[0] = (oui >> 16) & 0xFF;
  out.addr[1] = (oui >> 8) & 0xFF;
  out.addr[2] = (oui >> 0) & 0xFF;
  out.addr[3] = 0xFF;
  out.addr[4] = 0xFF;
  out.addr[5] = 0xFF;

  return(success);
}

bool String_to_eth_addr(String &in, eth_addr &out) {
  bool success = c_str_to_eth_addr(in.c_str(), out);

  return(success);
}

bool c_str_to_eth_addr(const char *in, eth_addr &out) {
  bool success = false;

  //clear:
  for(int n=0; n<6; ++n) out.addr[n] = 0;

  //basic format test ##:##:##:##:##:##
  if(strlen(in) == 17) {
    int a, b, c, d, e, f;
    sscanf(in, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);

    out.addr[0] = a;
    out.addr[1] = b;
    out.addr[2] = c;
    out.addr[3] = d;
    out.addr[4] = e;
    out.addr[5] = f;

    success = true;
  }

  return(success);
}

bool c_str_to_MacAddr(const char *in, MacAddr &out) {
  bool success = false;

  //clear:
  for(int n=0; n<6; ++n) out.mac[n] = 0;

  //basic format test ##:##:##:##:##:##
  if(strlen(in) == 17) {
    int a, b, c, d, e, f;
    sscanf(in, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);

    out.mac[0] = a;
    out.mac[1] = b;
    out.mac[2] = c;
    out.mac[3] = d;
    out.mac[4] = e;
    out.mac[5] = f;

    success = true;
  }

  return(success);
}

bool eth_addr_to_String(eth_addr &in, String &out) {
  bool success = true;

  char macAddressAsCharArray[18];
  eth_addr_to_c_str(in, macAddressAsCharArray);
  out = String(macAddressAsCharArray);

  return(success);
}

bool eth_addr_to_c_str(eth_addr &in, char *out) {
  bool success = true;

  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X\0", in.addr[0], in.addr[1], in.addr[2], in.addr[3], in.addr[4], in.addr[5]);

  return(success);
}

bool MacAddr_to_c_str(MacAddr *in, char *out) {
  bool success = true;

  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X\0", in->mac[0], in->mac[1], in->mac[2], in->mac[3], in->mac[4], in->mac[5]);

  return(success);
}

bool MacAddr_to_oui(MacAddr *in, int &out) {
  bool success = true;

  out = ((in->mac[0] << 16) & 0xFF0000) | ((in->mac[1] << 8) & 0xFF00) | ((in->mac[2] << 0) & 0xFF);

  return(success);
}

bool MacAddr_to_MacAddr(MacAddr *in, MacAddr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.mac[n] = in -> mac[n];

  return(success);
}
