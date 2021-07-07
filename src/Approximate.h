/*
    Approximate.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef Approximate_h
#define Approximate_h

#include <Arduino.h>
#include "Approximate/eth_addr.h"
#include "Approximate/wifi_pkt.h"

#include "Approximate/ArpTable.h"
#include "Approximate/Channel.h"
#include "Approximate/Device.h"
#include "Approximate/Filter.h"
#include "Approximate/Network.h"
#include "Approximate/Packet.h"
#include "Approximate/PacketSniffer.h"

#include <ListLib.h>              //https://github.com/luisllamasbinaburo/Arduino-List

#define APPROXIMATE_INTIMATE_RSSI -20
#define APPROXIMATE_PERSONAL_RSSI -40
#define APPROXIMATE_SOCIAL_RSSI -60
#define APPROXIMATE_PUBLIC_RSSI -80

class Approximate {
  public:
    typedef enum {
      PKT_MGMT,
      PKT_CTRL,
      PKT_DATA,
      PKT_MISC,
    } PacketType;

    typedef enum {
      ARRIVE,
      DEPART,
      SEND,
      RECEIVE,
      INACTIVE
    } DeviceEvent;

    typedef void (*DeviceHandler)(Device *device, DeviceEvent event);
    typedef void (*ChannelStateHandler)(Channel *channel);

    static String toString(DeviceEvent e) {
      switch (e) {
        case Approximate::SEND:       return("SEND");
        case Approximate::RECEIVE:    return("RECEIVE");
        case Approximate::ARRIVE:     return("ARRIVE");
        case Approximate::DEPART:     return("DEPART");
        default:                      return("INACTIVE");
      }
    }

  private:
    static bool running;
    static bool onlyIndividualDevices;

    static PacketSniffer *packetSniffer;
    static ArpTable *arpTable;

    char *ssid = new char[32];
    char *password = new char[64];

    wl_status_t currentWifiStatus = WL_IDLE_STATUS;
    bool initBlind(int channel, uint8_t *bssid, bool ipAddressResolution, bool csiEnabled, bool onlyIndividualDevices);
    bool initBlind(bool ipAddressResolution, bool csiEnabled, bool onlyIndividualDevices);
    void onWifiStatusChange(wl_status_t oldStatus, wl_status_t newStatus);

    //TODO: template this?
    typedef void (*voidFnPtr)();
    typedef void (*voidFnPtrWithStringPayload)(String);
    typedef void (*voidFnPtrWithBoolPayload)(bool);
    typedef void (*voidFnPtrWithFnPtrPayload)(voidFnPtr);    
    voidFnPtr onceWifiStatusFnPtr = NULL;
    voidFnPtrWithStringPayload onceWifiStatusWithStringPayloadFnPtr = NULL;
    voidFnPtrWithBoolPayload onceWifiStatusWithBoolPayloadFnPtr = NULL;
    voidFnPtrWithFnPtrPayload onceWifiStatusWithFnPtrPayloadFnPtr = NULL;
    String onceWifiStatusStringPayload;
    bool onceWifiStatusBoolPayload;
    voidFnPtr onceWifiStatusFnPtrPayload;
    wl_status_t triggerWifiStatus = WL_IDLE_STATUS;

    static void parsePacket(wifi_promiscuous_pkt_t *pkt, uint16_t len, int type);
    static void parseMgmtPacket(wifi_promiscuous_pkt_t *pkt);
    static void parseCtrlPacket(wifi_promiscuous_pkt_t *pkt);
    static void parseDataPacket(wifi_promiscuous_pkt_t *pkt, uint16_t payloadLength);
    static void parseMiscPacket(wifi_promiscuous_pkt_t *pkt);

    static void parseChannelStateInformation(wifi_csi_info_t *info);

    static DeviceHandler activeDeviceHandler;
    static DeviceHandler proximateDeviceHandler;
    static ChannelStateHandler channelStateHandler;

    void updateProximateDeviceList();

    static eth_addr ownMacAddress;

    static eth_addr localBSSID;
    static List<Filter *> activeDeviceFilterList;
    static bool applyDeviceFilters(Device *device);

    static List<Device *> proximateDeviceList;
    static Device *getProximateDevice(Device *device);
    static Device *getProximateDevice(eth_addr &macAddress);
    static int proximateRSSIThreshold;
    static int proximateLastSeenTimeoutMs;

    void printWiFiStatus();

    static bool wifi_promiscuous_pkt_to_Device(wifi_promiscuous_pkt_t *pkt, uint16_t payloadLengthBytes, Device *device);
    static bool wifi_promiscuous_pkt_to_Packet(wifi_promiscuous_pkt_t *in, uint16_t payloadLengthBytes, Packet *out);
    static bool Packet_to_Device(Packet *packet, eth_addr &bssid, Device *device);

    static bool wifi_csi_info_to_Channel(wifi_csi_info_t *info, Channel *channel);

  public:
    Approximate();
    bool init(String ssid, String password, bool ipAddressResolution = false, bool csiEnabled = false, bool onlyIndividualDevices = true);

    void begin(voidFnPtr thenFnPtr = NULL);
    void end();

    void loop();
    bool isRunning();

    //add one more filter
    void addActiveDeviceFilter(String macAddress);
    void addActiveDeviceFilter(char *macAddress);
    void addActiveDeviceFilter(Device &device);
    void addActiveDeviceFilter(Device *device);
    void addActiveDeviceFilter(eth_addr &macAddress);
    void addActiveDeviceFilter(int oui);

    //set exactly one filter
    void setActiveDeviceFilter(String macAddress);
    void setActiveDeviceFilter(char *macAddress);
    void setActiveDeviceFilter(Device &device);
    void setActiveDeviceFilter(Device *device);
    void setActiveDeviceFilter(eth_addr &macAddress);
    void setActiveDeviceFilter(int oui);

    void removeActiveDeviceFilter(String macAddress);
    void removeActiveDeviceFilter(Device &device);
    void removeActiveDeviceFilter(Device *device);
    void removeActiveDeviceFilter(eth_addr &macAddress);
    void removeActiveDeviceFilter(int oui);
    void removeAllActiveDeviceFilters();

    void setLocalBSSID(String macAddress);
    void setLocalBSSID(eth_addr &macAddress);

    static bool isProximateDevice(Device *device);
    static bool isProximateDevice(String macAddress);
    static bool isProximateDevice(eth_addr &macAddress);

    void setActiveDeviceHandler(DeviceHandler activeDeviceHandler, bool inclusive = true);
    void setProximateDeviceHandler(DeviceHandler deviceHandler, int rssiThreshold = APPROXIMATE_PERSONAL_RSSI, int lastSeenTimeoutMs = 60000);
    void setChannelStateHandler(ChannelStateHandler channelStateHandler);

    static void setProximateRSSIThreshold(int proximateRSSIThreshold);
    static void setProximateLastSeenTimeoutMs(int proximateLastSeenTimeoutMs);

    wl_status_t connectWiFi(String ssid, String password);
    wl_status_t connectWiFi(char *ssid, char *password);
    wl_status_t connectWiFi();
    void disconnectWiFi();

    void onceWifiStatus(wl_status_t status, voidFnPtr callBackFnPtr);
    void onceWifiStatus(wl_status_t status, voidFnPtrWithStringPayload callBackFnPtr, String payload);
    void onceWifiStatus(wl_status_t status, voidFnPtrWithBoolPayload callBackFnPtr, bool payload);
    void onceWifiStatus(wl_status_t status, voidFnPtrWithFnPtrPayload callBackFnPtr, voidFnPtr payload);

    static bool MacAddr_to_eth_addr(MacAddr *in, eth_addr &out);
    static bool uint8_t_to_eth_addr(uint8_t *in, eth_addr &out);
    static bool oui_to_eth_addr(int oui, eth_addr &out);
    static bool c_str_to_eth_addr(const char *in, eth_addr &out);
    static bool String_to_eth_addr(String &in, eth_addr &out);
    static bool eth_addr_to_String(eth_addr &in, String &out);
    static bool eth_addr_to_c_str(eth_addr &in, char *out);
    static bool MacAddr_to_c_str(MacAddr *in, char *out);
    static bool MacAddr_to_oui(MacAddr *in, int &out);
    static bool MacAddr_to_MacAddr(MacAddr *in, MacAddr &out);
};

#endif