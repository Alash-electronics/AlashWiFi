// [ru] AlashWiFi: не подключается? Скетч покажет сети вокруг, видна ли ваша, и подскажет причину (имя, пароль, диапазон 2.4 ГГц, расстояние).
// [kk] AlashWiFi: қосылмай ма? Скетч айналадағы желілерді, сіздікінің көрінетінін көрсетеді және себебін айтады (аты, құпиясөз, 2.4 ГГц диапазоны, қашықтық).
// [en] AlashWiFi: does not connect? The sketch shows the networks around, whether yours is visible, and tells the reason (name, password, 2.4 GHz band, distance).
#include <AlashWiFi.h>

AlashWiFi wifi;
const char *WIFI_NAME = "ИМЯ_СЕТИ";
const char *WIFI_PASS = "ПАРОЛЬ";

void setup() {
  Serial.begin(115200);
  delay(500);
  wifi.setLanguage(AlashWiFi::LANG_RU);                // LANG_KK: қазақша, LANG_EN: English
  wifi.setProgress(&Serial);
  if (!wifi.begin(WIFI_NAME, WIFI_PASS, 10000)) {
    Serial.println(wifi.errorText());
  }
  wifi.diagnose(Serial, WIFI_NAME);
}

void loop() {}
