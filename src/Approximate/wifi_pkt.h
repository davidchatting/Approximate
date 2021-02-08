/*
    wifi_pkt.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#ifndef wifi_pkt_h
#define wifi_pkt_h

#if defined(ESP8266)
  #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

  // Expose Espressif SDK functionality
  extern "C" {
  #include "user_interface.h"
    typedef void (*freedom_outside_cb_t)(uint8 status);
    int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
    void wifi_unregister_send_pkt_freedom_cb(void);
    int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
  }

  typedef enum {
    WIFI_PKT_MGMT,  /**< Management frame, indicates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_CTRL,  /**< Control frame, indicates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_DATA,  /**< Data frame, indiciates 'buf' argument is wifi_promiscuous_pkt_t */
    WIFI_PKT_MISC  /**< Other type, such as MIMO etc. 'buf' argument is wifi_promiscuous_pkt_t but the payload is zero length. */
  } wifi_promiscuous_pkt_type_t;

  typedef enum {
    ASSOCIATION_REQ,
    ASSOCIATION_RES,
    REASSOCIATION_REQ,
    REASSOCIATION_RES,
    PROBE_REQ,
    PROBE_RES,
    NU0,
    NU1,
    BEACON,
    ATIM,
    DISASSOCIATION,
    AUTHENTICATION,
    DEAUTHENTICATION,
    ACTION,
    ACTION_NACK,
  } wifi_mgmt_subtypes_t;

  typedef struct {
      signed rssi: 8;             // signal intensity of packet
      unsigned rate: 4;
      unsigned is_group: 1;
      unsigned: 1;
      unsigned sig_mode: 2;       // 0:is not 11n packet; non-0:is 11n packet;
      unsigned legacy_length: 12; // if not 11n packet, shows length of packet.
      unsigned damatch0: 1;
      unsigned damatch1: 1;
      unsigned bssidmatch0: 1;
      unsigned bssidmatch1: 1;
      unsigned MCS: 7;            // if is 11n packet, shows the modulation
                                  // and code used (range from 0 to 76)
      unsigned CWB: 1;            // if is 11n packet, shows if is HT40 packet or not
      unsigned HT_length: 16;     // if is 11n packet, shows length of packet.
      unsigned Smoothing: 1;
      unsigned Not_Sounding: 1;
      unsigned: 1;
      unsigned Aggregation: 1;
      unsigned STBC: 2;
      unsigned FEC_CODING: 1;     // if is 11n packet, shows if is LDPC packet or not.
      unsigned SGI: 1;
      unsigned rxend_state: 8;
      unsigned ampdu_cnt: 8;
      unsigned channel: 4;        //which channel this packet in.
      unsigned: 12;
  } wifi_pkt_rx_ctrl_t;
  
  typedef struct {
      wifi_pkt_rx_ctrl_t rx_ctrl;
      u8 payload[36]; // head of ieee80211 packet
      u16 cnt;        // number count of packet
  } wifi_promiscuous_pkt_t;

  typedef struct {
  } wifi_csi_info_t;
  
#elif defined(ESP32)
  #define CONFIG_ESP32_WIFI_CSI_ENABLED 1
  #define WIFI_MODE WIFI_APSTA_MODE

  #include <WiFi.h>
  #include <Wire.h>
  #include "esp_wifi.h"
  #include "esp_event.h"
  #include "esp_wifi_types.h"
  
#endif

typedef struct {
  unsigned fctl:16;
  unsigned duration:16;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) wifi_mgmt_hdr;

#endif