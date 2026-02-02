/*
    wifi_pkt.h
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
    Updated 2026
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

  typedef struct {
      signed rssi: 8;             // signal intensity of packet
      unsigned rate: 4;
      unsigned is_group: 1;
      unsigned: 1;
      unsigned sig_mode: 2;       // 0: non-HT(11bg) packet; 1: HT(11n) packet; 3: VHT(11ac) packet
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
      u8 payload[0]; // ieee80211 payload
      //u16 cnt;        // number count of packet
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

// ---- IEEE 802.11 Management Frame Subtypes (Type 0) ----
typedef enum {
  ASSOCIATION_REQ     = 0,
  ASSOCIATION_RES     = 1,
  REASSOCIATION_REQ   = 2,
  REASSOCIATION_RES   = 3,
  PROBE_REQ           = 4,
  PROBE_RES           = 5,
  TIMING_ADV          = 6,   // Timing Advertisement
  NU1                 = 7,   // Reserved
  BEACON              = 8,
  ATIM                = 9,
  DISASSOCIATION      = 10,
  AUTHENTICATION      = 11,
  DEAUTHENTICATION    = 12,
  ACTION              = 13,
  ACTION_NACK         = 14,
} wifi_mgmt_subtypes_t;

// ---- IEEE 802.11 Control Frame Subtypes (Type 1) ----
typedef enum {
  CTRL_BEAMFORMING    = 4,   // Beamforming Report Poll
  CTRL_VHT_NDP        = 5,   // VHT NDP Announcement
  CTRL_CTRL_EXT       = 6,   // Control Frame Extension
  CTRL_WRAPPER        = 7,   // Control Wrapper
  CTRL_BLOCK_ACK_REQ  = 8,   // Block Ack Request (BAR)
  CTRL_BLOCK_ACK      = 9,   // Block Ack (BA)
  CTRL_PS_POLL        = 10,  // PS-Poll
  CTRL_RTS            = 11,  // Request to Send
  CTRL_CTS            = 12,  // Clear to Send
  CTRL_ACK            = 13,  // Acknowledgement
  CTRL_CF_END         = 14,  // CF-End
  CTRL_CF_END_ACK     = 15,  // CF-End + CF-Ack
} wifi_ctrl_subtypes_t;

// ---- IEEE 802.11 Data Frame Subtypes (Type 2) ----
typedef enum {
  DATA_DATA           = 0,
  DATA_CF_ACK         = 1,
  DATA_CF_POLL        = 2,
  DATA_CF_ACK_POLL    = 3,
  DATA_NULL           = 4,   // Null (no data)
  DATA_CF_ACK_NODATA  = 5,
  DATA_CF_POLL_NODATA = 6,
  DATA_CF_ACK_POLL_NODATA = 7,
  DATA_QOS            = 8,   // QoS Data
  DATA_QOS_CF_ACK     = 9,
  DATA_QOS_CF_POLL    = 10,
  DATA_QOS_CF_ACK_POLL = 11,
  DATA_QOS_NULL       = 12,  // QoS Null (no data)
  DATA_QOS_RESERVED   = 13,
  DATA_QOS_CF_POLL_NODATA = 14,
  DATA_QOS_CF_ACK_POLL_NODATA = 15,
} wifi_data_subtypes_t;

// ---- IEEE 802.11 ToDS/FromDS Direction Values ----
// ds field interpretation for address mapping:
//   ds=0: IBSS (ad-hoc) - Addr1=DA, Addr2=SA, Addr3=BSSID
//   ds=1: To AP         - Addr1=BSSID, Addr2=SA, Addr3=DA
//   ds=2: From AP       - Addr1=DA, Addr2=BSSID, Addr3=SA
//   ds=3: WDS bridge    - Addr1=RA, Addr2=TA, Addr3=DA, Addr4=SA
#define DS_IBSS     0  // ToDS=0, FromDS=0
#define DS_TO_AP    1  // ToDS=1, FromDS=0
#define DS_FROM_AP  2  // ToDS=0, FromDS=1
#define DS_WDS      3  // ToDS=1, FromDS=1

// ---- Frame Control Field ----
typedef struct {
  unsigned vers:2;
  wifi_promiscuous_pkt_type_t type:2;
  unsigned subtype:4;
  unsigned ds:2;
  unsigned moreFrag:1;
  unsigned retry:1;
  unsigned pwrMgt:1;
  unsigned moreData:1;
  unsigned protect:1;
  unsigned order:1;
} __attribute__((packed)) wifi_80211_fctl;

// ---- Management Frame Header (IEEE 802.11 Section 9.3.3) ----
// Used for all management frames: beacon, probe req/resp, auth, assoc, etc.
// Management frames always have: Addr1=DA, Addr2=SA, Addr3=BSSID
typedef struct {
  wifi_80211_fctl fctl;
  unsigned duration:16;
  MacAddr addr1;  // DA  - Destination Address (often broadcast FF:FF:FF:FF:FF:FF)
  MacAddr addr2;  // SA  - Source Address (transmitter)
  MacAddr addr3;  // BSSID
  int16_t seqctl:16;
  unsigned char payload[];
} __attribute__((packed)) wifi_80211_mgmt_frame;

// ---- Control Frame Headers (IEEE 802.11 Section 9.3.1) ----
// RTS frame: has both receiver and transmitter addresses
typedef struct {
  wifi_80211_fctl fctl;
  unsigned duration:16;
  MacAddr addr1;  // RA - Receiver Address
  MacAddr addr2;  // TA - Transmitter Address
} __attribute__((packed)) wifi_80211_ctrl_rts_frame;

// CTS and ACK frames: have only the receiver address
typedef struct {
  wifi_80211_fctl fctl;
  unsigned duration:16;
  MacAddr addr1;  // RA - Receiver Address
} __attribute__((packed)) wifi_80211_ctrl_ack_frame;

// Block Ack Request / Block Ack: receiver and transmitter addresses
typedef struct {
  wifi_80211_fctl fctl;
  unsigned duration:16;
  MacAddr addr1;  // RA - Receiver Address
  MacAddr addr2;  // TA - Transmitter Address
  uint16_t bar_ctrl;
  uint16_t bar_seq;
} __attribute__((packed)) wifi_80211_ctrl_bar_frame;

// ---- Data Frame Header (IEEE 802.11 Section 9.3.2) ----
// Address field meanings depend on ToDS/FromDS (ds field), see DS_* constants above.
typedef struct {
  wifi_80211_fctl fctl;
  unsigned duration:16;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl:16;
  unsigned char payload[];
} __attribute__((packed)) wifi_80211_data_frame;

// ---- Information Element (IE) for management frame bodies ----
typedef struct {
  uint8_t id;
  uint8_t length;
  uint8_t data[];
} __attribute__((packed)) wifi_80211_ie;

// Common IE IDs
#define IE_SSID             0
#define IE_SUPPORTED_RATES  1
#define IE_DS_PARAM_SET     3
#define IE_TIM              5
#define IE_COUNTRY          7
#define IE_RSN              48
#define IE_VENDOR_SPECIFIC  221

// ---- Generic 4-address MAC header ----
typedef struct {
    wifi_80211_fctl frame_ctrl;
    uint8_t addr1[6]; // receiver address
    uint8_t addr2[6]; // sender address
    uint8_t addr3[6]; // filtering address
    unsigned sequence_ctrl:16;
    uint8_t addr4[6]; // optional
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[2]; //  network data ended with 4 bytes csum (CRC32)
} wifi_ieee80211_packet_t;

#endif
