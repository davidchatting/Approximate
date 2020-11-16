/*
    Packet.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef Packet_h
#define Packet_h

#include "eth_addr.h"

class Packet {
    public:
        eth_addr src = {{0,0,0,0,0,0}};
        eth_addr dst = {{0,0,0,0,0,0}};
        eth_addr bssid = {{0,0,0,0,0,0}};
        int rssi = 0;
        int channel = -1;
        uint16_t payloadLengthBytes = 0;
};

#endif