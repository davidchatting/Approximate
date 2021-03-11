/*
    Channel.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

#ifndef Channel_h
#define Channel_h

#include <Arduino.h>
#include "Network.h"
#include "eth_addr.h"

#define APPROXIMATE_UNKNOWN_RSSI 0

class Channel : public Network {
    private:
        int8_t buf[128];
    public:
        Channel();

        void setBuffer(int8_t *buf);
        int8_t getBufferN(int n);

        void getSubCarrier(int n, float &magnitude, float &phase);
        void getSubCarrier(int n, int8_t &a, int8_t &bi);
};

#endif