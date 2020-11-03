/*
    Device.cpp
    Approximate Library
    -
    David Chatting - github.com/davidchatting/Approximate
    MIT License - Copyright (c) October 2020
*/

#include "Device.h"
#include "Approximate.h"

Device::Device() {
    ipAddress.addr = IPADDR_ANY;
}

Device::Device(Device *b) {
    init(b -> macAddress, b -> bssid, b -> channel, b -> rssi, b -> lastSeenAtMs, b -> dataFlowBytes, b -> ipAddress.addr);
}

Device::Device(eth_addr &macAddress, eth_addr &bssid, int channel, int rssi, long lastSeenAtMs, int dataFlowBytes, u32_t ipAddress) {
    init(macAddress, bssid, channel, rssi, lastSeenAtMs, dataFlowBytes, ipAddress);
}

//TODO: tidy-up these operators and matches()
bool Device::operator ==(Device *b) {
    return(*this == *b);
}

bool Device::operator ==(Device const& b) {
    return(eth_addr_cmp(&this -> macAddress, &b.macAddress));
}

bool Device::operator ==(eth_addr &macAddress) {
    return(matches(macAddress));
}

void Device::init(eth_addr &macAddress, eth_addr &bssid, int channel, int rssi, long lastSeenAtMs, int dataFlowBytes, u32_t ipAddress) {
    ETHADDR16_COPY(&this -> macAddress, &macAddress);
    ETHADDR16_COPY(&this -> bssid, &bssid);
    
    this -> channel = channel;
    this -> rssi = rssi;
    setLastSeenAtMs(lastSeenAtMs);
    this -> dataFlowBytes = dataFlowBytes;

    this -> ipAddress.addr = ipAddress;
}

void Device::update(Device *d) {
    if(d) init(d -> macAddress, d -> bssid, d -> channel, d -> rssi, d -> lastSeenAtMs, d -> dataFlowBytes, d -> ipAddress.addr);
}

void Device::getMacAddress(eth_addr &macAddress) {
    eth_addr_cmp(&this -> macAddress, &macAddress);
}

String Device::getMacAddressAsString() {
    String macAddressAsString = "";

    Approximate::eth_addr_to_String(macAddress, macAddressAsString);

    return(macAddressAsString);
}

char *Device::getMacAddressAs_c_str(char *out) {
    Approximate::eth_addr_to_c_str(macAddress, out);
    
    return(out);
}

String Device::getBssidAsString() {
    String bssidAsString = "";

    Approximate::eth_addr_to_String(bssid, bssidAsString);

    return(bssidAsString);
}

char *Device::getBssidAs_c_str(char *out) {
    Approximate::eth_addr_to_c_str(bssid, out);
    
    return(out);
}

void Device::getIPAddress(ip4_addr_t &ipAddress) {
    ipAddress.addr = this -> ipAddress.addr;
}

String Device::getIPAddressAsString() {
    String ipAddressAsString;
    ipAddressAsString.reserve(16);

    if(ipAddress.addr != IPADDR_ANY) {
        ipAddressAsString = String(ip4addr_ntoa(&ipAddress));
    }

    return(ipAddressAsString);
}

char *Device::getIPAddressAs_c_str(char *out) {
    if(ipAddress.addr != IPADDR_ANY) {
        strcpy(out, ip4addr_ntoa(&ipAddress));
    }

    return(out);
}

void Device::setIPAddress(ip4_addr_t &ipAddress) {
    this -> ipAddress.addr = ipAddress.addr;
}

bool Device::hasIPAddress() {
    return(ipAddress.addr != IPADDR_ANY);
}

void Device::setRSSI(int rssi) {
    this -> rssi = rssi;
}

int Device::getRSSI() {
    return(rssi);
}

void Device::setLastSeenAtMs(long lastSeenAtMs) {
    if(lastSeenAtMs == -1) lastSeenAtMs = millis(); 
    this -> lastSeenAtMs = lastSeenAtMs;
}

int Device::getLastSeenAtMs() {
    return(lastSeenAtMs);
}

bool Device::matches(eth_addr &macAddress) {
    return(eth_addr_cmp(&this -> macAddress, &macAddress));
}

uint32_t Device::getOUI() {
    uint32_t oui = 0;
    
    oui = (macAddress.addr[0] << 16) | (macAddress.addr[1] << 8) | (macAddress.addr[2] & 0xFF);

    return(oui);
}

int Device::getChannel() {
    return(channel);
}

bool Device::isUploading() {
    return(dataFlowBytes < 0);
}

bool Device::isDownloading() {
    return(dataFlowBytes > 0);
}

int Device::getDownloadSizeBytes() {
    int downloadSizeBytes = isDownloading() ? getPayloadSizeBytes() : 0;

    return(downloadSizeBytes);
}

int Device::getUploadSizeBytes() {
    int uploadSizeBytes = isUploading() ? getPayloadSizeBytes() : 0;

    return(uploadSizeBytes);
}

int Device::getPayloadSizeBytes(){
    return(abs(dataFlowBytes));
}