// AlashWiFi: какие Wi-Fi сети видит плата и с каким сигналом. Помогает понять, виден ли ваш роутер и не слишком ли он далеко.
#include <AlashWiFi.h>

AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  delay(500);
  wifi.scan(Serial);                                   // имя, уровень сигнала (-50 отлично, -80 слабо), канал, защищена ли
}

void loop() {}
