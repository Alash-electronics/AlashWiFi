// Тесты AlashWiFi без железа: имитация WiFi, WebServer, DNS и Preferences (как на ESP32), время вручную.
//   g++ -std=c++17 -Wall -DESP32 -I mock -I ../../src test_wifi.cpp ../../src/AlashWiFi.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "DNSServer.h"
#include "Preferences.h"
#include "AlashWiFi.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static bool has(const std::string &hay, const char *needle) { return hay.find(needle) != std::string::npos; }
static void reset() {
  WiFi = WiFiClass(); g_prefs.clear(); g_httpQueue.clear(); g_httpResp.clear(); g_onDelay = nullptr; g_ms += 100000;
  FakeAp home; home.ssid = "Home-2G"; home.pass = "secret123"; home.rssi = -58;
  FakeAp cafe; cafe.ssid = "Cafe <Free>"; cafe.pass = ""; cafe.open = true; cafe.rssi = -80;
  WiFi.aps = {home, cafe};
}

// "Пользователь" портала: в нужные моменты присылает запросы
static uint32_t t0 = 0; static int stage = 0; static bool goodSecond = true;
static void user() {
  uint32_t t = g_ms - t0;
  if (stage == 0 && t > 500) { g_httpQueue.push_back({"/", {}}); stage = 1; }
  if (stage == 1 && t > 1500) { g_httpQueue.push_back({"/generate_204", {}}); stage = 2; }       // проверка captive portal телефоном
  if (stage == 2 && t > 2500) { g_httpQueue.push_back({"/save", {{"s", "short"}, {"p", "abc"}}}); stage = 3; }   // пароль слишком короткий
  if (stage == 3 && t > 3500) { g_httpQueue.push_back({"/save", {{"s", "Home-2G"}, {"p", "wrongpass1"}}}); stage = 4; }   // неверный пароль
  if (stage == 4 && t > 30000) { g_httpQueue.push_back({"/", {}}); stage = 5; }                  // после неудачной попытки страница показывает причину
  if (stage == 5 && t > 31000) { g_httpQueue.push_back({"/save", {{"s", "Home-2G"}, {"p", goodSecond ? "secret123" : "nope"}}}); stage = 6; }
}

int main() {
  // 1) вспомогательные функции
  CHECK(AlashWiFi::validSsid("Home") && !AlashWiFi::validSsid("") && !AlashWiFi::validSsid(nullptr) && !AlashWiFi::validSsid("123456789012345678901234567890123"));
  CHECK(AlashWiFi::validPassword("") && AlashWiFi::validPassword("12345678") && !AlashWiFi::validPassword("1234567") && !AlashWiFi::validPassword(nullptr));
  char buf[80];
  AlashWiFi::urlDecode("My+Net%21%2Fa", buf, sizeof buf); CHECK(strcmp(buf, "My Net!/a") == 0);
  AlashWiFi::urlDecode("%D0%BF%D1%80", buf, sizeof buf); CHECK(strcmp(buf, "пр") == 0);
  AlashWiFi::urlDecode("100%", buf, sizeof buf); CHECK(strcmp(buf, "100%") == 0);
  AlashWiFi::escapeHtml("a<b>&\"c'", buf, sizeof buf); CHECK(strcmp(buf, "a&lt;b&gt;&amp;&quot;c&#39;") == 0);
  AlashWiFi::escapeHtml("aaaaaaaaaa", buf, 6); CHECK(strlen(buf) <= 5);

  // 2) успешное подключение: IP, точки прогресса, имя устройства, режим STA
  reset(); AlashWiFi w; Print log; w.setProgress(&log); w.setHostname("alash-robot");
  CHECK(w.begin("Home-2G", "secret123", 15000) && w.isConnected() && w.error() == AlashWiFi::ERR_OK);
  CHECK(has(log.log, "...") && WiFi.lastHost == "alash-robot" && WiFi.md == WIFI_STA && WiFi.autoRec);
  CHECK(w.ip().toString() == String("192.168.1.50"));

  // 3) причины неудач: неверный пароль (быстро), сети нет, сеть видна но не подключается, пустое имя
  reset(); AlashWiFi p;
  uint32_t t = g_ms; CHECK(!p.begin("Home-2G", "badpassword", 15000) && p.error() == AlashWiFi::ERR_WRONG_PASSWORD && g_ms - t < 5000);
  reset(); AlashWiFi n; CHECK(!n.begin("Unknown", "12345678", 5000) && n.error() == AlashWiFi::ERR_NO_NETWORK);
  reset(); WiFi.aps[0].neverConnects = true; AlashWiFi s; CHECK(!s.begin("Home-2G", "secret123", 4000) && s.error() == AlashWiFi::ERR_TIMEOUT);
  reset(); AlashWiFi e; CHECK(!e.begin("", "x") && e.error() == AlashWiFi::ERR_BAD_INPUT);
  CHECK(e.begin("Cafe <Free>", "") );                                  // открытая сеть без пароля

  // 4) сохранение: save / hasSaved / savedSsid / beginSaved / forget
  reset(); AlashWiFi sv;
  CHECK(!sv.hasSaved() && !sv.beginSaved() && sv.error() == AlashWiFi::ERR_NO_CREDENTIALS);
  CHECK(!sv.save("", "x") && sv.error() == AlashWiFi::ERR_BAD_INPUT);
  CHECK(sv.save("Home-2G", "secret123") && sv.hasSaved());
  char name[33]; CHECK(sv.savedSsid(name, sizeof name) && strcmp(name, "Home-2G") == 0);
  CHECK(sv.beginSaved(15000) && sv.isConnected());
  CHECK(sv.forget() && !sv.hasSaved());

  // 5) автопереподключение: связь пропала, попытки через 5, 10, 20 секунд (удвоение), потом успех
  reset(); AlashWiFi r; r.begin("Home-2G", "secret123");
  WiFi.dropConnection(); WiFi.aps[0].neverConnects = true; int b0 = WiFi.begins;       // роутер ушёл: попытки не удаются
  uint32_t t1 = g_ms; int attempts = 0; uint32_t first = 0, second = 0;
  for (int i = 0; i < 120000; i += 10) {
    g_ms += 10; int before = WiFi.begins; r.update();
    if (WiFi.begins != before) { attempts++; if (attempts == 1) first = g_ms - t1; if (attempts == 2) second = g_ms - t1; if (attempts == 3) WiFi.aps[0].neverConnects = false; }               // на третьей попытке роутер вернулся
  }
  CHECK(attempts >= 3 && first >= 4990 && first <= 5100 && second - first >= 9990 && second - first <= 10100);
  CHECK(WiFi.begins - b0 == attempts);
  CHECK(r.isConnected());                                                                  // после возвращения роутера снова на связи
  reset(); AlashWiFi off; off.setAutoReconnect(false); off.begin("Home-2G", "secret123"); WiFi.dropConnection(); int b1 = WiFi.begins;
  for (int i = 0; i < 20000; i += 10) { g_ms += 10; off.update(); }
  CHECK(WiFi.begins == b1);                                           // отключено вручную: не лезет

  // 6) уровень сигнала словами
  reset(); AlashWiFi q;
  WiFi.rssiNow = -50; CHECK(strcmp((const char *)q.signalText(), "отличный") == 0);
  WiFi.rssiNow = -65; CHECK(strcmp((const char *)q.signalText(), "хороший") == 0);
  WiFi.rssiNow = -72; CHECK(strcmp((const char *)q.signalText(), "средний") == 0);
  WiFi.rssiNow = -80; CHECK(strcmp((const char *)q.signalText(), "слабый") == 0);
  WiFi.rssiNow = -90; q.setLanguage(AlashWiFi::LANG_EN); CHECK(strcmp((const char *)q.signalText(), "very weak") == 0);

  // 7) портал: страница со списком сетей (с экранированием), captive-редирект, ошибка ввода, неверный пароль, затем верный
  reset(); AlashWiFi pt; Print plog; pt.setProgress(&plog);
  t0 = g_ms; stage = 0; goodSecond = true; g_onDelay = user;
  bool ok = pt.startPortal("AlashSetup", "", 120);
  CHECK(ok && pt.error() == AlashWiFi::ERR_OK && pt.isConnected());
  CHECK(WiFi.apName == "AlashSetup" && WiFi.apDisconnected && WiFi.md == WIFI_STA);   // точка доступа выключена, режим клиента
  CHECK(DNSServer::processed > 0);
  CHECK(g_httpResp.size() >= 5);
  CHECK(g_httpResp[0].code == 200 && has(g_httpResp[0].body, "Home-2G") && has(g_httpResp[0].body, "Cafe &lt;Free&gt;") && !has(g_httpResp[0].body, "Cafe <Free>"));   // имена сетей экранированы
  CHECK(has(g_httpResp[0].body, "Подключение к Wi-Fi") && has(g_httpResp[0].body, "name=p type=password"));
  CHECK(g_httpResp[1].code == 302 && g_httpResp[1].location == "http://192.168.4.1");   // проверка captive portal ведёт на страницу
  CHECK(has(g_httpResp[2].body, "Имя сети от 1 до 32"));                                  // слишком короткий пароль: понятное сообщение
  CHECK(has(g_httpResp[3].body, "Данные приняты"));
  CHECK(has(g_httpResp[4].body, "Пароль не подошёл"));                                    // после неудачной попытки страница объясняет причину
  char saved[33]; CHECK(pt.savedSsid(saved, sizeof saved) && strcmp(saved, "Home-2G") == 0);
  CHECK(has(plog.log, "AlashSetup") && has(plog.log, "192.168.4.1"));

  // 8) портал без ввода закрывается по времени
  reset(); AlashWiFi to; to.setProgress(nullptr);
  uint32_t ts = g_ms; CHECK(!to.startPortal("X", "", 20) && to.error() == AlashWiFi::ERR_PORTAL_TIMEOUT && g_ms - ts >= 20000 && !WiFi.apUp);

  // 9) autoConnect: сохранённая сеть без портала, иначе портал
  reset(); AlashWiFi a1; a1.save("Home-2G", "secret123");
  CHECK(a1.autoConnect("AlashSetup", "", 30) && !WiFi.apDisconnected && WiFi.apName.empty());
  reset(); AlashWiFi a2; a2.save("Home-2G", "oldpassword");
  t0 = g_ms; stage = 5; g_onDelay = user; goodSecond = true;                           // сохранённый пароль устарел, портал сразу, "пользователь" вводит верный
  CHECK(a2.autoConnect("AlashSetup", "", 100) && WiFi.apName == "AlashSetup");
  reset(); AlashWiFi a3; CHECK(!a3.autoConnect("AlashSetup", "", 10) && a3.error() == AlashWiFi::ERR_PORTAL_TIMEOUT && WiFi.apName == "AlashSetup");

  // 10) сканирование и диагностика
  reset(); AlashWiFi d; Print o;
  CHECK(d.scan(o) == 2 && has(o.log, "Home-2G") && has(o.log, "-58 dBm") && has(o.log, "open") && has(o.log, "secured"));
  Print o2; d.diagnose(o2, "Home-2G"); CHECK(has(o2.log, "видна") && !has(o2.log, "НЕ видна"));
  Print o3; d.diagnose(o3, "Ghost"); CHECK(has(o3.log, "НЕ видна"));
  reset(); WiFi.aps.clear(); Print o4; d.diagnose(o4); CHECK(has(o4.log, "Сетей не видно совсем"));
  d.setLanguage(AlashWiFi::LANG_KK); CHECK(strlen((const char *)d.errorText()) == 2);   // ERR_OK: "OK"

  // 11) свои поля портала: проверка id, значения по умолчанию, сохранение
  {
    reset(); AlashWiFi f;
    CHECK(f.addParam("mqtt", "MQTT сервер", "broker.local", 40) && f.addParam("token", "Токен", "", 20));
    CHECK(!f.addParam("", "x") && !f.addParam("bad id", "x") && !f.addParam("has-dash", "x") && !f.addParam("averyveryverylongid", "x") && f.error() == AlashWiFi::ERR_BAD_PARAM);
    f.clearError(); CHECK(!f.addParam("mqtt", "dup") && f.error() == AlashWiFi::ERR_BAD_PARAM);
    f.clearError(); CHECK(!f.addParam("long", "x", "this default is too long", 5) && !f.addParam("zero", "x", "", 0));
    CHECK(f.addParam("a", "a") && f.addParam("b", "b") && f.addParam("c", "c") && f.addParam("d", "d") && f.paramCount() == 6);
    f.clearError(); CHECK(!f.addParam("seventh", "7") && f.error() == AlashWiFi::ERR_BAD_PARAM);   // больше ALASH_WIFI_MAX_PARAMS
    CHECK(strcmp(f.param("mqtt"), "broker.local") == 0 && strcmp(f.param("token"), "") == 0 && strcmp(f.param("nope"), "") == 0 && strcmp(f.param(nullptr), "") == 0);
    AlashWiFi big; CHECK(big.addParam("p1", "p", "", 200) && big.addParam("p2", "p", "", 200) && !big.addParam("p3", "p", "", 100));   // 480 байт на все поля

    // setParam сохраняет и переживает «перезагрузку»
    CHECK(f.setParam("token", "abc123") && !f.setParam("token", "this token is far too long for twenty") && !f.setParam("zzz", "x"));
    AlashWiFi f2; f2.addParam("mqtt", "MQTT", "other.default", 40); f2.addParam("token", "T", "", 20);
    CHECK(strcmp(f2.param("token"), "abc123") == 0 && strcmp(f2.param("mqtt"), "broker.local") == 0);   // сохраняются все поля сразу: сохранённое значение важнее defaults нового объекта
    f2.save("Home-2G", "secret123"); f2.forget(); AlashWiFi f3; f3.addParam("token", "T", "", 20);
    CHECK(strcmp(f3.param("token"), "abc123") == 0);                                           // forget() забывает сеть, но не поля
  }

  // 12) портал с полями: форма, экранирование, ввод, сохранение; потом правка только полей при сохранённой сети
  {
    reset(); AlashWiFi pp; Print lg; pp.setProgress(&lg);
    pp.addParam("mqtt", "Адрес <MQTT>", "br\"oker", 30); pp.addParam("name", "Имя", "robot1", 12);
    t0 = g_ms; stage = 0; g_onDelay = []() {
      uint32_t t = g_ms - t0;
      if (stage == 0 && t > 500) { g_httpQueue.push_back({"/", {}}); stage = 1; }
      if (stage == 1 && t > 1500) { g_httpQueue.push_back({"/save", {{"s", "Home-2G"}, {"p", "secret123"}, {"q0", "mqtt.example.kz"}, {"q1", "this value is much too long"}}}); stage = 2; }   // q1 длиннее 12
      if (stage == 2 && t > 2500) { g_httpQueue.push_back({"/save", {{"s", "Home-2G"}, {"p", "secret123"}, {"q0", "mqtt.example.kz"}, {"q1", "rob%C3%B3+2"}}}); stage = 3; }
    };
    CHECK(pp.startPortal("AlashSetup", "", 60) && pp.isConnected());
    CHECK(g_httpResp.size() >= 3);
    CHECK(has(g_httpResp[0].body, "Адрес &lt;MQTT&gt;") && !has(g_httpResp[0].body, "Адрес <MQTT>"));      // подписи экранированы
    CHECK(has(g_httpResp[0].body, "name=q0 maxlength=30 value=\"br&quot;oker\"") && has(g_httpResp[0].body, "name=q1 maxlength=12 value=\"robot1\""));
    CHECK(has(g_httpResp[0].body, "required"));                                                             // сеть ещё не сохранена: имя обязательно
    CHECK(has(g_httpResp[1].body, "Имя сети от 1 до 32"));                                                  // слишком длинное значение отвергнуто
    CHECK(has(g_httpResp[2].body, "Данные приняты"));
    CHECK(strcmp(pp.param("mqtt"), "mqtt.example.kz") == 0 && strcmp(pp.param("name"), "robó 2") == 0);
    AlashWiFi after; after.addParam("mqtt", "x", "", 30); after.addParam("name", "y", "", 12);
    CHECK(strcmp(after.param("mqtt"), "mqtt.example.kz") == 0 && strcmp(after.param("name"), "robó 2") == 0);   // после перезагрузки значения на месте

    // сеть сохранена: имя не обязательно, пустое имя оставляет сохранённую сеть и меняет только поля
    g_httpQueue.clear(); g_httpResp.clear(); t0 = g_ms; stage = 0;
    g_onDelay = []() {
      uint32_t t = g_ms - t0;
      if (stage == 0 && t > 500) { g_httpQueue.push_back({"/", {}}); stage = 1; }
      if (stage == 1 && t > 1500) { g_httpQueue.push_back({"/save", {{"s", ""}, {"p", ""}, {"q0", "new.host"}, {"q1", "robot9"}}}); stage = 2; }
    };
    WiFi.dropConnection();
    CHECK(after.startPortal("AlashSetup", "", 60) && after.isConnected());
    CHECK(!has(g_httpResp[0].body, " required") && has(g_httpResp[0].body, "value=\"mqtt.example.kz\""));
    CHECK(strcmp(after.param("mqtt"), "new.host") == 0);
    char nm[33]; CHECK(after.savedSsid(nm, sizeof nm) && strcmp(nm, "Home-2G") == 0);                     // сеть та же
    AlashWiFi again; again.addParam("mqtt", "x", "", 30); CHECK(strcmp(again.param("mqtt"), "new.host") == 0);

    // diagnose показывает поля
    Print dg; after.diagnose(dg); CHECK(has(dg.log, "Поля портала") && has(dg.log, "mqtt=new.host"));
  }

  // 13) формат хранения для ESP8266: упаковка, проверка суммы, смена набора полей
  {
    char v1[11] = "hello", v2[6] = "wifi"; char *vals[2] = {v1, v2}; uint8_t lens[2] = {10, 5};
    uint8_t buf[64]; size_t n = AlashWiFi::packParams(buf, sizeof buf, vals, lens, 2);
    CHECK(n == 4 + 11 + 6 && buf[0] == 0xA1 && buf[2] == 2);
    char r1[11] = "", r2[6] = ""; char *rv[2] = {r1, r2};
    CHECK(AlashWiFi::unpackParams(buf, n, rv, lens, 2) && strcmp(r1, "hello") == 0 && strcmp(r2, "wifi") == 0);
    uint8_t bad[64]; memcpy(bad, buf, n); bad[6] ^= 0x10;
    CHECK(!AlashWiFi::unpackParams(bad, n, rv, lens, 2));                                       // порча: сумма не сошлась
    uint8_t other[2] = {10, 6}; CHECK(AlashWiFi::unpackParams(buf, n, rv, other, 2) == false);   // длины полей другие: сумма или размер не подходят
    CHECK(!AlashWiFi::unpackParams(buf, n, rv, lens, 1));                                       // другое число полей
    uint8_t blank[64]; memset(blank, 0xFF, sizeof blank); CHECK(!AlashWiFi::unpackParams(blank, n, rv, lens, 2));   // чистая память
    CHECK(AlashWiFi::packParams(buf, 10, vals, lens, 2) == 0);                                   // не помещается
    AlashWiFi q; q.addParam("a", "a", "", 4); q.addParam("a2", "a", "", 4); q.setLanguage(AlashWiFi::LANG_KK);
    q.addParam("a", "dup"); const char *kk = (const char *)q.errorText(); q.setLanguage(AlashWiFi::LANG_EN); const char *en = (const char *)q.errorText();
    q.setLanguage(AlashWiFi::LANG_RU); const char *ru = (const char *)q.errorText();
    CHECK(q.error() == AlashWiFi::ERR_BAD_PARAM && strlen(ru) > 20 && strcmp(ru, kk) && strcmp(kk, en));
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
