/*
    ArpTable.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "ArpTable.h"

bool ArpTable::running = false;

ip4_addr_t ArpTable::localNetwork;
uint32_t *ArpTable::cache = new uint32_t[256];

#if defined(ESP8266)
    const int ArpTable::minUpdateIntervalMs = 300;  //updating more frequently is unsafe
#elif defined(ESP32)
    const int ArpTable::minUpdateIntervalMs = 50;   //updating more frequently is unsafe
#endif

ArpTable::ArpTable(int updateIntervalMs, bool repeatedScans) {
    updateIntervalMs = max(updateIntervalMs, minUpdateIntervalMs);

    this -> updateIntervalMs = updateIntervalMs;
    this -> lastUpdateTimeMs = -updateIntervalMs;

    this -> repeatedScans = repeatedScans;
}

ArpTable* ArpTable::getInstance(int updateIntervalMs, bool repeatedScans) {
    static ArpTable at(updateIntervalMs, repeatedScans);
    return &at;
}

void ArpTable::begin() {
    running = true;
}

void ArpTable::end() {
    running = false;
}

void ArpTable::loop() {
    if(running && WiFi.status() == WL_CONNECTED && millis() > (lastUpdateTimeMs + updateIntervalMs)) {
        lastUpdateTimeMs = millis();
        find(scannedDevice, true);
    
        if((scannedDevice == 255) && !repeatedScans) end();
        else {
            scannedDevice = (scannedDevice + 1) % 256;
        }
    }
}

bool ArpTable::isRunning() {
  return(running);
}

void ArpTable::scan() {
    if(WiFi.status() == WL_CONNECTED) {
        Serial.printf("Building ARP table, takes %i seconds...\t", (minUpdateIntervalMs * 256)/1000);
        IP4_ADDR(&localNetwork, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], 0);

        //initate a full (blocking) scan:
        for(int n=0; n<256; ++n) {
            find(n, true);
            delay(minUpdateIntervalMs);
        }

        //write results to the cache:
        for(int n=0; n<256; ++n)    find(n, false);

        Serial.printf("DONE\n");
    }
}

bool ArpTable::find(int localDevice, bool requestIfNotFound) {
    ip4_addr_t ipaddr;
    ipaddr.addr = (localNetwork.addr & 0xFFFFFF) | (localDevice << 24);

    return(find(ipaddr, requestIfNotFound));
}

bool ArpTable::find(ip4_addr_t &ipaddr, bool requestIfNotFound) {
    bool found = false;

    struct eth_addr *eth_ret;
    const ip4_addr_t *ip_ret;
    if(etharp_find_addr(netif_default, &ipaddr, &eth_ret, &ip_ret) == -1) {
        //unknown
        if(requestIfNotFound && WiFi.status() == WL_CONNECTED) {
            etharp_request(netif_default, &ipaddr); //if found, ARP table will be updated shortly afterwards
        }
    }
    else {
        //known - already in ARP table - add to cache
        cache[(ipaddr.addr >> 24) & 0xFF] = getHash(*eth_ret);
        found = true;
    }

    return(found);
}

bool ArpTable::lookupIPAddress(Device *device) {
    bool success = false;

    if(device) {
        eth_addr macAddress;
        device -> getMacAddress(macAddress);

        ip4_addr_t ipAddress;
        if(lookupIPAddress(macAddress, ipAddress)) {
            device -> setIPAddress(ipAddress);
            success = true;
        }
    }
    
    return(success);
}

bool ArpTable::lookupIPAddress(eth_addr &macAddress, ip4_addr_t &ipaddr) {
    bool found = false;

    uint32_t hash = getHash(macAddress);
    for(int n=0; n<256 && !found; ++n) {
        if(cache[n] == hash) {
            ipaddr.addr = (localNetwork.addr & 0xFFFFFF) | (n << 24);
            found = true;
        }
    }

    if(!found) {
        ip4_addr_t ipaddrProbe;
        struct eth_addr *eth_ret;
        const ip4_addr_t *ip_ret;
        for(int n=0; n<256 && !found; ++n) {
            ipaddrProbe.addr = (localNetwork.addr & 0xFFFFFF) | (n << 24);

            if(etharp_find_addr(netif_default, &ipaddrProbe, &eth_ret, &ip_ret) >= 0) {
                if(eth_addr_cmp(&macAddress, eth_ret)) {
                    ip4_addr_copy(ipaddr, ipaddrProbe);
                    found = true;
                }
            }
        }
    }

    return(found);
}

uint32_t ArpTable::getHash(eth_addr &macAddress) {
    uint32_t hash = 0;

    //last 4 bytes:
    hash = (macAddress.addr[2] << 24) | (macAddress.addr[3] << 16) | (macAddress.addr[4] << 8) | (macAddress.addr[5] & 0xFF);

    return(hash);
}