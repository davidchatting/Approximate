/*
    Filter.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef Filter_h
#define Filter_h

#include <Arduino.h>
#include "eth_addr.h"

#include "Device.h"

class Filter {
    public:
        typedef enum {
            SENDS,
            RECEIVES,
            EITHER,
            NEITHER
        } Direction;

        eth_addr macAddress;
        Direction direction = Direction::NEITHER;

        static eth_addr NONE;
        static eth_addr ANY;

        Filter(eth_addr &macAddress, Direction direction = Direction::EITHER);
        bool matches(eth_addr *macAddress);
        bool matches(Device *device);
        bool isOUIFilter();
};

#endif