/*
    PacketSniffer.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "PacketSniffer.h"

PacketSniffer::PacketEventHandler PacketSniffer::packetEventHandler = NULL;
PacketSniffer::ChannelEventHandler PacketSniffer::channelEventHandler = NULL;
bool PacketSniffer::running = false;

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

          tcpip_adapter_init();
          esp_event_loop_init(NULL, NULL);

          wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
          esp_wifi_init(&cfg);

          esp_wifi_set_mode(WIFI_MODE_APSTA);
          esp_wifi_start();
        }
      #endif

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

void PacketSniffer::rxCallback_8266(uint8_t *buf, uint16_t len) {
  wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *) buf;

  unsigned int frameControl = ((unsigned int)packet->payload[1] << 8) + packet->payload[0];
  wifi_promiscuous_pkt_type_t type = (wifi_promiscuous_pkt_type_t) ((frameControl & 0b0000000000001100) >> 2);

  uint16_t sig_len = 0;
  #if defined(ESP8266)
    sig_len = packet->rx_ctrl.sig_mode ? packet->rx_ctrl.HT_length : packet->rx_ctrl.legacy_length;
  #endif

  rxCallback(packet, sig_len, type);
}

void PacketSniffer::rxCallback_32(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *) buf;
  
  uint16_t sig_len = 0;
  #if defined(ESP32)
    sig_len = packet->rx_ctrl.sig_len;
  #endif

  rxCallback(packet, sig_len, type);
}

void PacketSniffer::rxCallback(wifi_promiscuous_pkt_t *packet, uint16_t len, wifi_promiscuous_pkt_type_t type) {
  if (running && packetEventHandler) {
    packetEventHandler(packet, len, (int) type);
  }
}

void PacketSniffer::csiCallback_32(void *ctx, wifi_csi_info_t *data) {
  if (running && channelEventHandler) {
    channelEventHandler(data);
  }
}