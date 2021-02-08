/*
    PacketSniffer.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef PacketSniffer_h
#define PacketSniffer_h

#include <Arduino.h>
#include "eth_addr.h"
#include "wifi_pkt.h"

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

    typedef void (*PacketEventHandler)(wifi_promiscuous_pkt_t *packet, uint16_t len, int type);
    void setPacketEventHandler(PacketEventHandler packetEventHandler);

    typedef void (*ChannelEventHandler)(wifi_csi_info_t *data);
    void setChannelEventHandler(ChannelEventHandler channelEventHandler);

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
    static void rxCallback(wifi_promiscuous_pkt_t *packet, uint16_t len, wifi_promiscuous_pkt_type_t type);

    static void csiCallback_32(void *ctx, wifi_csi_info_t *data);

    static PacketEventHandler packetEventHandler;
    static ChannelEventHandler channelEventHandler;
};

#endif