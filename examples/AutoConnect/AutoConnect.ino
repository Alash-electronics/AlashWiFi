// AlashWiFi: настройка Wi-Fi с телефона, без пароля в скетче (как у WiFiManager).
// Первый запуск: плата создаёт Wi-Fi сеть "AlashSetup". Подключитесь к ней телефоном, откроется страница: выберите свою сеть и введите пароль.
// Плата запомнит сеть и при следующих включениях подключится сама. Забыть сеть: wifi.forget() (например, по долгому нажатию кнопки).
#include <AlashWiFi.h>

AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);
  wifi.setLanguage(AlashWiFi::LANG_RU);                // язык страницы настройки: LANG_KK, LANG_EN
  // сохранённая сеть, а если её нет или не подходит, портал на 3 минуты
  if (wifi.autoConnect("AlashSetup")) {
    Serial.print("Подключились, IP: ");
    Serial.println(wifi.ip());
  } else {
    Serial.println(wifi.errorText());
  }
}

void loop() {
  wifi.update();
}
