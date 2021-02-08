/*
    ArpTable.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef ArpTable_h
#define ArpTable_h

#include <Arduino.h>
#include "eth_addr.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>        //https://github.com/esp8266/Arduino

#elif defined(ESP32)
    #include <WiFi.h>               //https://github.com/espressif/arduino-esp32/

#endif

#include "Device.h"

class ArpTable {
    private:
        static uint32_t *cache;
        static ip4_addr_t localNetwork;

        static bool running;
        bool repeatedScans = true;

        int updateIntervalMs;
        static const int minUpdateIntervalMs;
        int lastUpdateTimeMs;

        ArpTable(int updateIntervalMs = 500, bool repeatedScans = true);
        ArpTable(ArpTable const&);
        void operator=(ArpTable const&);

        static bool find(int localDevice, bool requestIfNotFound);
        static bool find(ip4_addr_t &ipaddr, bool requestIfNotFound);
        int scannedDevice = 0;

        static uint32_t getHash(eth_addr &macAddress);

    public:
        static ArpTable* getInstance(int updateIntervalMs = 1000, bool repeatedScans = true);

        void begin();
        void end();
        void loop();
        bool isRunning();

        static bool lookupIPAddress(Device *device);
        static bool lookupIPAddress(eth_addr &macAddress, ip4_addr_t &ipaddr);

        static void scan();
};

#endif