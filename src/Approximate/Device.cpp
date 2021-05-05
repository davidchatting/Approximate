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
    setMacAddress(macAddress);
    setBssid(bssid);
    
    setChannel(channel);
    setRSSI(rssi);
    setLastSeenAtMs(lastSeenAtMs);
    setDataFlowBytes(dataFlowBytes);

    setIPAddress(ipAddress);
}

void Device::update(Device *d) {
    if(d) init(d -> macAddress, d -> bssid, d -> channel, d -> rssi, d -> lastSeenAtMs, d -> dataFlowBytes, d -> ipAddress.addr);
}

void Device::getMacAddress(eth_addr &macAddress) {
    ETHADDR16_COPY(&macAddress, &this -> macAddress);
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

void Device::setMacAddress(eth_addr &macAddress) {
    ETHADDR16_COPY(&this -> macAddress, &macAddress);
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
    setIPAddress(ipAddress.addr);
}

void Device::setIPAddress(u32_t ipAddress) {
    this -> ipAddress.addr = ipAddress;
}

bool Device::hasIPAddress() {
    return(ipAddress.addr != IPADDR_ANY);
}

void Device::setRSSI(int rssi) {
    this -> rssi = rssi;
}

int Device::getRSSI(bool uploadOnly) {
    int result = APPROXIMATE_UNKNOWN_RSSI;
    
    if(uploadOnly) result = isUploading() ? rssi : APPROXIMATE_UNKNOWN_RSSI;
    else result = rssi;

    return(rssi);
}

void Device::setLastSeenAtMs(long lastSeenAtMs) {
    if(lastSeenAtMs == -1) lastSeenAtMs = millis(); 
    this -> lastSeenAtMs = lastSeenAtMs;
}

int Device::getLastSeenAtMs() {
    return(lastSeenAtMs);
}

void Device::setTimeOutAtMs(long timeOutAtMs) {
    this -> timeOutAtMs = timeOutAtMs;
}

void Device::setReducedTimeOutAtMs(long timeOutAtMs) {
    setTimeOutAtMs(min(timeOutAtMs, this -> timeOutAtMs));
}

bool Device::hasTimedOut() {
    return(millis() > timeOutAtMs || timeOutAtMs == -1);
}

bool Device::matches(eth_addr &macAddress) {
    return(eth_addr_cmp(&this -> macAddress, &macAddress));
}

uint32_t Device::getOUI() {
    uint32_t oui = 0;
    
    oui = (macAddress.addr[0] << 16) | (macAddress.addr[1] << 8) | (macAddress.addr[2] & 0xFF);

    return(oui);
}

void Device::setDataFlowBytes(int dataFlowBytes) {
    this -> dataFlowBytes = dataFlowBytes;
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

bool Device::isUniversal() {
    return(!isLocal());
}

bool Device::isIndividual() {
    //sometimes there are junk mac addresses where the last half is all zeros
    return(!isGroup() && !(macAddress.addr[3] == 0x0 && macAddress.addr[4] == 0x0 && macAddress.addr[5] == 0x0));
}

//Universal/local and individual/group defined by: https://standards.ieee.org/content/dam/ieee-standards/standards/web/documents/tutorials/macgrp.pdf

bool Device::isLocal() {
    return((macAddress.addr[0] & 0x2 == 0x2) && !isGroup());
}

bool Device::isGroup() {
    return(macAddress.addr[0] & 0x1 == 0x1);
}