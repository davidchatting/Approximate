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

 void Channel::getSubCarrier(int n, float &magnitude, float &phase) {
    int8_t a = 0, bi = 0;
    getSubCarrier(n, a, bi);

    magnitude = sqrt((a * a) + (bi * bi));
    phase = atan2(bi, a);
 }

void Channel::getSubCarrier(int n, int8_t &a, int8_t &bi) {
    if(n!=0 && n >= -26 && n <= 26) {
        //buffer format: [-,-], [bi, a](1), ... [bi, a](26), [bi, a](-26), ... [bi, a](-1)
        int index = (n>0) ? (n*2)+2 : (128-2)+(n*2)+2;
        a = buf[index + 1];
        bi = buf[index];
    }
}