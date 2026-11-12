#include <Arduino.h>
#include "flame_alarm.h"

#define FLAME_PIN  27
#define BUZZER_PIN 25
#define HOLD_MS    2000   // hết lửa 2 giây mới tắt còi

bool alarmOn = false;
unsigned long lastFire = 0;

void flameSetup() {
  Serial.begin(115200);
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void flameLoop() {
  if (digitalRead(FLAME_PIN) == LOW) lastFire = millis();   // LOW = thấy lửa
  bool fire = (lastFire != 0) && (millis() - lastFire < HOLD_MS);

  if (fire != alarmOn) {          // chỉ xử lý khi trạng thái đổi
    alarmOn = fire;
    digitalWrite(BUZZER_PIN, fire ? HIGH : LOW);
    Serial.println(fire ? "PHAT HIEN LUA!" : "Het lua");
  }
}