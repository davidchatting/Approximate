/*
    Channel.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

#include "Channel.h"
#include "Approximate.h"

Channel::Channel() {
}

void Channel::setBuffer(int8_t *buf) {
    memcpy(this -> buf, buf, 128 * sizeof(int8_t));
}

int8_t Channel::getBufferN(int n) {
    int8_t result = 0;

    if(n >= 0 && n < 128) result = buf[n];

    return(result);
}

void Channel::getSubCarrier(int n, int8_t &a, int8_t &bi) {
    if(n >= -26 && n <= 26) {
        int index = (n>0) ? (n*2)+2 : 126+(n*2);
        a = buf[index + 1];
        bi = buf[index];
    }
}