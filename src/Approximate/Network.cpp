/*
    Network.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

#include "Network.h"
#include "Approximate.h"

Network::Network() {
}

Network::Network(eth_addr &bssid, int channel) {
    init(bssid, channel);
}

void Network::init(eth_addr &bssid, int channel) {
    setBssid(bssid);
    setChannel(channel);
}

void Network::getBssid(eth_addr &bssid) {
     ETHADDR16_COPY(&bssid, &this -> bssid);
}

String Network::getBssidAsString() {
    String bssidAsString = "";

    Approximate::eth_addr_to_String(bssid, bssidAsString);

    return(bssidAsString);
}

char *Network::getBssidAs_c_str(char *out) {
    Approximate::eth_addr_to_c_str(bssid, out);
    
    return(out);
}

void Network::setBssid(eth_addr &bssid) {
    ETHADDR16_COPY(&this -> bssid, &bssid);
}

int Network::getChannel() {
    return(channel);
}

void Network::setChannel(int channel) {
    this -> channel = channel;
}