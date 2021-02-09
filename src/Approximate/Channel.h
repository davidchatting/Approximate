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

    public:
        Channel();
};

#endif