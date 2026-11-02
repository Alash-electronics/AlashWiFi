// [ru] AlashWiFi: самый простой пример. Подключение к Wi-Fi с понятным сообщением, если не получилось.
//      Для ESP32 и ESP8266. Только Wi-Fi 2.4 ГГц (5 ГГц эти платы не видят).
// [kk] AlashWiFi: ең қарапайым мысал. Wi-Fi-ға қосылу, болмаса түсінікті хабарлама.
//      ESP32 және ESP8266 үшін. Тек 2.4 ГГц Wi-Fi (бұл тақталар 5 ГГц көрмейді).
// [en] AlashWiFi: the simplest example. Connecting to Wi-Fi with a clear message if it failed.
//      For ESP32 and ESP8266. 2.4 GHz Wi-Fi only (these boards do not see 5 GHz).
#include <AlashWiFi.h>

AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);                           // печатать точки, пока подключается
  if (wifi.begin("ИМЯ_СЕТИ", "ПАРОЛЬ")) {
    Serial.print("Подключились, IP: ");
    Serial.println(wifi.ip());
  } else {
    Serial.println(wifi.errorText());                  // "Сеть не найдена...", "Пароль не подошёл..." и что делать
  }
}

void loop() {
  wifi.update();                                       // сам переподключается, если связь пропала
  delay(1000);
}
