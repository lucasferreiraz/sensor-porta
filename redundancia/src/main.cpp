#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FS.h>
#include <time.h>

typedef struct {
  char message[200];
} espnow_message_t;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 3600;

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len);
String getFormattedTime();

void setup() {
  Serial.begin(9600);

  if (!SPIFFS.begin()) {
    Serial.println("Falha ao inicializar o SPIFFS");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao Wi-Fi");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (esp_now_init() != 0) {
    Serial.println("Falha ao inicializar ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  espnow_message_t receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  String logMessage = String(receivedData.message);

  String currentTime = getFormattedTime();

  Serial.printf("[TIMESTAMP: %s] -- [MSG: %s]\n", currentTime.c_str(), logMessage.c_str());

  File logFile = SPIFFS.open("/log.txt", "a");
  if (!logFile) {
    Serial.println("Erro ao abrir o arquivo de log");
    return;
  }

  logFile.printf("[TIMESTAMP: %s] -- [MSG: %s]\n", currentTime.c_str(), logMessage.c_str());
  logFile.close();
  Serial.println("Mensagem armazenada no log.");
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter a hora");
    return "Erro";
  }
  
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}
