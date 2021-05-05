/*
    Device.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef Device_h
#define Device_h

#include <Arduino.h>
#include "Network.h"
#include "eth_addr.h"

#define APPROXIMATE_UNKNOWN_RSSI 0

class Device : public Network {
    private:
        eth_addr macAddress = {{0,0,0,0,0,0}};
        ip4_addr_t ipAddress;
        int rssi = APPROXIMATE_UNKNOWN_RSSI;
        long lastSeenAtMs = -1;
        int dataFlowBytes = 0;  //uploading is negative, downloading positive

        long timeOutAtMs = -1;

    public:
        Device();
        Device(Device *b);
        Device(eth_addr &macAddress, eth_addr &bssid, int channel, int rssi = APPROXIMATE_UNKNOWN_RSSI, long lastSeenAtMs = -1, int bytesFlow = 0, u32_t ipAddress = IPADDR_ANY);

        //TODO: tidy-up these operators and matches()
        bool operator ==(Device *b);
        bool operator ==(Device const& b);
        bool operator ==(eth_addr &macAddress);

        void init(eth_addr &macAddress, eth_addr &bssid, int channel, int rssi, long lastSeenAtMs, int bytesFlow, u32_t ipAddress = IPADDR_ANY);
        void update(Device *d);

        void getMacAddress(eth_addr &macAddress);
        String getMacAddressAsString();
        char *getMacAddressAs_c_str(char *out);
        void setMacAddress(eth_addr &macAddress);

        void getIPAddress(ip4_addr_t &ipAddress);
        String getIPAddressAsString();
        char *getIPAddressAs_c_str(char *out);
        void setIPAddress(ip4_addr_t &ipAddress);
        void setIPAddress(u32_t ipAddress);
        bool hasIPAddress();

        void setRSSI(int rssi);
        int getRSSI(bool uploadOnly = true);

        void setLastSeenAtMs(long lastSeenAtMs = -1);
        int getLastSeenAtMs();

        void setTimeOutAtMs(long timeOutAtMs);
        void setReducedTimeOutAtMs(long timeOutAtMs);
        bool hasTimedOut();

        bool matches(eth_addr &macAddress);

        uint32_t getOUI();

        void setDataFlowBytes(int dataFlowBytes);

        bool isUploading();
        bool isDownloading();

        int getUploadSizeBytes();
        int getDownloadSizeBytes();
        int getPayloadSizeBytes();

        bool isUniversal();
        bool isLocal();
        bool isIndividual();
        bool isGroup();
};

#endif