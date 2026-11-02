# AlashWiFi

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для подключения **ESP32 и ESP8266 к Wi-Fi**: `wifi.begin(имя, пароль)` и **понятная причина**, если не вышло («сеть не найдена, плата видит только 2.4 ГГц», «пароль не подошёл»). Умеет **настройку с телефона** (как WiFiManager): плата сама создаёт сеть, вы вводите имя и пароль на странице, плата запоминает и подключается сама. Переподключается при потере связи. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashWiFi.h>
AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);
  if (wifi.autoConnect("AlashSetup")) {      // сохранённая сеть или настройка с телефона
    Serial.println(wifi.ip());
  } else {
    Serial.println(wifi.errorText());
  }
}
void loop() { wifi.update(); }               // переподключение само / reconnects by itself
```

### Что умеет
- **Понятные причины:** `errorText()` различает «сеть не найдена» (имя, расстояние, 5 ГГц), «пароль не подошёл» и «слабый сигнал / роутер не принял».
- **Настройка с телефона:** `autoConnect("AlashSetup")`: плата создаёт сеть, страница на телефоне показывает список сетей вокруг, вы нажимаете на свою и вводите пароль. Страница на русском, казахском или английском.
- **Запоминает сеть:** `save()`, `hasSaved()`, `forget()`; в памяти платы с контрольной суммой, пароль наружу не отдаётся.
- **Свои поля на странице настройки:** `addParam("mqtt", "Адрес MQTT", "broker.local")`, значение потом в `param("mqtt")`; можно поправить только поля, не вводя сеть заново.
- **Переподключается само:** `update()` в `loop()`; паузы растут 5, 10, 20, 40, 60 секунд, чтобы не донимать роутер.
- **Защита от неверного ввода:** имя 1-32 символа, пароль пустой или 8-63; имена сетей на странице экранируются.
- **Сканер:** `scan(Serial)` показывает сети, сигнал, канал, защищена ли; `diagnose(Serial, имя)` говорит, видна ли ваша.
- **Сигнал словами:** `signalText()` («отличный», «слабый»).
- Для **ESP32 и ESP8266**.

### Чем отличается от других библиотек
Мы посмотрели WiFiManager (tzapu), WiFiMulti и примеры из курсов:
- WiFiManager большой, требует ArduinoJson в части версий, и у него английские сообщения; у нас короткая библиотека на трёх языках без зависимостей.
- В стандартных примерах `while (WiFi.status() != WL_CONNECTED)` крутится вечно и ни слова о причине. У нас таймаут и причина.
- Нет подсказки про 2.4 ГГц, самую частую ошибку школьников: у нас она в тексте ошибки и в `diagnose()`.

### Примеры
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

### Тесты
Подключение, неверный пароль (быстро), сеть не найдена, сеть видна но не подключается, сохранение и забывание сети, автопереподключение с растущими паузами, портал настройки (страница со списком сетей и экранированием, captive-перенаправление, ошибка ввода, неверный и верный пароль, закрытие по времени, выключение точки доступа), autoConnect проверены без железа на имитации WiFi, WebServer, DNS и Preferences: `extras/test`. Версия 1.1.0: свои поля портала проверены так же (проверка id, повторы, лимиты, форма с экранированием, ввод и сохранение, правка только полей при сохранённой сети, формат хранения ESP8266 с контрольной суммой и защитой от смены набора полей).

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**ESP32 және ESP8266-ны Wi-Fi-ға** қосуға арналған қарапайым кітапхана: `wifi.begin(аты, құпиясөз)` және сәтсіз болса **түсінікті себеп**. Телефоннан **баптауды** (WiFiManager сияқты) біледі: тақта желі жасайды, сіз бетте аты мен құпиясөзді енгізесіз, тақта есте сақтап, өзі қосылады. Байланыс үзілсе қайта қосылады. Тәуелділіксіз.

```cpp
#include <AlashWiFi.h>
AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);
  if (wifi.autoConnect("AlashSetup")) {      // сохранённая сеть или настройка с телефона
    Serial.println(wifi.ip());
  } else {
    Serial.println(wifi.errorText());
  }
}
void loop() { wifi.update(); }               // переподключение само / reconnects by itself
```

### Мүмкіндіктері
- **Түсінікті себептер:** `errorText()` «желі табылмады», «құпиясөз сәйкес емес», «сигнал әлсіз» айырады.
- **Телефоннан баптау:** `autoConnect("AlashSetup")`: тақта желі жасайды, телефондағы бет айналадағы желілерді көрсетеді.
- **Желіні есте сақтайды:** `save()`, `hasSaved()`, `forget()`.
- **Баптау бетіндегі өз өрістері:** `addParam("mqtt", "MQTT мекенжайы", "broker.local")`, мән кейін `param("mqtt")` арқылы; желіні қайта енгізбей тек өрістерді түзетуге болады.
- **Өзі қайта қосылады:** `update()`; кідірістер 5, 10, 20, 40, 60 секунд.
- **Қате енгізуден қорғау:** аты 1-32 таңба, құпиясөз бос не 8-63.
- **Сканер:** `scan(Serial)`; `diagnose(Serial, аты)`.
- **Сигнал сөзбен:** `signalText()`.
- **ESP32 және ESP8266**.

### Басқа кітапханалардан айырмашылығы
WiFiManager (tzapu), WiFiMulti және курс мысалдарын қарадық:
- WiFiManager үлкен, ағылшынша хабарлар; бізде үш тілде қысқа кітапхана.
- Стандартты мысалдарда `while (WiFi.status() != WL_CONNECTED)` мәңгі айналады; бізде уақыт шегі мен себеп.
- 2.4 ГГц туралы кеңес бізде.

### Мысалдар
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

### Тесттер
Қосылу, қате құпиясөз, желі табылмауы, сақтау, автоқосылу, баптау порталы, autoConnect тексерілген: `extras/test`. 1.1.0 нұсқасы: порталдың өз өрістері де осылай тексерілген.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library to connect an **ESP32 and ESP8266 to Wi-Fi**: `wifi.begin(name, password)` and a **clear reason** when it fails ("network not found, the board sees only 2.4 GHz", "the password was rejected"). It can do **phone-based setup** (like WiFiManager): the board creates a network, you enter the name and password on a page, the board remembers it and connects by itself. It reconnects when the link drops. No dependencies. Made for classes and Alash Electronics kits.

```cpp
#include <AlashWiFi.h>
AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);
  if (wifi.autoConnect("AlashSetup")) {      // сохранённая сеть или настройка с телефона
    Serial.println(wifi.ip());
  } else {
    Serial.println(wifi.errorText());
  }
}
void loop() { wifi.update(); }               // переподключение само / reconnects by itself
```

### Features
- **Clear reasons:** `errorText()` tells "network not found" (name, distance, 5 GHz), "the password was rejected" and "weak signal / the router refused".
- **Phone setup:** `autoConnect("AlashSetup")`: the board creates a network, a page on the phone lists the networks around, you tap yours and type the password. The page is in Russian, Kazakh or English.
- **Remembers the network:** `save()`, `hasSaved()`, `forget()`; stored with a checksum, the password is never handed back.
- **Your own fields on the setup page:** `addParam("mqtt", "MQTT address", "broker.local")`, the value is then in `param("mqtt")`; you can change only the fields without re-entering the network.
- **Reconnects by itself:** `update()` in `loop()`; the delays grow 5, 10, 20, 40, 60 seconds so the router is not pestered.
- **Input protection:** name 1-32 characters, password empty or 8-63; network names on the page are HTML-escaped.
- **Scanner:** `scan(Serial)` shows networks, signal, channel and whether secured; `diagnose(Serial, name)` says if yours is visible.
- **Signal in words:** `signalText()` ("excellent", "weak").
- For **ESP32 and ESP8266**.

### How it differs from other libraries
We looked at WiFiManager (tzapu), WiFiMulti and course examples:
- WiFiManager is large, needs ArduinoJson in some versions and shows English-only messages; ours is a short, three-language library with no dependencies.
- In standard examples `while (WiFi.status() != WL_CONNECTED)` spins forever with no word about the reason. Ours has a timeout and a reason.
- No tip about 2.4 GHz, the most common student mistake; ours is in the error text and in `diagnose()`.

### Examples
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

### Tests
Connecting, a wrong password (fast), network not found, visible but not connecting, saving and forgetting a network, auto-reconnect with growing delays, the setup portal (page with an escaped network list, captive redirect, input errors, wrong then right password, timeout, access point shutdown) and autoConnect are checked without hardware on simulated WiFi, WebServer, DNS and Preferences: `extras/test`. Version 1.1.0: the portal's own fields are checked the same way (id validation, duplicates, limits, a form with escaping, input and saving, editing only the fields when a network is saved, the ESP8266 storage format with checksum and protection from a changed field set).

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
