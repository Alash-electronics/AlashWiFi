// [ru] AlashWiFi: свои поля на странице настройки. Кроме имени сети и пароля телефон вводит адрес MQTT-сервера и имя устройства.
//      Значения сохраняются вместе с сетью и доступны после перезагрузки через wifi.param("id"). Менять их позже можно, открыв портал снова:
//      сеть уже сохранена, поэтому имя сети вводить не нужно, достаточно поправить поля.
// [kk] AlashWiFi: баптау бетіндегі өз өрістеріңіз. Желі аты мен құпиясөзден басқа телефон MQTT сервер мекенжайы мен құрылғы атын енгізеді.
//      Мәндер желімен бірге сақталады және қайта жүктегеннен кейін wifi.param("id") арқылы қолжетімді. Кейін порталды қайта ашып өзгертуге болады:
//      желі сақталған, сондықтан желі атын енгізудің қажеті жоқ, өрістерді түзету жеткілікті.
// [en] AlashWiFi: your own fields on the setup page. Besides the network name and password the phone enters the MQTT server address and the device name.
//      The values are saved together with the network and are available after a reboot through wifi.param("id"). You can change them later by opening the portal again:
//      the network is already saved, so there is no need to enter the network name, just edit the fields.
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
