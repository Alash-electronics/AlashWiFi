#pragma once
#include <map>
#include "Arduino.h"
inline std::map<std::string, std::string> g_prefs;
struct Preferences {
  bool begin(const char *, bool = false) { return true; }
  void end() {}
  void putString(const char *k, const char *v) { g_prefs[k] = v; }
  String getString(const char *k, const char *d = "") { auto it = g_prefs.find(k); return String(it == g_prefs.end() ? d : it->second.c_str()); }
  void remove(const char *k) { g_prefs.erase(k); }
  bool isKey(const char *k) { return g_prefs.count(k) > 0; }
  void clear() { g_prefs.clear(); }
};
