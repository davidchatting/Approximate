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

Channel::Channel(eth_addr &bssid, int channel) {
    init(bssid, channel);
}

void Channel::init(eth_addr &bssid, int channel) {
    setBssid(bssid);
    setChannel(channel);
}

void Channel::getBssid(eth_addr &bssid) {
     ETHADDR16_COPY(&bssid, &this -> bssid);
}

String Channel::getBssidAsString() {
    String bssidAsString = "";

    Approximate::eth_addr_to_String(bssid, bssidAsString);

    return(bssidAsString);
}

char *Channel::getBssidAs_c_str(char *out) {
    Approximate::eth_addr_to_c_str(bssid, out);
    
    return(out);
}

void Channel::setBssid(eth_addr &bssid) {
    ETHADDR16_COPY(&this -> bssid, &bssid);
}

int Channel::getChannel() {
    return(channel);
}

void Channel::setChannel(int channel) {
    this -> channel = channel;
}