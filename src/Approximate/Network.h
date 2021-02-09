/*
    Network.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) February 2021
*/

#ifndef Network_h
#define Network_h

#include <Arduino.h>
#include "eth_addr.h"

class Network {
    protected:
        eth_addr bssid = {{0,0,0,0,0,0}};
        int channel = -1;

    public:
        Network();
        Network(eth_addr &bssid, int channel);
        void init(eth_addr &bssid, int channel);

        void getBssid(eth_addr &bssid);
        String getBssidAsString();
        char *getBssidAs_c_str(char *out);
        void setBssid(eth_addr &bssid);

        int getChannel();
        void setChannel(int channel);
};

#endif