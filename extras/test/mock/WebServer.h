// Имитация WebServer (ESP32): обработчики запоминаются, запросы подаёт тест через очередь, ответы записываются.
#pragma once
#include <map>
#include <vector>
#include "Arduino.h"
struct HttpReq { std::string path; std::map<std::string, std::string> args; };
struct HttpResp { int code; std::string type, body, location; };
inline std::vector<HttpReq> g_httpQueue;
inline std::vector<HttpResp> g_httpResp;
class WebServer {
 public:
  explicit WebServer(int) {}
  std::map<std::string, std::function<void()>> routes; std::function<void()> notFound; bool started = false;
  HttpReq cur; std::string locationHeader;
  void on(const char *path, std::function<void()> f) { routes[path] = f; }
  void onNotFound(std::function<void()> f) { notFound = f; }
  void begin() { started = true; }
  void close() { started = false; }
  String arg(const char *name) { auto it = cur.args.find(name); return String(it == cur.args.end() ? "" : it->second.c_str()); }
  void sendHeader(const char *, const String &v, bool = false) { locationHeader = v.s; }
  void send(int code, const char *type, const String &body) { g_httpResp.push_back({code, type, body.s, locationHeader}); locationHeader.clear(); }
  void handleClient() {
    while (!g_httpQueue.empty()) {
      cur = g_httpQueue.front(); g_httpQueue.erase(g_httpQueue.begin());
      auto it = routes.find(cur.path);
      if (it != routes.end()) it->second(); else if (notFound) notFound();
    }
  }
};
