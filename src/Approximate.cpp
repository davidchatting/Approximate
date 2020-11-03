/*
    Approximate.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "Approximate.h"

bool Approximate::running = false;

PacketSniffer *Approximate::packetSniffer = PacketSniffer::getInstance();
ArpTable *Approximate::arpTable = NULL;

Approximate::DeviceHandler Approximate::activeDeviceHandler = NULL;
Approximate::DeviceHandler Approximate::proximateDeviceHandler = NULL;

int Approximate::proximateRSSIThreshold = APPROXIMATE_PERSONAL_RSSI;
eth_addr Approximate::localBSSID = {{0,0,0,0,0,0}};
List<Filter *> Approximate::activeDeviceFilterList;

List<Device *> Approximate::proximateDeviceList;
int Approximate::proximateLastSeenTimeoutMs = 60000;

Approximate::Approximate() {
}

bool Approximate::init(bool ipAddressResolution) {
  bool success = false;

  if(WiFi.status() == WL_CONNECTED) {
    this->ssid = WiFi.SSID();
    this->password = WiFi.psk();

    if(init(WiFi.channel(), WiFi.BSSID(), ipAddressResolution)) {
      success = true;
    }
  }

  return(success);
}

bool Approximate::init(String ssid, String password, bool ipAddressResolution) {
  bool success = false;

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && !success; ++i) {
    if(WiFi.SSID(i) == ssid) {
      if(WiFi.encryptionType(i) == 0x7 || password.length() > 0) {
        //Network is either open or a password is supplied
        this->ssid = ssid;
        this->password = password;

        if(init(WiFi.channel(i), WiFi.BSSID(i), ipAddressResolution)) {
          success = true;
        }
      }
    }
  }

  return(success);
}

bool Approximate::init(int channel, uint8_t *bssid, bool ipAddressResolution) {
  bool success = true;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  packetSniffer -> init(channel);
  packetSniffer -> setPacketEventHandler(packetEventHandler);

  eth_addr networkBSSID; 
  uint8_t_to_eth_addr(bssid, networkBSSID);
  setLocalBSSID(networkBSSID);

  String networkBSSIDAsString;
  eth_addr_to_String(networkBSSID, networkBSSIDAsString);
  Serial.printf("\n-\nRouter: %s\t\tChannel: %i\n-\n", networkBSSIDAsString.c_str(), channel);

  if(ipAddressResolution) arpTable = ArpTable::getInstance();

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

void Approximate::connectWiFi() {
  connectWiFi(ssid, password);
}

void Approximate::connectWiFi(String ssid, String password) {
  if(WiFi.status() != WL_CONNECTED) {
    if(ssid.length() > 0) {
      #if defined(ESP8266)
        if (packetSniffer)  packetSniffer -> end();
      #endif

      Serial.printf("Approximate::connectWiFi %s %s\n", ssid.c_str(), password.c_str());
      WiFi.begin(ssid.c_str(), password.c_str());
    }
  }
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

void Approximate::packetEventHandler(wifi_promiscuous_pkt_t *pkt, uint16_t len, int type) {
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
  Packet *packet = new Packet();
  if(wifi_pkt_to_Packet(pkt, payloadLength, packet)) {
    Device *device = new Device();
    if(Approximate::Packet_to_Device(packet, localBSSID, device)) {
      if(proximateDeviceHandler && device -> getRSSI() < 0 && device -> getRSSI() > proximateRSSIThreshold) {
        onProximateDevice(device);
      }

      if(activeDeviceHandler && (activeDeviceFilterList.IsEmpty() || applyDeviceFilters(device))) {
        DeviceEvent event = device -> isUploading() ? Approximate::SEND : Approximate::RECEIVE;
        activeDeviceHandler(device, event); 
      }
    }
    delete(device);
  }
  delete(packet);
}

void Approximate::parseMiscPacket(wifi_promiscuous_pkt_t *pkt) {
}

void Approximate::onProximateDevice(Device *d) {
  if(d) {
    eth_addr macAddress;
    d -> getMacAddress(macAddress);

    Device *proximateDevice = Approximate::getProximateDevice(macAddress);

    if(proximateDevice) {
      proximateDevice->update(d);

      if(activeDeviceHandler) {
        DeviceEvent event = proximateDevice -> isUploading() ? Approximate::SEND : Approximate::RECEIVE;
        activeDeviceHandler(proximateDevice, event);
      }
    }
    else {
      proximateDevice = new Device(d);
      proximateDeviceList.Add(proximateDevice);
      proximateDeviceHandler(proximateDevice, Approximate::ARRIVE);
    }
  }
}

void Approximate::updateProximateDeviceList() {
  if(packetSniffer && packetSniffer -> isRunning()) {
    //only update if we have the possibility of new observations
    Device *proximateDevice = NULL;
    for (int n = 0; n < proximateDeviceList.Count(); n++) {
      proximateDevice = proximateDeviceList[n];

      if((millis() - proximateDevice -> getLastSeenAtMs()) > proximateLastSeenTimeoutMs) {
        proximateDeviceHandler(proximateDevice, Approximate::DEPART);

        proximateDeviceList.Remove(n);
        n=0;
        delete proximateDevice;
      }
    }
  }
}

bool Approximate::isProximateDevice(String macAddress) {
  eth_addr macAddress_eth_addr;
  String_to_eth_addr(macAddress, macAddress_eth_addr);

  return(isProximateDevice(macAddress_eth_addr));
}

bool Approximate::isProximateDevice(eth_addr &macAddress) {
  return(Approximate::getProximateDevice(macAddress));
}

Device *Approximate::getProximateDevice(eth_addr &macAddress) {
  Device *proximateDevice = NULL;

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
  bool success = false;

  //clear:
  for(int n=0; n<6; ++n) out.addr[n] = 0;

  //basic format test ##.##.##.##.##.##
  if(in.length() == 17) {
    int a, b, c, d, e, f;
    sscanf((char *)in.c_str(), "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);

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

bool Approximate::wifi_pkt_to_Packet(wifi_promiscuous_pkt_t *wifi_pkt, uint16_t payloadLengthBytes, Packet *packet) {
  bool success = false;

  if(wifi_pkt && packet) {
    eth_addr src, dst, bssid;
    wifi_mgmt_hdr* header = (wifi_mgmt_hdr*)wifi_pkt -> payload;
    MacAddr_to_eth_addr(&header -> sa, packet -> src);
    MacAddr_to_eth_addr(&header -> da, packet -> dst);
    MacAddr_to_eth_addr(&header -> bssid, packet -> bssid);

    packet -> rssi = wifi_pkt -> rx_ctrl.rssi;
    packet -> channel = wifi_pkt -> rx_ctrl.channel;
    packet -> payloadLengthBytes = payloadLengthBytes;

    success = true;
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
  }

  return(success);
}