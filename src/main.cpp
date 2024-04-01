#include <Arduino.h>

#define GREEN_LED_PIN 25
#define PIR_PIN 13

void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(PIR_PIN, LOW);
}

void loop() {
  int movimento = digitalRead(PIR_PIN);

  if (movimento == HIGH) {
    digitalWrite(GREEN_LED_PIN, HIGH);
    Serial.println("Movimento Detectado.");
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);
    Serial.println("Sem Movimento.");
  }

  delay(2000);
}