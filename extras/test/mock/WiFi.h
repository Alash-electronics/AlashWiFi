// Имитация класса WiFi (ESP32): точки доступа задаёт тест, подключение занимает время.
#pragma once
#include <vector>
#include "Arduino.h"
enum { WL_NO_SHIELD = 255, WL_IDLE_STATUS = 0, WL_NO_SSID_AVAIL = 1, WL_SCAN_COMPLETED = 2, WL_CONNECTED = 3, WL_CONNECT_FAILED = 4, WL_CONNECTION_LOST = 5, WL_DISCONNECTED = 6 };
enum WiFiMode_t { WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 };
#define WIFI_AUTH_OPEN 0
struct FakeAp { std::string ssid, pass; int rssi = -60, ch = 6; bool open = false; bool neverConnects = false; };
struct WiFiClass {
  std::vector<FakeAp> aps;
  WiFiMode_t md = WIFI_OFF; uint8_t st = WL_DISCONNECTED;
  uint32_t doneAt = 0; int target = -1; bool outcomeConnected = false; uint8_t outcomeStatus = WL_DISCONNECTED;
  int begins = 0, scans = 0; std::string beginSsid, lastHost, apName; bool apUp = false; bool apDisconnected = false;
  std::string connectedSsid; bool dropped = false; bool autoRec = false;
  void persistent(bool) {}
  void mode(WiFiMode_t m) { md = m; }
  WiFiMode_t getMode() { return md; }
  void setAutoReconnect(bool b) { autoRec = b; }
  void setHostname(const char *h) { lastHost = h; }
  void begin(const char *ssid, const char *pass) {
    begins++; beginSsid = ssid; connectedSsid.clear(); dropped = false; doneAt = g_ms; target = -1;
    for (size_t i = 0; i < aps.size(); i++) if (aps[i].ssid == ssid) target = (int)i;
    if (target < 0) { outcomeStatus = WL_NO_SSID_AVAIL; doneAt = g_ms + 1000; }
    else if (aps[target].neverConnects) { outcomeStatus = WL_DISCONNECTED; doneAt = g_ms; }
    else if (aps[target].pass == (pass ? pass : "")) { outcomeStatus = WL_CONNECTED; doneAt = g_ms + 2000; }
    else { outcomeStatus = WL_CONNECT_FAILED; doneAt = g_ms + 2000; }
    st = WL_IDLE_STATUS;
  }
  uint8_t status() {
    if (dropped) return WL_DISCONNECTED;
    if (st != WL_CONNECTED && g_ms >= doneAt && doneAt != 0) { st = outcomeStatus; if (st == WL_CONNECTED) connectedSsid = beginSsid; }
    return st;
  }
  void disconnect() { st = WL_DISCONNECTED; doneAt = 0; connectedSsid.clear(); }
  IPAddress localIP() { return IPAddress(192, 168, 1, 50); }
  int RSSI() { return rssiNow; }
  int rssiNow = -60;
  String SSID() { return String(connectedSsid.c_str()); }
  String SSID(int i) { return String(aps[i].ssid.c_str()); }
  int RSSI(int i) { return aps[i].rssi; }
  int channel(int i) { return aps[i].ch; }
  int encryptionType(int i) { return aps[i].open ? 0 : 3; }
  int scanNetworks() { scans++; g_ms += 2000; return (int)aps.size(); }
  void scanDelete() {}
  bool softAP(const char *n, const char * = nullptr) { apName = n; apUp = true; return true; }
  IPAddress softAPIP() { return IPAddress(192, 168, 4, 1); }
  void softAPdisconnect(bool) { apUp = false; apDisconnected = true; }
  void dropConnection() { dropped = true; st = WL_DISCONNECTED; }
};
inline WiFiClass WiFi;
