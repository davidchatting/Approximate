/*
    Approximate.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "Approximate.h"

bool Approximate::running = false;
bool Approximate::onlyIndividualDevices = true;

PacketSniffer *Approximate::packetSniffer = PacketSniffer::getInstance();
ArpTable *Approximate::arpTable = NULL;

Approximate::DeviceHandler Approximate::activeDeviceHandler = NULL;
Approximate::DeviceHandler Approximate::proximateDeviceHandler = NULL;
Approximate::ChannelStateHandler Approximate::channelStateHandler = NULL;

eth_addr Approximate::ownMacAddress = {{0,0,0,0,0,0}};

int Approximate::proximateRSSIThreshold = APPROXIMATE_PERSONAL_RSSI;
eth_addr Approximate::localBSSID = {{0,0,0,0,0,0}};
List<Filter *> Approximate::activeDeviceFilterList;

List<Device *> Approximate::proximateDeviceList;
int Approximate::proximateLastSeenTimeoutMs = 60000;

Approximate::Approximate() {
  uint8_t ma[6];
  WiFi.macAddress(ma);          
  uint8_t_to_eth_addr(ma, ownMacAddress);
}

bool Approximate::init(String ssid, String password, bool ipAddressResolution, bool csiEnabled, bool onlyIndividualDevices) {
  bool success = false;

  if(ssid.length() > 0) {
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n && !success; ++i) {
      if(WiFi.SSID(i) == ssid) {
        if(WiFi.encryptionType(i) == 0x7 || password.length() > 0) {
          //Network is either open or a password is supplied
          strcpy(this->ssid, ssid.c_str());
          strcpy(this->password, password.c_str());

          if(initBlind(WiFi.channel(i), WiFi.BSSID(i), ipAddressResolution, csiEnabled, onlyIndividualDevices)) {
            success = true;
          }
        }
      }
    }
  }
  else {
    success = initBlind(ipAddressResolution, csiEnabled, onlyIndividualDevices);
  }
  
  return(success);
}

bool Approximate::initBlind(int channel, uint8_t *bssid, bool ipAddressResolution, bool csiEnabled, bool onlyIndividualDevices) {
  bool success = true;

  WiFi.disconnect();
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  delay(100);

  packetSniffer -> init(channel);
  packetSniffer -> setPacketEventHandler(parsePacket);
  if(csiEnabled) packetSniffer -> setChannelEventHandler(parseChannelStateInformation);
  this -> onlyIndividualDevices = onlyIndividualDevices;

  eth_addr networkBSSID; 
  uint8_t_to_eth_addr(bssid, networkBSSID);
  setLocalBSSID(networkBSSID);

  String networkBSSIDAsString;
  eth_addr_to_String(networkBSSID, networkBSSIDAsString);
  Serial.printf("\n-\nRouter: %s\t\tChannel: %i\n-\n", networkBSSIDAsString.c_str(), channel);

  if(ipAddressResolution) arpTable = ArpTable::getInstance();

  return(success);
}

bool Approximate::initBlind(bool ipAddressResolution, bool csiEnabled, bool onlyIndividualDevices) {
  bool success = false;

  if(WiFi.status() == WL_CONNECTED) {
    strcpy(this->ssid, WiFi.SSID().c_str());
    strcpy(this->password, WiFi.psk().c_str());

    if(initBlind(WiFi.channel(), WiFi.BSSID(), ipAddressResolution, csiEnabled, onlyIndividualDevices)) {
      success = true;
    }
  }

  return(success);
}

void Approximate::onceWifiStatus(wl_status_t status, voidFnPtr callBackFnPtr) {
  if(status != WL_IDLE_STATUS) {
    if(WiFi.status() == status) {
      callBackFnPtr();
      triggerWifiStatus = WL_IDLE_STATUS;
    }
    else {
      this -> triggerWifiStatus = status;
      this -> onceWifiStatusFnPtr = callBackFnPtr;
    }
  }
}

void Approximate::onceWifiStatus(wl_status_t status, voidFnPtrWithStringPayload callBackFnPtr, String payload) {
  if(status != WL_IDLE_STATUS) {
    if(WiFi.status() == status) {
      callBackFnPtr(payload);
      triggerWifiStatus = WL_IDLE_STATUS;
    }
    else {
      this -> triggerWifiStatus = status;
      this -> onceWifiStatusWithStringPayloadFnPtr = callBackFnPtr;
      this -> onceWifiStatusStringPayload = payload;
    }
  }
}

void Approximate::onceWifiStatus(wl_status_t status, voidFnPtrWithBoolPayload callBackFnPtr, bool payload) {
  if(status != WL_IDLE_STATUS) {
    if(WiFi.status() == status) {
      callBackFnPtr(payload);
      triggerWifiStatus = WL_IDLE_STATUS;
    }
    else {
      this -> triggerWifiStatus = status;
      this -> onceWifiStatusWithBoolPayloadFnPtr = callBackFnPtr;
      this -> onceWifiStatusBoolPayload = payload;
    }
  }
}

void Approximate::onceWifiStatus(wl_status_t status, voidFnPtrWithFnPtrPayload callBackFnPtr, voidFnPtr payload) {
  if(status != WL_IDLE_STATUS) {
    if(WiFi.status() == status) {
      callBackFnPtr(payload);
      triggerWifiStatus = WL_IDLE_STATUS;
    }
    else {
      this -> triggerWifiStatus = status;
      this -> onceWifiStatusWithFnPtrPayloadFnPtr = callBackFnPtr;
      this -> onceWifiStatusFnPtrPayload = payload;
    }
  }
}

void Approximate::begin(voidFnPtr thenFnPtr) {
  Serial.println("Approximate::begin");

  onceWifiStatus(WL_CONNECTED, [](voidFnPtr thenFnPtr) {
    if(thenFnPtr) thenFnPtr();

    if(arpTable) {
      arpTable -> scan(); //blocking
      arpTable -> begin();
    }

    #if defined(ESP8266)
      WiFi.disconnect();
    #endif

    //start the packetSniffer after the scan is complete:
    if(packetSniffer)  packetSniffer -> begin();

    running = true;
  }, thenFnPtr);
  connectWiFi();
  Serial.println("Approximate::begin DONE");
}

void Approximate::end() {
  if (packetSniffer)  packetSniffer -> end();
  if (arpTable)       arpTable -> end();

  running = false;
}

void Approximate::loop() {
  if(running) {
    if (packetSniffer)  {
      packetSniffer -> loop();
    }

    if (arpTable)       arpTable -> loop();

    updateProximateDeviceList(); 
  }

  if(currentWifiStatus != WiFi.status()) {
    printWiFiStatus();
    wl_status_t lastWifiStatus = currentWifiStatus;
    currentWifiStatus = WiFi.status();
    onWifiStatusChange(lastWifiStatus, currentWifiStatus);
  }
}

bool Approximate::isRunning() {
  return(running);
}

void Approximate::onWifiStatusChange(wl_status_t oldStatus, wl_status_t newStatus) {
  if(newStatus != WL_IDLE_STATUS && newStatus == triggerWifiStatus) {
    if(onceWifiStatusFnPtr != NULL ) {
      onceWifiStatusFnPtr();
    }
    else if(onceWifiStatusWithStringPayloadFnPtr != NULL) {
      onceWifiStatusWithStringPayloadFnPtr(onceWifiStatusStringPayload);
    }
    else if(onceWifiStatusWithBoolPayloadFnPtr != NULL) {
      onceWifiStatusWithBoolPayloadFnPtr(onceWifiStatusBoolPayload);
    }
    else if(onceWifiStatusWithFnPtrPayloadFnPtr != NULL) {
      onceWifiStatusWithFnPtrPayloadFnPtr(onceWifiStatusFnPtrPayload);
    }

    onceWifiStatusFnPtr = NULL;
    onceWifiStatusWithStringPayloadFnPtr = NULL;
    onceWifiStatusWithFnPtrPayloadFnPtr = NULL;
    triggerWifiStatus = WL_IDLE_STATUS;
  }
}

wl_status_t Approximate::connectWiFi() {
  return(connectWiFi(this -> ssid, this -> password));
}

wl_status_t Approximate::connectWiFi(String ssid, String password) {
  return(connectWiFi(ssid.c_str(), password.c_str()));
}

wl_status_t Approximate::connectWiFi(char *ssid, char *password) {
  Serial.printf("Approximate::connectWiFi %s %s\n", ssid, password);

  if(WiFi.status() != WL_CONNECTED) {
    if(strlen(ssid) > 0) {
      #if defined(ESP8266)
        if (packetSniffer)  packetSniffer -> end();
        WiFi.begin(ssid, password);

      #elif defined(ESP32)
        //WiFi.begin() for the ESP32 (1.0.4) > https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiSTA.cpp - doesn't call esp_wifi_init() or esp_wifi_start() - which are needed later for esp_wifi_set_csi()
        tcpip_adapter_init();
        esp_event_loop_init(NULL, NULL);

        if(!WiFi.enableSTA(true)) {
            log_e("STA enable failed!");
            return WL_CONNECT_FAILED;
        }

        if(!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
            log_e("SSID too long or missing!");
            return WL_CONNECT_FAILED;
        }

        if(password && strlen(password) > 64) {
            log_e("password too long!");
            return WL_CONNECT_FAILED;

        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);

        wifi_config_t conf;
        memset(&conf, 0, sizeof(wifi_config_t));
        strcpy(reinterpret_cast<char*>(conf.sta.ssid), ssid);

        if(password) {
            if (strlen(password) == 64){ // it's not a password, is the PSK
                memcpy(reinterpret_cast<char*>(conf.sta.password), password, 64);
            } else {
                strcpy(reinterpret_cast<char*>(conf.sta.password), password);
            }
        }

        if(esp_wifi_disconnect()){
            log_e("disconnect failed!");
            return WL_CONNECT_FAILED;
        }
        esp_wifi_set_config(WIFI_IF_STA, &conf);

        if(tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA) == ESP_ERR_TCPIP_ADAPTER_DHCPC_START_FAILED){
            log_e("dhcp client start failed!");
            return WL_CONNECT_FAILED;
        }

        esp_wifi_start();

        if(esp_wifi_connect()) {
            log_e("connect failed!");
            return WL_CONNECT_FAILED;
        }
        
      #endif
    }
  }

  return(WiFi.status());
}

void Approximate::disconnectWiFi() {
  WiFi.disconnect();

  #if defined(ESP8266)
    if (running && packetSniffer)  packetSniffer -> begin();
  #endif
}

void Approximate::printWiFiStatus() {
  switch(WiFi.status()) {
    case WL_CONNECTED:        Serial.println("WL_CONNECTED"); break;
    case WL_NO_SHIELD:        Serial.println("WL_NO_SHIELD"); break;
    case WL_IDLE_STATUS:      Serial.println("WL_IDLE_STATUS"); break;
    case WL_NO_SSID_AVAIL:    Serial.println("WL_NO_SSID_AVAIL"); break;
    case WL_SCAN_COMPLETED:   Serial.println("WL_SCAN_COMPLETED"); break;
    case WL_CONNECT_FAILED:   Serial.println("WL_CONNECT_FAILED"); break;
    case WL_CONNECTION_LOST:  Serial.println("WL_CONNECTION_LOST"); break;
    case WL_DISCONNECTED:     Serial.println("WL_DISCONNECTED"); break;
  }
}

void Approximate::addActiveDeviceFilter(String macAddress) {
  eth_addr macAddress_eth_addr;
  String_to_eth_addr(macAddress, macAddress_eth_addr);

  addActiveDeviceFilter(macAddress_eth_addr);
}

void Approximate::addActiveDeviceFilter(char *macAddress) {
  eth_addr macAddress_eth_addr;
  c_str_to_eth_addr(macAddress, macAddress_eth_addr);

  addActiveDeviceFilter(macAddress_eth_addr);
}

void Approximate::addActiveDeviceFilter(Device &device) {
  eth_addr macAddress;
  device.getMacAddress(macAddress);

  addActiveDeviceFilter(macAddress);
}

void Approximate::addActiveDeviceFilter(Device *device) {
  eth_addr macAddress;
  device -> getMacAddress(macAddress);

  addActiveDeviceFilter(macAddress);
}

void Approximate::addActiveDeviceFilter(int oui) {
  eth_addr macAddress;
  oui_to_eth_addr(oui, macAddress);

  addActiveDeviceFilter(macAddress);
}

void Approximate::addActiveDeviceFilter(eth_addr &macAddress) {
  Filter *f = new Filter(macAddress);
  activeDeviceFilterList.Add(f);
}

void Approximate::setActiveDeviceFilter(String macAddress) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(macAddress);
}

void Approximate::setActiveDeviceFilter(char *macAddress) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(macAddress);
}

void Approximate::setActiveDeviceFilter(Device &device) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(device);
}

void Approximate::setActiveDeviceFilter(Device *device) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(device);
}

void Approximate::setActiveDeviceFilter(eth_addr &macAddress) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(macAddress);
}

void Approximate::setActiveDeviceFilter(int oui) {
  removeAllActiveDeviceFilters();
  addActiveDeviceFilter(oui);
}

void Approximate::removeActiveDeviceFilter(String macAddress) {
  eth_addr macAddress_eth_addr;
  String_to_eth_addr(macAddress, macAddress_eth_addr);

  removeActiveDeviceFilter(macAddress_eth_addr);
}

void Approximate::removeActiveDeviceFilter(Device &device) {
  eth_addr macAddress;
  device.getMacAddress(macAddress);

  removeActiveDeviceFilter(macAddress);
}

void Approximate::removeActiveDeviceFilter(Device *device) {
  eth_addr macAddress;
  device -> getMacAddress(macAddress);

  removeActiveDeviceFilter(macAddress);
}

void Approximate::removeActiveDeviceFilter(int oui) {
  eth_addr macAddress;
  oui_to_eth_addr(oui, macAddress);

  removeActiveDeviceFilter(macAddress);
}

void Approximate::removeActiveDeviceFilter(eth_addr &macAddress) {
  for (int n = 0; n < activeDeviceFilterList.Count(); n++) {
    Filter *thisFilter = activeDeviceFilterList[n];
    if(thisFilter -> matches(&macAddress)) {
      activeDeviceFilterList.Remove(n);
      delete thisFilter;
      n = 0;  //reset the count in case multiple matches
    }
  }
}

void Approximate::removeAllActiveDeviceFilters() {
  for (int n = 0; n < activeDeviceFilterList.Count(); n++) {
    Filter *thisFilter = activeDeviceFilterList[n];
    activeDeviceFilterList.Remove(n);
    delete thisFilter;
    n = 0;  //reset
  }
}

bool Approximate::applyDeviceFilters(Device *device) {
  bool result = false;

  for (int n = 0; n < activeDeviceFilterList.Count() && !result; n++) {
    Filter *thisFilter = activeDeviceFilterList[n];
    result = thisFilter -> matches(device);
  }

  return(result);
}

void Approximate::setLocalBSSID(String macAddress) {
  eth_addr macAddress_eth_addr;
  String_to_eth_addr(macAddress, macAddress_eth_addr);

  setLocalBSSID(macAddress_eth_addr);
}

void Approximate::setLocalBSSID(eth_addr &macAddress) {
  ETHADDR16_COPY(&this -> localBSSID, &macAddress);
}

void Approximate::setActiveDeviceHandler(DeviceHandler activeDeviceHandler, bool inclusive) {
  if(!inclusive) {
    addActiveDeviceFilter(Filter::NONE); 
  }
  Approximate::activeDeviceHandler = activeDeviceHandler;
}

void Approximate::setProximateDeviceHandler(DeviceHandler deviceHandler, int rssiThreshold, int lastSeenTimeoutMs) {
  setProximateRSSIThreshold(rssiThreshold);
  setProximateLastSeenTimeoutMs(lastSeenTimeoutMs);
  Approximate::proximateDeviceHandler = deviceHandler;
}

void Approximate::setProximateRSSIThreshold(int proximateRSSIThreshold) {
  Approximate::proximateRSSIThreshold = proximateRSSIThreshold;
}

void Approximate::setProximateLastSeenTimeoutMs(int proximateLastSeenTimeoutMs) {
  Approximate::proximateLastSeenTimeoutMs = proximateLastSeenTimeoutMs;
}

void Approximate::setChannelStateHandler(ChannelStateHandler channelStateHandler){
  Approximate::channelStateHandler = channelStateHandler;
}

void Approximate::parsePacket(wifi_promiscuous_pkt_t *pkt, uint16_t len, int type) {
  switch (type) {
    case PKT_MGMT: parseMgmtPacket(pkt); break;
    case PKT_CTRL: parseCtrlPacket(pkt); break;
    case PKT_DATA: parseDataPacket(pkt, len); break;
    case PKT_MISC: parseMiscPacket(pkt); break;
  }
}

void Approximate::parseCtrlPacket(wifi_promiscuous_pkt_t *pkt) {
}

void Approximate::parseMgmtPacket(wifi_promiscuous_pkt_t *pkt) {
}

void Approximate::parseDataPacket(wifi_promiscuous_pkt_t *pkt, uint16_t payloadLength) {
  Device *device = new Device();
  if(Approximate::wifi_promiscuous_pkt_to_Device(pkt, payloadLength, device)) {
    if(!device -> matches(ownMacAddress) && (!onlyIndividualDevices || device -> isIndividual())) {
      if(proximateDeviceHandler) {
        Device *proximateDevice = Approximate::getProximateDevice(device);
        int rssi = device -> getRSSI();

        if(rssi != APPROXIMATE_UNKNOWN_RSSI) {
          if(rssi > proximateRSSIThreshold) {
            if(proximateDevice) {
              //A known proximate device - already in the list
              proximateDevice->update(device);
            }
            else {
              //A new proximate device - not already in the list
              proximateDevice = new Device(device);

              proximateDeviceList.Add(proximateDevice);
              proximateDeviceHandler(proximateDevice, Approximate::ARRIVE);
            }
            proximateDeviceHandler(proximateDevice, proximateDevice -> isUploading() ? Approximate::SEND : Approximate::RECEIVE);
            proximateDevice -> setTimeOutAtMs(millis() + proximateLastSeenTimeoutMs);
          }
          else {
            if(proximateDevice) proximateDevice->update(device);
          }
        }
      }

      if(activeDeviceHandler && (activeDeviceFilterList.IsEmpty() || applyDeviceFilters(device))) {
        activeDeviceHandler(device, device -> isUploading() ? Approximate::SEND : Approximate::RECEIVE); 
      }
    }
  }
  delete(device);
}

void Approximate::parseMiscPacket(wifi_promiscuous_pkt_t *pkt) {
}

void Approximate::parseChannelStateInformation(wifi_csi_info_t *info) {
  #if defined(ESP32)
    if(channelStateHandler) {
      Channel *channel = new Channel();
      if(wifi_csi_info_to_Channel(info, channel)) {
        //TODO: apply filtering
        channelStateHandler(channel);
      }
      delete channel;
    }
  #endif
}

void Approximate::updateProximateDeviceList() {
  if(packetSniffer && packetSniffer -> isRunning() && proximateLastSeenTimeoutMs > 0) {
    //only update if we have the possibility of new observations
    Device *proximateDevice = NULL;
    for (int n = 0; n < proximateDeviceList.Count(); n++) {
      proximateDevice = proximateDeviceList[n];

      if(proximateDevice -> hasTimedOut()) {
        proximateDeviceHandler(proximateDevice, Approximate::DEPART);

        proximateDeviceList.Remove(n);
        delete proximateDevice;
        n=0;
      }
    }
  }
}

bool Approximate::isProximateDevice(Device *device) {
  bool result = false;

  if(device) {
    eth_addr macAddress_eth_addr;
    device -> getMacAddress(macAddress_eth_addr);
    result = Approximate::getProximateDevice(macAddress_eth_addr);
  }

  return(result);
}

bool Approximate::isProximateDevice(String macAddress) {
  eth_addr macAddress_eth_addr;
  String_to_eth_addr(macAddress, macAddress_eth_addr);

  return(isProximateDevice(macAddress_eth_addr));
}

bool Approximate::isProximateDevice(eth_addr &macAddress) {
  return(Approximate::getProximateDevice(macAddress));
}

Device *Approximate::getProximateDevice(Device *device) {
  Device *proximateDevice = NULL;

  if(device) {
    eth_addr macAddress;
    device -> getMacAddress(macAddress);
    proximateDevice = getProximateDevice(macAddress);
  }

  return(proximateDevice);
}

Device *Approximate::getProximateDevice(eth_addr &macAddress) {
  Device *proximateDevice = NULL;

  //Get known proximate device with this mac address:
  for (int n = 0; n < proximateDeviceList.Count() && !proximateDevice; n++) {
		if(proximateDeviceList[n] -> matches(macAddress)) {
      proximateDevice = proximateDeviceList[n];
    }
	}

  return(proximateDevice);
}

bool Approximate::MacAddr_to_eth_addr(MacAddr *in, eth_addr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.addr[n] = in->mac[n];

  return(success);
}

bool Approximate::uint8_t_to_eth_addr(uint8_t *in, eth_addr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.addr[n] = in[n];

  return(success);
}

bool Approximate::oui_to_eth_addr(int oui, eth_addr &out) {
  bool success = true;

  out.addr[0] = (oui >> 16) & 0xFF;
  out.addr[1] = (oui >> 8) & 0xFF;
  out.addr[2] = (oui >> 0) & 0xFF;
  out.addr[3] = 0xFF;
  out.addr[4] = 0xFF;
  out.addr[5] = 0xFF;

  return(success);
}

bool Approximate::String_to_eth_addr(String &in, eth_addr &out) {
  bool success = c_str_to_eth_addr(in.c_str(), out);

  return(success);
}

bool Approximate::c_str_to_eth_addr(const char *in, eth_addr &out) {
  bool success = false;

  //clear:
  for(int n=0; n<6; ++n) out.addr[n] = 0;

  //basic format test ##:##:##:##:##:##
  if(strlen(in) == 17) {
    int a, b, c, d, e, f;
    sscanf(in, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);

    out.addr[0] = a;
    out.addr[1] = b;
    out.addr[2] = c;
    out.addr[3] = d;
    out.addr[4] = e;
    out.addr[5] = f;

    success = true;
  }

  return(success);
}

bool Approximate::eth_addr_to_String(eth_addr &in, String &out) {
  bool success = true;

  char macAddressAsCharArray[18];
  eth_addr_to_c_str(in, macAddressAsCharArray);
  out = String(macAddressAsCharArray);

  return(success);
}

bool Approximate::eth_addr_to_c_str(eth_addr &in, char *out) {
  bool success = true;

  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X\0", in.addr[0], in.addr[1], in.addr[2], in.addr[3], in.addr[4], in.addr[5]);

  return(success);
}

bool Approximate::MacAddr_to_c_str(MacAddr *in, char *out) {
  bool success = true;

  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X\0", in->mac[0], in->mac[1], in->mac[2], in->mac[3], in->mac[4], in->mac[5]);

  return(success);
}

bool Approximate::MacAddr_to_oui(MacAddr *in, int &out) {
  bool success = true;

  out = ((in->mac[0] << 16) & 0xFF0000) | ((in->mac[1] << 8) & 0xFF00) | ((in->mac[2] << 0) & 0xFF);

  return(success);
}

bool Approximate::MacAddr_to_MacAddr(MacAddr *in, MacAddr &out) {
  bool success = true;

  for(int n=0; n<6; ++n) out.mac[n] = in -> mac[n];

  return(success);
}

bool Approximate::wifi_promiscuous_pkt_to_Device(wifi_promiscuous_pkt_t *pkt, uint16_t payloadLengthBytes, Device *device) {
  bool success = false;

  Packet *packet = new Packet();
  if(wifi_promiscuous_pkt_to_Packet(pkt, payloadLengthBytes, packet)) {
      if(Approximate::Packet_to_Device(packet, localBSSID, device)) {
        success = true;
      }
  }
  delete(packet);
  
  return(success);
}

bool Approximate::wifi_promiscuous_pkt_to_Packet(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t payloadLengthBytes, Packet *packet) {
  bool success = false;

  if(wifi_pkt && packet) {
    packet -> rssi = wifi_pkt -> rx_ctrl.rssi;
    packet -> channel = wifi_pkt -> rx_ctrl.channel;
    packet -> payloadLengthBytes = payloadLengthBytes;

    if(wifi_pkt -> rx_ctrl.sig_mode == 0) {
      //Has a 802.11 header
      wifi_mgmt_hdr* header = (wifi_mgmt_hdr*)wifi_pkt -> payload;
      MacAddr_to_eth_addr(&header -> sa, packet -> src);
      MacAddr_to_eth_addr(&header -> da, packet -> dst);
      //MacAddr_to_eth_addr(&header -> bssid, packet -> bssid);

      success = true;
    }
    else {
      ///Has a 802.11n header
    }
  }

  return(success);
}

bool Approximate::Packet_to_Device(Packet *packet, eth_addr &bssid, Device *device) {
  bool success = false;

  if(packet && device) {
    if(eth_addr_cmp(&(packet -> src), &bssid)) {
      //packet sent to this device - RSSI only informative for messages from device
      device -> init(packet -> dst, bssid, packet -> channel, packet -> rssi, millis(), packet -> payloadLengthBytes);
      ArpTable::lookupIPAddress(device);
      success = true;
    }
    else if(eth_addr_cmp(&(packet -> dst), &bssid)) {
      //packet sent by this device
      device -> init(packet -> src, bssid, packet -> channel, packet -> rssi, millis(), packet -> payloadLengthBytes * -1);
      ArpTable::lookupIPAddress(device);
      success = true;
    }
    else {
      //not associated with this bssid - not on this network
    }
  }

  return(success);
}

bool Approximate::wifi_csi_info_to_Channel(wifi_csi_info_t *info, Channel *channel) {
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