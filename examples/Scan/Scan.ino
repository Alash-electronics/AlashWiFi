// [ru] AlashWiFi: какие Wi-Fi сети видит плата и с каким сигналом. Помогает понять, виден ли ваш роутер и не слишком ли он далеко.
// [kk] AlashWiFi: тақта қандай Wi-Fi желілерін және қандай сигналмен көретіні. Роутеріңіздің көрінетінін және тым алыс емес екенін түсінуге көмектеседі.
// [en] AlashWiFi: which Wi-Fi networks the board sees and with what signal. Helps to understand whether your router is visible and not too far away.
#include <AlashWiFi.h>

AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  delay(500);
  wifi.scan(Serial);                                   // имя, уровень сигнала (-50 отлично, -80 слабо), канал, защищена ли
}

void loop() {}
