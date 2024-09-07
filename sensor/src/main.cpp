#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <StreamString.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <esp_now.h>

#define YELLOW_LED_PIN 25
#define PIR_PIN 13
#define REED_PIN 14

#define BOTtoken "BOT_TOKEN"
#define CHAT_ID "CHAT_ID"

typedef struct {
  char message[200];
} espnow_message_t;

esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = {0x68, 0xC6, 0x3A, 0xB8, 0x27, 0x5B};

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

RTC_DATA_ATTR int movimento = LOW;
RTC_DATA_ATTR int reedState = LOW;

RTC_DATA_ATTR int lastMovimento = LOW;
RTC_DATA_ATTR int lastReedState = LOW;

void connectToWiFi();
void disconnectWiFi();
void messageToTelegram(const String& text);
void sendMessageToESPNow(const String &message);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(REED_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  digitalWrite(YELLOW_LED_PIN, LOW);
  
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, HIGH);  
  
  reedState = digitalRead(REED_PIN);
  movimento = digitalRead(PIR_PIN);
  
  if (lastMovimento == LOW && movimento == HIGH) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Serial.println("Movimento Detectado.");
    String logMessage = "Movimento detectado e porta aberta!";
    sendMessageToESPNow(logMessage);
    connectToWiFi();
    messageToTelegram("A porta foi aberta! Um movimento foi detectado!");
    disconnectWiFi();
    delay(5000);
  } else if (lastMovimento == HIGH && movimento == LOW) {
    digitalWrite(YELLOW_LED_PIN, LOW);
    Serial.println("Sem Movimento.");
  }

  if (lastReedState == HIGH && reedState == LOW) {
    Serial.println("Porta fechada.");
    String logMessage = "Porta fechada.";
    sendMessageToESPNow(logMessage);
    delay(2000);
  } else if (lastReedState == LOW && reedState == HIGH) {
    Serial.println("Porta aberta");
    String logMessage = "Porta aberta.";
    sendMessageToESPNow(logMessage);
    delay(2000);
  }

  lastMovimento = movimento;
  lastReedState = reedState;
  
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

void messageToTelegram(const String& text) {
  if (bot.sendMessage(CHAT_ID, text, "")) {
    Serial.println("Mensagem enviada com sucesso!");
  } else {
    Serial.println("Falha ao enviar mensagem!");
  }
}

void sendMessageToESPNow(const String &message) {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  bool peerExists = esp_now_is_peer_exist(broadcastAddress);

  if (!peerExists) {
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Erro ao adicionar peer");
      return;
    }
  }

  espnow_message_t msg;
  message.toCharArray(msg.message, 200);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&msg, sizeof(msg));

  if (result == ESP_OK) {
    Serial.println("Mensagem enviada via ESP-NOW");
  } else {
    Serial.println("Falha ao enviar via ESP-NOW");
  }

  esp_now_deinit();
}
