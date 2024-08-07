#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <StreamString.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#define YELLOW_LED_PIN 25
#define PIR_PIN 13

#define BOTtoken ""
#define CHAT_ID ""

const char *ssid = "";
const char *password = "";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

RTC_DATA_ATTR int movimento = LOW;
RTC_DATA_ATTR int lastMovimento = LOW;

void connectToWiFi();
void disconnectWiFi();
void message(const String& text);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  digitalWrite(YELLOW_LED_PIN, LOW);
  
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, HIGH);

  movimento = digitalRead(PIR_PIN);
  
  if (movimento == HIGH && lastMovimento == LOW) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Serial.println("Movimento Detectado.");
    delay(4000);
  } else if (movimento == LOW && lastMovimento == HIGH) {
    digitalWrite(YELLOW_LED_PIN, LOW);
    Serial.println("Sem Movimento.");
  }
  
  lastMovimento = movimento;
  
  Serial.println("Entrando em modo deep sleep...");
  esp_deep_sleep_start();
}

void loop() {

}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conectando ao WiFi...");
      delay(1000);
  }

  Serial.print("Conectado com sucesso na rede ");
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

void disconnectWiFi() {
  if (WiFi.isConnected()) {
    WiFi.disconnect(true);
    Serial.println("Desconectado do Wi-Fi.");
  }
  WiFi.mode(WIFI_OFF);
  Serial.println("Módulo Wi-Fi desligado.");
}

void message(const String& text) {
  if (bot.sendMessage(CHAT_ID, text, "")) {
    Serial.println("Mensagem enviada com sucesso!");
  } else {
    Serial.println("Falha ao enviar mensagem!");
  }
}