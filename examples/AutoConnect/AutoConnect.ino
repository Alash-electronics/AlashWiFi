// [ru] AlashWiFi: настройка Wi-Fi с телефона, без пароля в скетче (как у WiFiManager).
//      Первый запуск: плата создаёт Wi-Fi сеть "AlashSetup". Подключитесь к ней телефоном, откроется страница: выберите свою сеть и введите пароль.
//      Плата запомнит сеть и при следующих включениях подключится сама. Забыть сеть: wifi.forget() (например, по долгому нажатию кнопки).
// [kk] AlashWiFi: Wi-Fi-ды телефоннан баптау, скетчте құпиясөзсіз (WiFiManager сияқты).
//      Алғашқы іске қосу: тақта "AlashSetup" Wi-Fi желісін жасайды. Оған телефонмен қосылыңыз, бет ашылады: өз желіңізді таңдап, құпиясөзді енгізіңіз.
//      Тақта желіні есте сақтайды және келесі қосуларда өзі қосылады. Желіні ұмыту: wifi.forget() (мысалы, батырманы ұзақ басқанда).
// [en] AlashWiFi: setting up Wi-Fi from a phone, without a password in the sketch (like WiFiManager).
//      The first start: the board creates a Wi-Fi network "AlashSetup". Connect to it with a phone, a page opens: choose your network and enter the password.
//      The board remembers the network and connects by itself on the next power-ons. To forget the network: wifi.forget() (for example on a long button press).
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
