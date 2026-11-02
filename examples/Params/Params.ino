// AlashWiFi: свои поля на странице настройки. Кроме имени сети и пароля телефон вводит адрес MQTT-сервера и имя устройства.
// Значения сохраняются вместе с сетью и доступны после перезагрузки через wifi.param("id"). Менять их позже можно, открыв портал снова:
// сеть уже сохранена, поэтому имя сети вводить не нужно, достаточно поправить поля.
#include <AlashWiFi.h>

AlashWiFi wifi;

void setup() {
  Serial.begin(115200);
  wifi.setProgress(&Serial);
  // id (латиница), подпись на странице, значение по умолчанию, максимальная длина. Добавлять до autoConnect()
  wifi.addParam("mqtt", "Адрес MQTT сервера", "broker.hivemq.com", 40);
  wifi.addParam("name", "Имя устройства", "robot1", 16);

  if (wifi.autoConnect("AlashSetup")) {
    Serial.print("Подключились, IP: "); Serial.println(wifi.ip());
    Serial.print("MQTT сервер: "); Serial.println(wifi.param("mqtt"));
    Serial.print("Имя: "); Serial.println(wifi.param("name"));
  } else {
    Serial.println(wifi.errorText());
  }
}

void loop() {
  wifi.update();
  // Открыть портал снова (например, по долгому нажатию кнопки): wifi.startPortal("AlashSetup");
}
