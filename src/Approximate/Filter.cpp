/*
    Filter.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "Filter.h"

eth_addr Filter::NONE = eth_addr({{0xff,0xff,0xff,0xff,0xff,0xff}});
eth_addr Filter::ANY = eth_addr({{0x00,0x00,0x00,0x00,0x00,0x00}});

Filter::Filter(eth_addr &macAddress, Direction direction) {
    ETHADDR16_COPY(&this -> macAddress, &macAddress);
    
    this -> direction = direction;
}

bool Filter::matches(eth_addr *macAddress) {
    bool result = true;

    if(eth_addr_cmp(macAddress, &ANY)) {
        result = true;
    }
    else if(eth_addr_cmp(macAddress, &NONE)) {
        result = false;
    }
    else {
        //if an OUI only match on first 3 bytes:
        int N = isOUIFilter() ? 3 : 6;

        for(int n=0; n<N && result; ++n) {
            result = (this -> macAddress.addr[n] == macAddress -> addr[n]);
        }
    }

    return(result);
}

bool Filter::matches(Device *device) {
    bool result = false;

    if(device) {
        eth_addr macAddress;
        device -> getMacAddress(macAddress);

        if(matches(&macAddress)) {
            switch(direction) {
                case EITHER:
                    result = true;  break;     
                case NEITHER:
                    result = false; break;
                case SENDS:
                    result = device -> isUploading();
                    break;
                case RECEIVES:
                    result = device -> isDownloading();
                    break;      
            }
        }
    }

    return(result);
}

bool Filter::isOUIFilter() {
    bool result = 
        this -> macAddress.addr[3] == 0xFF && 
        this -> macAddress.addr[4] == 0xFF &&
        this -> macAddress.addr[5] == 0xFF;

    return(result);
}