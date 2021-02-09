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