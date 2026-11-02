// AlashWiFi: подключение к Wi-Fi для ESP32 и ESP8266 без боли: понятные причины ошибок, автопереподключение,
// сохранение пароля и портал настройки с телефона (имя и пароль сети вводятся на странице, как у WiFiManager).
// Без зависимостей (встроенные WiFi, WebServer, DNSServer). MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashWiFi
#ifndef ALASH_WIFI_H
#define ALASH_WIFI_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
typedef WebServer AlashWebServerType;
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
typedef ESP8266WebServer AlashWebServerType;
#endif

#ifndef ALASH_WIFI_MAX_PARAMS
#define ALASH_WIFI_MAX_PARAMS 6                          // сколько своих полей можно добавить на страницу портала
#endif

class AlashWiFi {
 public:
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_NETWORK,        // сеть с таким именем не найдена (имя неверно, роутер далеко, сеть 5 ГГц)
    ERR_WRONG_PASSWORD,    // пароль не подошёл
    ERR_TIMEOUT,           // подключиться не удалось за отведённое время
    ERR_NO_CREDENTIALS,    // сохранённого имени и пароля нет
    ERR_PORTAL_TIMEOUT,    // портал настройки закрыт по времени: никто не ввёл данные
    ERR_BAD_INPUT,         // имя пустое или длиннее 32 символов; пароль 1-7 или больше 63 символов
    ERR_BAD_PARAM          // свой параметр портала: плохой id, повтор, слишком много или не хватает места
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  // ---- Подключение ---------------------------------------------------------------------
  // Подключиться к сети и ждать до timeoutMs. Пока ждёт, печатает точки в progress (см. setProgress). true при успехе.
  bool begin(const char *ssid, const char *password, uint16_t timeoutMs = 15000);
  bool beginSaved(uint16_t timeoutMs = 15000);          // подключиться к сети, сохранённой через save() или порталом
  // Как у WiFiManager: сохранённая сеть, а если не получилось, портал настройки с телефона (точка доступа apName).
  bool autoConnect(const char *apName = "AlashSetup", const char *apPassword = "", uint16_t portalTimeoutSec = 180, uint16_t connectTimeoutMs = 15000);
  bool startPortal(const char *apName = "AlashSetup", const char *apPassword = "", uint16_t timeoutSec = 180);

  // ---- Свои поля на странице портала (адрес MQTT, токен бота, имя устройства) ----------------------------
  // Добавлять до autoConnect() / startPortal(). id: латиница, цифры и _ (до 13 символов). Значения сохраняются вместе с сетью.
  AlashWiFi() {}
  ~AlashWiFi();
  AlashWiFi(const AlashWiFi &) = delete;
  AlashWiFi &operator=(const AlashWiFi &) = delete;
  bool addParam(const char *id, const char *label, const char *defaultValue = "", uint8_t maxLen = 40);
  const char *param(const char *id);                     // текущее значение: введённое в портале или по умолчанию; "" если id нет
  bool setParam(const char *id, const char *value);      // задать и сохранить из кода
  uint8_t paramCount() const { return _np; }
  // Формат хранения для ESP8266 (EEPROM), доступен для тестов
  static size_t packParams(uint8_t *buf, size_t cap, char *const *values, const uint8_t *maxLens, uint8_t n);
  static bool unpackParams(const uint8_t *buf, size_t len, char *const *values, const uint8_t *maxLens, uint8_t n);

  // ---- Состояние ---------------------------------------------------------------------------
  bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
  IPAddress ip() const { return WiFi.localIP(); }
  int rssi() const { return WiFi.RSSI(); }               // уровень сигнала, дБм (-50 отлично, -70 нормально, -80 слабо)
  const __FlashStringHelper *signalText();               // сигнал словами: "отличный", "хороший", "слабый"
  void update();                                         // вызывайте в loop(): переподключает при потере связи

  // ---- Сохранённая сеть ----------------------------------------------------------------------
  bool save(const char *ssid, const char *password);
  bool forget();
  bool hasSaved();
  bool savedSsid(char *out, size_t size);                // имя сохранённой сети (пароль не отдаётся)

  // ---- Настройки -------------------------------------------------------------------------------
  void setAutoReconnect(bool on) { _auto = on; }        // переподключаться самой (по умолчанию включено)
  void setHostname(const char *name) { _host = name; }  // имя устройства в сети; задать до begin()
  void setProgress(Print *out) { _out = out; }           // куда печатать точки и сообщения при подключении (например, &Serial)

  // ---- Диагностика ---------------------------------------------------------------------------
  int scan(Print &out);                                  // найденные сети с уровнем сигнала; возвращает их число
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out, const char *ssid = nullptr); // что видит плата вокруг и что не так с подключением к ssid

  // Для тестов и своих порталов
  static bool validSsid(const char *s) { return s && s[0] && strlen(s) <= 32; }
  static bool validPassword(const char *p) { return p && (p[0] == 0 || (strlen(p) >= 8 && strlen(p) <= 63)); }
  static void urlDecode(const char *in, char *out, size_t size);
  static void escapeHtml(const char *in, char *out, size_t size);

 private:
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  bool _auto = true, _wantConnected = false, _down = false;
  const char *_host = nullptr;
  Print *_out = nullptr;
  char _ssid[33] = {0}, _pass[64] = {0};
  uint32_t _lastTry = 0, _retryMs = 5000;
  bool connectTo(const char *ssid, const char *pass, uint16_t timeoutMs);
  Error classify(const char *ssid, uint8_t lastStatus);
  bool loadSaved(char *ssid, char *pass);
  // портал настройки
  bool _pending = false, _portalDone = false;
  char _pSsid[33] = {0}, _pPass[64] = {0};
  Error _portalErr = ERR_OK;
  int _portalNets = 0;
  struct Param { char id[14]; char label[33]; uint8_t maxLen; char *value; };
  Param _params[ALASH_WIFI_MAX_PARAMS];
  uint8_t _np = 0; bool _paramsLoaded = false;
  int findParam(const char *id) const;
  void loadParams();
  bool saveParams();
  void sendRoot(AlashWebServerType &server);
  void sendSaved(AlashWebServerType &server);
};

#endif  // ALASH_WIFI_H
