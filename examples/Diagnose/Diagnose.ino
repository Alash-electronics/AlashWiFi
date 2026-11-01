// AlashWiFi: не подключается? Скетч покажет сети вокруг, видна ли ваша, и подскажет причину (имя, пароль, диапазон 2.4 ГГц, расстояние).
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
