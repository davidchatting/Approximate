/*
    PacketSniffer.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
    Updated 2026
*/

#ifndef PacketSniffer_h
#define PacketSniffer_h

#include <Arduino.h>
#include "eth_addr.h"
#include "wifi_pkt.h"
#include "Device.h"
#include "Channel.h"
#include "Packet.h"
#include "ArpTable.h"

class PacketSniffer {
  public:
    static PacketSniffer* getInstance();
    void init(int channel=1, bool isScanning=false);
    bool begin();
    void end();
    void loop();
    bool isRunning();

    uint8_t getCurrentChannel();
    void setCurrentChannel(int channel);

    bool getChannelScan();
    void setChannelScan(bool channelScan);

    typedef bool (*PacketEventHandler)(wifi_promiscuous_pkt_t *packet, uint16_t len, int type, int subtype);
    void setPacketEventHandler(PacketEventHandler packetEventHandler);

    typedef void (*ChannelEventHandler)(wifi_csi_info_t *data);
    void setChannelEventHandler(ChannelEventHandler channelEventHandler);

    // Low-level frame parsing
    static bool parseMgmtFrame(wifi_promiscuous_pkt_t *pkt, uint16_t len, int subtype, Device *device);
    static bool parseCtrlFrame(wifi_promiscuous_pkt_t *pkt, uint16_t len, int subtype, Device *device);
    static bool parseDataFrame(wifi_promiscuous_pkt_t *pkt, uint16_t payloadLengthBytes, Device *device);
    static bool parseCSI(wifi_csi_info_t *info, Channel *channel);

    // Local BSSID management
    static void setLocalBSSID(eth_addr &bssid);

    // Country info parsed from beacons
    static String getCountryCode();
    static char getCountryEnvironment();
    static bool hasCountryInfo();

  private:
    PacketSniffer();
    PacketSniffer(PacketSniffer const&);
    void operator=(PacketSniffer const&);

    static bool running;

    uint8_t currentChannel = -1;
    bool channelScan = false;

    int channelSamplingIntervalMs = 1000;
    int channelSamplingStartedAtMs = 0;
    int highestChannel = 13; //US = 11, EU = 13, Japan = 14

    static void rxCallback_8266(uint8_t *buf, uint16_t len);
    static void rxCallback_32(void* buf, wifi_promiscuous_pkt_type_t type);
    static void rxCallback(wifi_promiscuous_pkt_t *packet, uint16_t len, wifi_promiscuous_pkt_type_t type, int subtype);

    static void csiCallback_32(void *ctx, wifi_csi_info_t *data);

    // Returns pointer to start of 802.11 MAC frame within the packet payload.
    // On ESP8266, AMPDU subframes have a 4-byte delimiter before the MAC header.
    static uint8_t* getFrameStart(wifi_promiscuous_pkt_t *pkt);

    static PacketEventHandler packetEventHandler;
    static ChannelEventHandler channelEventHandler;

    static eth_addr localBSSID;
    static char countryCode[3];
    static char countryEnvironment;
};

#endif