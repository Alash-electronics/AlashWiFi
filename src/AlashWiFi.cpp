// MIT License. Alash Electronics.
#include "AlashWiFi.h"

#if defined(ESP32) || defined(ESP8266)

#if defined(ESP32)
#include <Preferences.h>
#else
#include <EEPROM.h>
#endif

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))
#define TC(ru, kk, en) (_lang == LANG_KK ? kk : _lang == LANG_EN ? en : ru)       // то же, но обычная строка (для страниц портала)

// ---- Сохранённая сеть: Preferences (ESP32) или EEPROM (ESP8266) с контрольной суммой -----------------------

#if defined(ESP8266)
static uint8_t crc8(const uint8_t *d, size_t n) {
  uint8_t c = 0;
  while (n--) {
    c ^= *d++;
    for (uint8_t i = 0; i < 8; i++) c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x07) : (uint8_t)(c << 1);
  }
  return c;
}

static const int EE_OFFSET = 400;                        // 102 байта с 400-го: 400..501
static const int EE_SIZE = 512;
#endif

bool AlashWiFi::save(const char *ssid, const char *pass) {
  if (!validSsid(ssid) || !(pass && strlen(pass) <= 63)) {
    _err = ERR_BAD_INPUT;
    return false;
  }
#if defined(ESP32)
  Preferences p;
  if (!p.begin("alashwifi", false)) return false;
  p.putString("s", ssid);
  p.putString("p", pass);
  p.end();
#else
  uint8_t buf[2 + 1 + 1 + 32 + 63 + 1] = {0};
  buf[0] = 0xA1; buf[1] = 0xA5; buf[2] = strlen(ssid); buf[3] = strlen(pass);
  memcpy(buf + 4, ssid, buf[2]);
  memcpy(buf + 36, pass, buf[3]);
  buf[99] = crc8(buf, 99);
  EEPROM.begin(EE_SIZE);
  for (size_t i = 0; i < sizeof buf; i++) EEPROM.write(EE_OFFSET + i, buf[i]);
  EEPROM.commit();
  EEPROM.end();
#endif
  _err = ERR_OK;
  return true;
}

bool AlashWiFi::loadSaved(char *ssid, char *pass) {
#if defined(ESP32)
  Preferences p;
  if (!p.begin("alashwifi", true)) return false;
  String s = p.getString("s", ""), w = p.getString("p", "");
  p.end();
  if (!validSsid(s.c_str())) return false;
  strncpy(ssid, s.c_str(), 32); ssid[32] = 0;
  strncpy(pass, w.c_str(), 63); pass[63] = 0;
  return true;
#else
  uint8_t buf[100];
  EEPROM.begin(EE_SIZE);
  for (size_t i = 0; i < sizeof buf; i++) buf[i] = EEPROM.read(EE_OFFSET + i);
  EEPROM.end();
  if (buf[0] != 0xA1 || buf[1] != 0xA5 || buf[2] == 0 || buf[2] > 32 || buf[3] > 63 || crc8(buf, 99) != buf[99]) return false;
  memcpy(ssid, buf + 4, buf[2]); ssid[buf[2]] = 0;
  memcpy(pass, buf + 36, buf[3]); pass[buf[3]] = 0;
  return true;
#endif
}

bool AlashWiFi::hasSaved() {
  char s[33], p[64];
  return loadSaved(s, p);
}

bool AlashWiFi::savedSsid(char *out, size_t size) {
  char s[33], p[64];
  if (!loadSaved(s, p)) return false;
  strncpy(out, s, size - 1);
  out[size - 1] = 0;
  return true;
}

bool AlashWiFi::forget() {
#if defined(ESP32)
  Preferences p;
  if (!p.begin("alashwifi", false)) return false;
  p.clear();
  p.end();
#else
  EEPROM.begin(EE_SIZE);
  for (int i = 0; i < 100; i++) EEPROM.write(EE_OFFSET + i, 0xFF);
  EEPROM.commit();
  EEPROM.end();
#endif
  return true;
}

// ---- Подключение ---------------------------------------------------------------------------------------

AlashWiFi::Error AlashWiFi::classify(const char *ssid, uint8_t lastStatus) {
  if (lastStatus == WL_NO_SSID_AVAIL) return ERR_NO_NETWORK;
  if (lastStatus == WL_CONNECT_FAILED) return ERR_WRONG_PASSWORD;
#if defined(ESP8266)
  if (lastStatus == WL_WRONG_PASSWORD) return ERR_WRONG_PASSWORD;
#endif
  // Статус не ясен: смотрим, видна ли сеть вообще
  int n = WiFi.scanNetworks();
  bool seen = false;
  for (int i = 0; i < n; i++)
    if (WiFi.SSID(i) == ssid) seen = true;
  WiFi.scanDelete();
  return seen ? ERR_TIMEOUT : ERR_NO_NETWORK;
}

bool AlashWiFi::connectTo(const char *ssid, const char *pass, uint16_t timeoutMs) {
  if (!validSsid(ssid)) {
    _err = ERR_BAD_INPUT;
    return false;
  }
  WiFi.persistent(false);                                 // не писать пароль в память самой платы при каждом begin()
  if (WiFi.getMode() == WIFI_AP) WiFi.mode(WIFI_AP_STA);
  else if (WiFi.getMode() != WIFI_AP_STA) WiFi.mode(WIFI_STA);
  if (_host) {
#if defined(ESP32)
    WiFi.setHostname(_host);
#else
    WiFi.hostname(_host);
#endif
  }
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);
  uint32_t t0 = millis(), lastDot = t0;
  uint8_t st = WiFi.status();
  while ((uint32_t)(millis() - t0) < timeoutMs) {
    st = WiFi.status();
    if (st == WL_CONNECTED) break;
    if (st == WL_CONNECT_FAILED) break;
#if defined(ESP8266)
    if (st == WL_WRONG_PASSWORD) break;
#endif
    if (_out && (uint32_t)(millis() - lastDot) >= 500) {
      lastDot = millis();
      _out->print('.');
    }
    delay(50);
  }
  if (_out) _out->println();
  if (WiFi.status() == WL_CONNECTED) {
    _err = ERR_OK;
    _wantConnected = true;
    _retryMs = 5000;
    strncpy(_ssid, ssid, 32); _ssid[32] = 0;
    strncpy(_pass, pass ? pass : "", 63); _pass[63] = 0;
    return true;
  }
  _err = classify(ssid, st);
  return false;
}

bool AlashWiFi::begin(const char *ssid, const char *password, uint16_t timeoutMs) {
  return connectTo(ssid, password, timeoutMs);
}

bool AlashWiFi::beginSaved(uint16_t timeoutMs) {
  char s[33], p[64];
  if (!loadSaved(s, p)) {
    _err = ERR_NO_CREDENTIALS;
    return false;
  }
  return connectTo(s, p, timeoutMs);
}

void AlashWiFi::update() {
  if (!_auto || !_wantConnected) return;
  uint32_t now = millis();
  if (WiFi.status() == WL_CONNECTED) {
    _down = false;
    _retryMs = 5000;
    return;
  }
  if (!_down) {                                          // связь только что пропала: первая попытка через 5 секунд
    _down = true;
    _lastTry = now;
    return;
  }
  if ((uint32_t)(now - _lastTry) < _retryMs) return;
  _lastTry = now;
  WiFi.disconnect();
  WiFi.begin(_ssid, _pass);
  _retryMs = _retryMs < 60000 ? _retryMs * 2 : 60000;    // повторы реже и реже: 5, 10, 20, 40, 60 секунд
}

const __FlashStringHelper *AlashWiFi::signalText() {
  int r = WiFi.RSSI();
  if (r >= -55) return T("отличный", "тамаша", "excellent");
  if (r >= -67) return T("хороший", "жақсы", "good");
  if (r >= -75) return T("средний", "орташа", "fair");
  if (r >= -85) return T("слабый", "әлсіз", "weak");
  return T("очень слабый", "өте әлсіз", "very weak");
}

// ---- Строки и страница портала ----------------------------------------------------------------------------------

void AlashWiFi::urlDecode(const char *in, char *out, size_t size) {
  size_t o = 0;
  for (size_t i = 0; in[i] && o + 1 < size; i++) {
    if (in[i] == '+') out[o++] = ' ';
    else if (in[i] == '%' && isxdigit((unsigned char)in[i + 1]) && isxdigit((unsigned char)in[i + 2])) {
      char h[3] = {in[i + 1], in[i + 2], 0};
      out[o++] = (char)strtol(h, nullptr, 16);
      i += 2;
    } else out[o++] = in[i];
  }
  out[o] = 0;
}

void AlashWiFi::escapeHtml(const char *in, char *out, size_t size) {
  size_t o = 0;
  for (size_t i = 0; in[i]; i++) {
    const char *rep = in[i] == '<' ? "&lt;" : in[i] == '>' ? "&gt;" : in[i] == '&' ? "&amp;" : in[i] == '"' ? "&quot;" : in[i] == '\'' ? "&#39;" : nullptr;
    if (rep) {
      size_t n = strlen(rep);
      if (o + n + 1 >= size) break;
      memcpy(out + o, rep, n);
      o += n;
    } else {
      if (o + 2 >= size) break;
      out[o++] = in[i];
    }
  }
  out[o] = 0;
}

void AlashWiFi::sendRoot(AlashWebServerType &server) {
  String h;
  h.reserve(2400);
  h += F("<!doctype html><html><head><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'><title>Wi-Fi</title>"
         "<style>body{font-family:sans-serif;max-width:420px;margin:0 auto;padding:16px}input,button{width:100%;box-sizing:border-box;padding:12px;margin:6px 0;font-size:16px}"
         "button{background:#0a7;color:#fff;border:0;border-radius:6px}li{padding:8px 0;border-bottom:1px solid #ddd;cursor:pointer}.e{color:#b00}</style></head><body><h2>");
  h += TC("Подключение к Wi-Fi", "Wi-Fi-ға қосылу", "Connect to Wi-Fi");
  h += F("</h2>");
  if (_portalErr != ERR_OK) {
    h += F("<p class=e>");
    h += (const __FlashStringHelper *)errorText();
    h += F("</p>");
  }
  int n = _portalNets;
  if (n > 0) {
    h += F("<p>");
    h += TC("Нажмите на свою сеть:", "Желіңізді басыңыз:", "Tap your network:");
    h += F("</p><ul id=l>");
    for (int i = 0; i < n && i < 15; i++) {
      char esc[100];
      escapeHtml(WiFi.SSID(i).c_str(), esc, sizeof esc);
      if (!esc[0]) continue;
      h += F("<li data-s=\"");
      h += esc;
      h += F("\">");
      h += esc;
      h += F(" (");
      h += WiFi.RSSI(i);
      h += F(" dBm)</li>");
    }
    h += F("</ul>");
  }
  h += F("<form method=post action=/save><input id=s name=s maxlength=32 placeholder='");
  h += TC("Имя сети", "Желі аты", "Network name");
  h += F("' required><input name=p type=password maxlength=63 placeholder='");
  h += TC("Пароль", "Құпиясөз", "Password");
  h += F("'><button>");
  h += TC("Подключить", "Қосу", "Connect");
  h += F("</button></form><script>document.querySelectorAll('li').forEach(function(e){e.onclick=function(){document.getElementById('s').value=e.dataset.s}})</script></body></html>");
  server.send(200, "text/html; charset=utf-8", h);
}

void AlashWiFi::sendSaved(AlashWebServerType &server) {
  String h = F("<!doctype html><html><head><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'></head><body style='font-family:sans-serif;max-width:420px;margin:0 auto;padding:16px'><h2>");
  h += TC("Данные приняты", "Деректер қабылданды", "Settings received");
  h += F("</h2><p>");
  h += TC("Плата пробует подключиться к сети. Если получится, эта точка доступа выключится: подключите телефон обратно к своему Wi-Fi. Если не получится, подключитесь к точке доступа платы снова и введите данные ещё раз.",
          "Тақта желіге қосылып көреді. Сәтті болса, бұл нүкте өшеді: телефонды өз Wi-Fi-ыңызға қайта қосыңыз. Болмаса, тақта нүктесіне қайта қосылып, деректерді қайта енгізіңіз.",
          "The board is trying to connect. If it works, this access point turns off: reconnect your phone to your own Wi-Fi. If not, join the board's access point again and re-enter the details.");
  h += F("</p></body></html>");
  server.send(200, "text/html; charset=utf-8", h);
}

// ---- Портал настройки: точка доступа + DNS-перехват + страница ---------------------------------------------------------

bool AlashWiFi::startPortal(const char *apName, const char *apPassword, uint16_t timeoutSec) {
  AlashWebServerType server(80);
  DNSServer dns;
  _portalDone = false;
  _pending = false;
  _portalErr = ERR_OK;

  WiFi.mode(WIFI_AP_STA);
  bool apOk = (apPassword && apPassword[0]) ? WiFi.softAP(apName, apPassword) : WiFi.softAP(apName);
  delay(200);
  IPAddress ap = WiFi.softAPIP();
  dns.start(53, "*", ap);                                   // любой адрес в браузере ведёт на нашу страницу
  if (_out) {
    _out->print(TC("Откройте на телефоне Wi-Fi сеть ", "Телефонда Wi-Fi желісін ашыңыз ", "On your phone join Wi-Fi network "));
    _out->print(apName);
    _out->print(TC(" и страницу http://", " және http:// бетін ", " and open http://"));
    _out->println(ap);
  }
  _portalNets = WiFi.scanNetworks();

  server.on("/", [&]() { sendRoot(server); });
  server.on("/save", [&]() {
    char ssid[33], pass[64];
    urlDecode(server.arg("s").c_str(), ssid, sizeof ssid);
    urlDecode(server.arg("p").c_str(), pass, sizeof pass);
    if (!validSsid(ssid) || !validPassword(pass)) {
      _portalErr = ERR_BAD_INPUT;
      _err = ERR_BAD_INPUT;
      sendRoot(server);
      return;
    }
    strncpy(_pSsid, ssid, 32); _pSsid[32] = 0;
    strncpy(_pPass, pass, 63); _pPass[63] = 0;
    _pending = true;
    sendSaved(server);                                      // ответ уходит телефону до того, как радио начнёт искать роутер
  });
  server.onNotFound([&]() {                                 // системные проверки телефонов (captive portal) тоже попадают сюда
    server.sendHeader("Location", String("http://") + ap.toString(), true);
    server.send(302, "text/plain", "");
  });
  server.begin();

  uint32_t t0 = millis();
  bool ok = false;
  while (!ok && (uint32_t)(millis() - t0) < (uint32_t)timeoutSec * 1000UL) {
    dns.processNextRequest();
    server.handleClient();
    if (_pending) {
      _pending = false;
      delay(300);                                           // дать ответу уйти
      if (connectTo(_pSsid, _pPass, 15000)) {
        save(_pSsid, _pPass);
        ok = true;
      } else {
        _portalErr = _err;                                  // причина будет показана на странице при следующем заходе
      }
    }
    delay(2);
  }
  server.close();
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  if (!ok) _err = (_portalErr != ERR_OK) ? _portalErr : ERR_PORTAL_TIMEOUT;
  else _err = ERR_OK;
  (void)apOk;
  return ok;
}

bool AlashWiFi::autoConnect(const char *apName, const char *apPassword, uint16_t portalTimeoutSec, uint16_t connectTimeoutMs) {
  if (hasSaved() && beginSaved(connectTimeoutMs)) return true;
  return startPortal(apName, apPassword, portalTimeoutSec);
}

// ---- Диагностика --------------------------------------------------------------------------------------------------

int AlashWiFi::scan(Print &out) {
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  out.print(T("Найдено сетей: ", "Табылған желілер: ", "Networks found: "));
  out.println(n);
  for (int i = 0; i < n; i++) {
    out.print(F("  "));
    out.print(WiFi.SSID(i));
    out.print(F("  "));
    out.print(WiFi.RSSI(i));
    out.print(F(" dBm  ch "));
    out.print(WiFi.channel(i));
#if defined(ESP32)
    out.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? F("  open") : F("  secured"));
#else
    out.println(WiFi.encryptionType(i) == ENC_TYPE_NONE ? F("  open") : F("  secured"));
#endif
  }
  WiFi.scanDelete();
  return n;
}

const __FlashStringHelper *AlashWiFi::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_NETWORK:
      return T("Сеть не найдена. Проверьте имя сети (регистр букв важен), приблизьтесь к роутеру. ESP видит только Wi-Fi 2.4 ГГц, сеть 5 ГГц не подойдёт",
               "Желі табылмады. Желі атын (әріп регистрі маңызды) тексеріңіз, роутерге жақындаңыз. ESP тек 2.4 ГГц Wi-Fi көреді, 5 ГГц жарамайды",
               "The network was not found. Check the network name (letter case matters) and get closer to the router. ESP sees only 2.4 GHz Wi-Fi, a 5 GHz network will not work");
    case ERR_WRONG_PASSWORD:
      return T("Пароль не подошёл. Проверьте пароль (регистр букв, раскладка, пробелы в конце)",
               "Құпиясөз сәйкес келмеді. Құпиясөзді (әріп регистрі, пернетақта тілі, соңындағы бос орын) тексеріңіз",
               "The password was rejected. Check it (letter case, keyboard layout, trailing spaces)");
    case ERR_TIMEOUT:
      return T("Подключиться не удалось за отведённое время: сеть видна, но слабый сигнал или роутер перегружен / фильтрует устройства (MAC). Приблизьтесь к роутеру и повторите",
               "Белгіленген уақытта қосыла алмады: желі көрінеді, бірақ сигнал әлсіз не роутер жүктелген / құрылғыларды сүзеді (MAC). Роутерге жақындаңыз",
               "Could not connect in time: the network is visible but the signal is weak or the router is overloaded / filters devices (MAC). Move closer to the router and retry");
    case ERR_NO_CREDENTIALS:
      return T("Сохранённой сети нет: сначала подключитесь через begin(имя, пароль) или портал autoConnect()",
               "Сақталған желі жоқ: алдымен begin(аты, құпиясөз) не autoConnect() порталы арқылы қосылыңыз",
               "There is no saved network: connect first with begin(name, password) or the autoConnect() portal");
    case ERR_PORTAL_TIMEOUT:
      return T("Портал настройки закрыт по времени: никто не ввёл данные сети", "Баптау порталы уақыт бойынша жабылды: желі деректерін ешкім енгізбеді",
               "The setup portal closed on timeout: nobody entered the network details");
    case ERR_BAD_INPUT:
      return T("Имя сети от 1 до 32 символов, пароль пустой или от 8 до 63 символов", "Желі аты 1-32 таңба, құпиясөз бос не 8-63 таңба",
               "The network name is 1 to 32 characters, the password is empty or 8 to 63 characters");
  }
  return F("");
}

void AlashWiFi::diagnose(Print &out, const char *ssid) {
  out.println(T("AlashWiFi: диагностика", "AlashWiFi: диагностика", "AlashWiFi: diagnostics"));
  out.print(T("Состояние: ", "Күйі: ", "State: "));
  out.println(isConnected() ? T("подключён", "қосылған", "connected") : T("не подключён", "қосылмаған", "not connected"));
  if (isConnected()) {
    out.print(F("IP: ")); out.println(WiFi.localIP());
    out.print(T("Сеть: ", "Желі: ", "Network: ")); out.println(WiFi.SSID());
    out.print(T("Сигнал: ", "Сигнал: ", "Signal: ")); out.print(WiFi.RSSI()); out.print(F(" dBm, ")); out.println(signalText());
  }
  int n = scan(out);
  if (ssid) {
    bool seen = false;
    int n2 = WiFi.scanNetworks();
    for (int i = 0; i < n2; i++)
      if (WiFi.SSID(i) == ssid) seen = true;
    WiFi.scanDelete();
    out.print(T("Сеть «", "Желі «", "Network \""));
    out.print(ssid);
    out.println(seen ? T("» видна", "» көрінеді", "\" is visible") : T("» НЕ видна: проверьте имя, расстояние, диапазон 2.4 ГГц", "» КӨРІНБЕЙДІ: атын, қашықтықты, 2.4 ГГц диапазонын тексеріңіз", "\" is NOT visible: check the name, distance and the 2.4 GHz band"));
  } else if (n == 0) {
    out.println(T("Сетей не видно совсем: проверьте антенну и питание платы (USB-кабель), поднесите плату к окну или роутеру",
                  "Желілер мүлде көрінбейді: антенна мен тақта қуатын (USB кабелі) тексеріңіз",
                  "No networks at all: check the antenna and the board power (USB cable), move it near a window or the router"));
  }
  out.println(T("Подсказки: ESP32 и ESP8266 работают только с Wi-Fi 2.4 ГГц. Пароль хотя бы 8 символов. Часть школьных сетей требует вход через страницу (captive portal): для проекта раздайте интернет с телефона.",
                "Кеңестер: ESP32 және ESP8266 тек 2.4 ГГц Wi-Fi-мен жұмыс істейді. Құпиясөз кемінде 8 таңба. Мектеп желілерінің бір бөлігі бет арқылы кіруді талап етеді: жоба үшін телефоннан интернет таратыңыз.",
                "Tips: ESP32 and ESP8266 work only with 2.4 GHz Wi-Fi. The password has at least 8 characters. Some school networks require a web login (captive portal): use a phone hotspot for projects."));
}

#endif  // ESP32 || ESP8266
