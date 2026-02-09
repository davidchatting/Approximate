/*
    PacketSniffer.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
    Updated 2026
*/

#include "PacketSniffer.h"

PacketSniffer::PacketEventHandler PacketSniffer::packetEventHandler = NULL;
PacketSniffer::ChannelEventHandler PacketSniffer::channelEventHandler = NULL;
bool PacketSniffer::running = false;

eth_addr PacketSniffer::localBSSID = {{0,0,0,0,0,0}};
char PacketSniffer::countryCode[3] = {0};
char PacketSniffer::countryEnvironment = 0;

PacketSniffer::PacketSniffer() {
  Serial.println("PacketSniffer::PacketSniffer");
}

PacketSniffer* PacketSniffer::getInstance() {
  Serial.println("PacketSniffer::getInstance");
  static PacketSniffer ps;
  return &ps;
}

bool PacketSniffer::begin() {
  if(!running) {
    Serial.println("PacketSniffer::begin");

    #if defined(ESP8266)
      wifi_set_opmode(STATION_MODE);  //promiscuous works only with STATION_MODE
      
      wifi_promiscuous_enable(0);
      wifi_set_promiscuous_rx_cb(rxCallback_8266);
      wifi_promiscuous_enable(1);
      
    #elif defined(ESP32)
      bool CSI_ENABLED = false; 
      #if defined(CONFIG_ESP32_WIFI_CSI_ENABLED)
        CSI_ENABLED = (CONFIG_ESP32_WIFI_CSI_ENABLED == 1) && channelEventHandler;
        if(CSI_ENABLED) {
          //TODO - This shouldn't be necessary - Approximate::connectWiFi() should handles this as esp_wifi_set_csi() needs, but...
          esp_wifi_disconnect();
          delay(1000);
          esp_wifi_stop();
          esp_wifi_deinit();
        }
      #endif

      esp_netif_init();
      esp_event_loop_create_default();
      esp_netif_create_default_wifi_sta();

      wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
      esp_wifi_init(&cfg);

      esp_wifi_set_mode(WIFI_MODE_APSTA);
      esp_wifi_start();

      esp_wifi_set_promiscuous(true);
      esp_wifi_set_promiscuous_rx_cb(&rxCallback_32);

      if(CSI_ENABLED && esp_wifi_set_csi(true) == ESP_OK) {
        //See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv424esp_wifi_set_promiscuousb
        //WiFi must be initialized by esp_wifi_init() + WiFi must be started by esp_wifi_start() + promiscuous mode must be enabled

        wifi_csi_config_t configuration_csi;
        configuration_csi.lltf_en = true;
        configuration_csi.htltf_en = true;
        configuration_csi.stbc_htltf2_en = true;
        configuration_csi.ltf_merge_en = true;
        configuration_csi.channel_filter_en = true;
        configuration_csi.manu_scale = true;
        configuration_csi.shift = 0; // 0->15

        esp_wifi_set_csi_config(&configuration_csi);
        esp_wifi_set_csi_rx_cb(&csiCallback_32, NULL);

        Serial.printf("CSI STARTED\n");
      }

      esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);

    #endif
    
    running = true;
  }

  return(running);
}

void PacketSniffer::end() {
  if(running) {
    Serial.println("PacketSniffer::end");

    #if defined(ESP8266)
      wifi_promiscuous_enable(0);
      
    #elif defined(ESP32)
      esp_wifi_set_promiscuous(false);
      esp_wifi_set_csi(false);
      
    #endif

    running = false;
  }
}

void PacketSniffer::loop() {
  if(running) {
    if(channelScan) {
      long now = millis();
      if((channelSamplingStartedAtMs + channelSamplingIntervalMs) < now) {
        channelSamplingStartedAtMs = now;

        currentChannel += 1;
        if(currentChannel > highestChannel) currentChannel = 1;
        
        setCurrentChannel(currentChannel);
      }
    }

    if(currentChannel != getCurrentChannel()) {
      setCurrentChannel(currentChannel);
    }
  }
}

bool PacketSniffer::isRunning() {
  return(running);
}

void PacketSniffer::init(int channel, bool channelScan) {
  Serial.println("PacketSniffer::init");

  currentChannel = channel;
  setChannelScan(channelScan);
}

void PacketSniffer::setCurrentChannel(int channel) {
  //Serial.printf("PacketSniffer::setCurrentChannel %i\n", channel);

  #if defined(ESP8266)
    if(wifi_set_channel(currentChannel)) {
      currentChannel = channel;
    }
  #elif defined(ESP32)
    if(esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE) == ESP_OK) {
      currentChannel = channel;
    }
  #endif
}

uint8_t PacketSniffer::getCurrentChannel() {
  uint8_t currentChannel = -1;

  #if defined(ESP8266)
    currentChannel = wifi_get_channel();
  #elif defined(ESP32)
    wifi_second_chan_t secondChannel;
    esp_wifi_get_channel(&currentChannel, &secondChannel);
  #endif

  return(currentChannel);
}

bool PacketSniffer::getChannelScan() {
  return(channelScan);
}

void PacketSniffer::setChannelScan(bool channelScan) {
  this->channelScan = channelScan;
}

void PacketSniffer::setPacketEventHandler(PacketEventHandler packetEventHandler) {
  this -> packetEventHandler = packetEventHandler;
}

void PacketSniffer::setChannelEventHandler(ChannelEventHandler channelEventHandler) {
  this -> channelEventHandler = channelEventHandler;
}

uint8_t* PacketSniffer::getFrameStart(wifi_promiscuous_pkt_t *pkt) {
  #if defined(ESP8266)
    // 802.11n AMPDU subframes have a 4-byte delimiter (MPDU length + CRC +
    // signature 0x4E) before the actual MAC header. When the Aggregation bit
    // is set in rx_ctrl, skip the delimiter so frame control and MAC addresses
    // are read from the correct offset.
    if(pkt->rx_ctrl.Aggregation) {
      return pkt->payload + 4;
    }
  #endif
  return pkt->payload;
}

void PacketSniffer::rxCallback_8266(uint8_t *buf, uint16_t len) {
  wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *) buf;
  wifi_80211_data_frame *frame = (wifi_80211_data_frame *) getFrameStart(packet);
  wifi_promiscuous_pkt_type_t type = frame->fctl.type;
  int subtype = frame->fctl.subtype;

  uint16_t sig_len = 0;
  #if defined(ESP8266)
    sig_len = packet->rx_ctrl.sig_mode ? packet->rx_ctrl.HT_length : packet->rx_ctrl.legacy_length;
  #endif

  rxCallback(packet, sig_len, type, subtype);
}

void PacketSniffer::rxCallback_32(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *) buf;
  wifi_80211_data_frame *frame = (wifi_80211_data_frame *) getFrameStart(packet);
  int subtype = frame->fctl.subtype;

  uint16_t sig_len = 0;
  #if defined(ESP32)
    sig_len = packet->rx_ctrl.sig_len;
  #endif

  rxCallback(packet, sig_len, type, subtype);
}

void PacketSniffer::rxCallback(wifi_promiscuous_pkt_t *packet, uint16_t len, wifi_promiscuous_pkt_type_t type, int subtype) {
  if (running && packetEventHandler) {
    packetEventHandler(packet, len, (int) type, subtype);
  }
}

void PacketSniffer::csiCallback_32(void *ctx, wifi_csi_info_t *data) {
  if (running && channelEventHandler) {
    channelEventHandler(data);
  }
}

void PacketSniffer::setLocalBSSID(eth_addr &bssid) {
  ETHADDR16_COPY(&localBSSID, &bssid);
}

String PacketSniffer::getCountryCode() {
  return String(countryCode);
}

char PacketSniffer::getCountryEnvironment() {
  return countryEnvironment;
}

bool PacketSniffer::hasCountryInfo() {
  return (countryCode[0] != 0);
}

bool PacketSniffer::parseMgmtFrame(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t len, int subtype, Device *device) {
  bool success = false;

  if(wifi_pkt && device) {
    wifi_pkt_rx_ctrl_t *rx_ctrl = &(wifi_pkt->rx_ctrl);

    // Management frame header: Addr1=DA, Addr2=SA (transmitter), Addr3=BSSID
    wifi_80211_mgmt_frame *frame = (wifi_80211_mgmt_frame *) getFrameStart(wifi_pkt);

    eth_addr srcAddr;
    MacAddr_to_eth_addr(&(frame->addr2), srcAddr);

    // Skip broadcast/multicast source addresses
    if(srcAddr.addr[0] & 0x01) return false;
    // Skip junk MACs (last 3 bytes all zero)
    if(srcAddr.addr[3] == 0x0 && srcAddr.addr[4] == 0x0 && srcAddr.addr[5] == 0x0) return false;

    eth_addr bssidAddr;
    MacAddr_to_eth_addr(&(frame->addr3), bssidAddr);

    int rssi = rx_ctrl->rssi;
    int channel = rx_ctrl->channel;

    // Calculate the size of the management frame header
    const size_t mgmt_hdr_size = sizeof(wifi_80211_mgmt_frame);

    switch(subtype) {
      case PROBE_REQ:
        // Probe requests are sent by all WiFi devices scanning for networks.
        // The source MAC (addr2) is the device transmitting the probe.
        // Probe requests often have broadcast BSSID (FF:FF:FF:FF:FF:FF).
        device->init(srcAddr, bssidAddr, channel, rssi, millis(), 0);
        ArpTable::lookupIPAddress(device);

        // Parse Information Elements looking for SSID (IE id 0)
        if(len > mgmt_hdr_size) {
          const uint8_t *ie_ptr = frame->payload;
          size_t ie_remaining = len - mgmt_hdr_size;

          while(ie_remaining >= 2) {
            const wifi_80211_ie *ie = (const wifi_80211_ie *) ie_ptr;
            if(ie_remaining < (size_t)(2 + ie->length)) break;

            if(ie->id == IE_SSID && ie->length > 0 && ie->length <= 32) {
              char ssid_buf[33];
              memcpy(ssid_buf, ie->data, ie->length);
              ssid_buf[ie->length] = '\0';
              device->setSSID(ssid_buf);
            }

            ie_ptr += 2 + ie->length;
            ie_remaining -= 2 + ie->length;
          }
        }

        success = true;
        break;

      case PROBE_RES:
      case BEACON:
        // Probe responses and beacons are sent by APs.
        // Addr2=SA is the AP's MAC, Addr3=BSSID is the network BSSID.
        // Only process if from the local network's BSSID.
        if(eth_addr_cmp(&bssidAddr, &localBSSID)) {
          device->init(srcAddr, bssidAddr, channel, rssi, millis(), 0);

          // Parse Country IE from beacon/probe response body.
          // Frame body starts after mgmt header with 12 bytes of fixed fields:
          //   8-byte timestamp + 2-byte beacon interval + 2-byte capability info
          const size_t fixedFieldsSize = 12;
          if(len > mgmt_hdr_size + fixedFieldsSize) {
            const uint8_t *ie_ptr = frame->payload + fixedFieldsSize;
            size_t ie_remaining = len - mgmt_hdr_size - fixedFieldsSize;

            while(ie_remaining >= 2) {
              const wifi_80211_ie *ie = (const wifi_80211_ie *) ie_ptr;
              if(ie_remaining < (size_t)(2 + ie->length)) break;

              if(ie->id == IE_COUNTRY && ie->length >= 3) {
                countryCode[0] = (char) ie->data[0];
                countryCode[1] = (char) ie->data[1];
                countryCode[2] = '\0';
                countryEnvironment = (char) ie->data[2];
              }

              ie_ptr += 2 + ie->length;
              ie_remaining -= 2 + ie->length;
            }
          }

          success = true;
        }
        break;

      case AUTHENTICATION:
      case ASSOCIATION_REQ:
      case REASSOCIATION_REQ:
        // Auth/assoc requests from clients contain the client's MAC in addr2.
        device->init(srcAddr, bssidAddr, channel, rssi, millis(), 0);
        ArpTable::lookupIPAddress(device);
        success = true;
        break;

      case DEAUTHENTICATION:
      case DISASSOCIATION:
        // Deauth/disassoc frames - the source addr2 is the sender.
        device->init(srcAddr, bssidAddr, channel, rssi, millis(), 0);
        success = true;
        break;

      default:
        break;
    }
  }

  return(success);
}

bool PacketSniffer::parseCtrlFrame(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t len, int subtype, Device *device) {
  bool success = false;

  if(wifi_pkt && device) {
    wifi_pkt_rx_ctrl_t *rx_ctrl = &(wifi_pkt->rx_ctrl);
    int rssi = rx_ctrl->rssi;
    int channel = rx_ctrl->channel;

    eth_addr deviceAddr;
    eth_addr emptyBssid = {{0,0,0,0,0,0}};

    switch(subtype) {
      case CTRL_RTS:
      case CTRL_BLOCK_ACK_REQ:
      case CTRL_BLOCK_ACK:
      case CTRL_PS_POLL: {
        // These frames have both RA (addr1) and TA (addr2).
        // The transmitter address (addr2) identifies the sending device.
        wifi_80211_ctrl_rts_frame *frame = (wifi_80211_ctrl_rts_frame *) getFrameStart(wifi_pkt);
        MacAddr_to_eth_addr(&(frame->addr2), deviceAddr);

        // Skip broadcast/multicast
        if(deviceAddr.addr[0] & 0x01) return false;
        if(deviceAddr.addr[3] == 0x0 && deviceAddr.addr[4] == 0x0 && deviceAddr.addr[5] == 0x0) return false;

        device->init(deviceAddr, emptyBssid, channel, rssi, millis(), 0);
        ArpTable::lookupIPAddress(device);
        success = true;
        break;
      }

      case CTRL_CTS:
      case CTRL_ACK: {
        // CTS and ACK frames have only RA (addr1) - the receiver address.
        // The RSSI is from the device that transmitted this frame, but we only
        // know who they're talking TO (the RA). We skip these since we can't
        // reliably identify the transmitter.
        break;
      }

      default:
        break;
    }
  }

  return(success);
}

bool PacketSniffer::parseDataFrame(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t payloadLengthBytes, Device *device) {
  bool success = false;

  Packet *packet = new Packet();
  if(wifi_pkt && device && packet) {
    wifi_pkt_rx_ctrl_t *rx_ctrl = &(wifi_pkt -> rx_ctrl);
    packet -> rssi = rx_ctrl->rssi;
    packet -> channel = rx_ctrl->channel;
    packet -> payloadLengthBytes = payloadLengthBytes;

    //802.11 packet
    wifi_80211_data_frame* frame = (wifi_80211_data_frame*) getFrameStart(wifi_pkt);
    MacAddr_to_eth_addr(&(frame -> sa), packet -> src);
    MacAddr_to_eth_addr(&(frame -> da), packet -> dst);

    wifi_80211_fctl *fctl = &(frame -> fctl);
    byte ds = fctl -> ds;
    if(ds == 1 && eth_addr_cmp(&(packet -> dst), &localBSSID)) {
      //packet sent by this device
      device -> init(packet -> src, localBSSID, packet -> channel, packet -> rssi, millis(), packet -> payloadLengthBytes * -1);
      ArpTable::lookupIPAddress(device);
      success = true;
    }
    else if(ds == 2 && eth_addr_cmp(&(packet -> src), &localBSSID)) {
      //packet sent to this device - RSSI only informative for messages from device
      device -> init(packet -> dst, localBSSID, packet -> channel, packet -> rssi, millis(), packet -> payloadLengthBytes);
      ArpTable::lookupIPAddress(device);
      success = true;
    }
    else {
      //not associated with this bssid - not on this network
    }
  }
  delete(packet);

  return(success);
}

bool PacketSniffer::parseCSI(wifi_csi_info_t *info, Channel *channel) {
  bool success = false;

  #if defined(ESP32)
    if(info->len >= 128) {
      eth_addr thisBssid;
      uint8_t_to_eth_addr(info -> mac, thisBssid);

      //Filter this network:
      if(eth_addr_cmp(&thisBssid, &localBSSID)) {
        channel -> setBssid(thisBssid);
        channel -> setBuffer(info->buf);

        success = true;
      }
    }
  #endif

  return(success);
}