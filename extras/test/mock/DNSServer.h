#pragma once
#include "Arduino.h"
struct DNSServer {
  bool running = false; static inline int processed = 0;
  bool start(uint16_t, const char *, IPAddress) { running = true; return true; }
  void processNextRequest() { processed++; }
  void stop() { running = false; }
};
