# AlashWiFi: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для подключения **ESP32 и ESP8266 к Wi-Fi**: `wifi.begin(имя, пароль)` и **понятная причина**, если не вышло («сеть не найдена, плата видит только 2.4 ГГц», «пароль не подошёл»). Умеет **настройку с телефона** (как WiFiManager): плата сама создаёт сеть, вы вводите имя и пароль на странице, плата запоминает и подключается сама. Переподключается при потере связи. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashWiFi** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(name, password, timeoutMs)` | Подключиться, ждать до timeoutMs |
| `autoConnect(apName)` | Сохранённая сеть, иначе портал настройки с телефона |
| `startPortal(apName, pass, timeoutSec)` | Только портал настройки |
| `update()` | Вызывать в loop(): автопереподключение |
| `isConnected() / ip() / rssi() / signalText()` | Состояние, IP, сигнал |
| `save(name, pass) / forget() / hasSaved() / beginSaved()` | Сохранённая сеть |
| `scan(Serial)` | Список сетей вокруг |
| `setHostname(s) / setAutoReconnect(on) / setProgress(&Serial)` | Настройки |
| `errorText() / diagnose(Serial, name)` | Причина / самопроверка |
| `addParam(id, label, default, maxLen) / param(id) / setParam(id, value)` | Свои поля портала (MQTT, токен, имя) и их значения |

### Если что-то не работает
**Плата не видит мою сеть**

ESP32 и ESP8266 работают только с Wi-Fi 2.4 ГГц. В роутере с двумя диапазонами включите 2.4 ГГц (часто у него отдельное имя). Приблизьтесь к роутеру. Запустите пример Scan.

**Пароль точно верный, но «не подошёл»**

Проверьте регистр, раскладку и пробел в конце. Сеть с символами вне ASCII в имени иногда ломается. Попробуйте раздать интернет с телефона: у школьных сетей бывает вход через страницу или фильтр по MAC.

**После настройки с телефона плата не запомнила сеть**

На ESP32 данные пишутся в Preferences, на ESP8266 в EEPROM (адреса 400-501). Не очищайте всю память платы командой стирания flash перед запуском.

**Свои поля портала пустые или не сохраняются**

Вызывайте `addParam()` до `autoConnect()` или `startPortal()` и в том же порядке при каждом запуске: на ESP8266 значения лежат в EEPROM по порядку полей, и при смене набора полей они сбрасываются на значения по умолчанию. id только из латинских букв, цифр и _ до 13 символов; всего не больше 6 полей (ALASH_WIFI_MAX_PARAMS).

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

---

## Қазақша

**ESP32 және ESP8266-ны Wi-Fi-ға** қосуға арналған қарапайым кітапхана: `wifi.begin(аты, құпиясөз)` және сәтсіз болса **түсінікті себеп**. Телефоннан **баптауды** (WiFiManager сияқты) біледі: тақта желі жасайды, сіз бетте аты мен құпиясөзді енгізесіз, тақта есте сақтап, өзі қосылады. Байланыс үзілсе қайта қосылады. Тәуелділіксіз.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashWiFi** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(name, password, timeoutMs)` | Қосылу |
| `autoConnect(apName)` | Сақталған желі, әйтпесе порталь |
| `startPortal(apName, pass, timeoutSec)` | Тек баптау порталы |
| `update()` | loop() ішінде: автоқосылу |
| `isConnected() / ip() / rssi() / signalText()` | Күйі, IP, сигнал |
| `save(name, pass) / forget() / hasSaved() / beginSaved()` | Сақталған желі |
| `scan(Serial)` | Желілер тізімі |
| `setHostname(s) / setAutoReconnect(on) / setProgress(&Serial)` | Баптаулар |
| `errorText() / diagnose(Serial, name)` | Себеп / өзін тексеру |
| `addParam(id, label, default, maxLen) / param(id) / setParam(id, value)` | Порталдың өз өрістері және мәндері |

### Бірдеңе жұмыс істемесе
**Тақта желіні көрмейді**

ESP32 және ESP8266 тек 2.4 ГГц Wi-Fi-мен жұмыс істейді. Scan мысалын іске қосыңыз.

**Құпиясөз дұрыс, бірақ «сәйкес келмеді»**

Регистр мен пернетақта тілін тексеріңіз. Телефоннан интернет таратып көріңіз.

**Баптаудан кейін тақта желіні есте сақтамады**

ESP32-де Preferences, ESP8266-де EEPROM (400-501).

**Порталдың өз өрістері бос немесе сақталмайды**

`addParam()` функциясын `autoConnect()` немесе `startPortal()` алдында және әр іске қосуда бірдей ретпен шақырыңыз: ESP8266-да мәндер EEPROM-да өрістер ретімен сақталады, өрістер жиыны өзгерсе әдепкі мәндерге тасталады. id тек латын әріптері, сандар және _, 13 таңбаға дейін; барлығы 6 өрістен аспайды.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

---

## English

A simple library to connect an **ESP32 and ESP8266 to Wi-Fi**: `wifi.begin(name, password)` and a **clear reason** when it fails ("network not found, the board sees only 2.4 GHz", "the password was rejected"). It can do **phone-based setup** (like WiFiManager): the board creates a network, you enter the name and password on a page, the board remembers it and connects by itself. It reconnects when the link drops. No dependencies. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashWiFi** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin(name, password, timeoutMs)` | Connect, wait up to timeoutMs |
| `autoConnect(apName)` | The saved network, otherwise the phone setup portal |
| `startPortal(apName, pass, timeoutSec)` | The setup portal only |
| `update()` | Call in loop(): auto-reconnect |
| `isConnected() / ip() / rssi() / signalText()` | State, IP, signal |
| `save(name, pass) / forget() / hasSaved() / beginSaved()` | The saved network |
| `scan(Serial)` | List nearby networks |
| `setHostname(s) / setAutoReconnect(on) / setProgress(&Serial)` | Settings |
| `errorText() / diagnose(Serial, name)` | Reason / self-check |
| `addParam(id, label, default, maxLen) / param(id) / setParam(id, value)` | Your own portal fields (MQTT, token, name) and their values |

### If something does not work
**The board does not see my network**

ESP32 and ESP8266 work only with 2.4 GHz Wi-Fi. On a dual-band router enable 2.4 GHz (often a separate name). Get closer to the router. Run the Scan example.

**The password is right but "rejected"**

Check letter case, layout and a trailing space. Try a phone hotspot: school networks may require a web login or filter by MAC.

**The board did not remember the network after phone setup**

ESP32 stores it in Preferences, ESP8266 in EEPROM (addresses 400-501). Do not erase the whole flash before running.

**The portal's own fields are empty or not saved**

Call `addParam()` before `autoConnect()` or `startPortal()` and in the same order on every start: on ESP8266 the values are kept in the EEPROM in field order, and a changed set of fields resets them to the defaults. id only of Latin letters, digits and _ up to 13 characters; at most 6 fields in total (ALASH_WIFI_MAX_PARAMS).

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `AutoConnect`, `Scan`, `Diagnose`, `Params`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashWiFi
