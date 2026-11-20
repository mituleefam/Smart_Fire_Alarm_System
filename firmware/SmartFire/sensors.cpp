// sensors.cpp
#include <Arduino.h>
#include <DHT.h>
#include "config.h"
#include "net.h"
#include "sensors.h"

enum SmokeLevel { LV_SAFE, LV_WARN, LV_DANGER };
static SmokeLevel current = LV_SAFE;
static DHT dht(PIN_DHT, DHT11);
static unsigned long lastSendMs = 0;

// Điện áp chân 34 (mV), trung bình 16 lần; tránh được vùng chết của analogRead()
static int readMq2Mv() {
  long sum = 0;
  for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(PIN_MQ2);
  return sum / 16;
}

// Phân mức có vùng trễ: lên theo ngưỡng, xuống phải thấp hơn ngưỡng một khoảng HYST_MV
const char* classifySmoke(int mv) {
  if (mv >= DANGER_MV || (current == LV_DANGER && mv >= DANGER_MV - HYST_MV)) {
    current = LV_DANGER;
    return "DANGER";
  }
  if (mv >= WARN_MV || (current != LV_SAFE && mv >= WARN_MV - HYST_MV)) {
    current = LV_WARN;
    return "WARN";
  }
  current = LV_SAFE;
  return "SAFE";
}

void sensorsSetup() {
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_MQ2, ADC_11db);
  dht.begin();
}

void sensorsLoop() {
  unsigned long now = millis();
  if (now - lastSendMs < SENSOR_PERIOD_MS) return;
  lastSendMs = now;

  char buf[64];

  int mv = readMq2Mv();
  const char* level = (now < WARMUP_MS) ? "WARMUP" : classifySmoke(mv);
  snprintf(buf, sizeof(buf), "{\"mv\":%d,\"level\":\"%s\"}", mv, level);
  netPublish(TOPIC_SMOKE, buf);
  Serial.println(buf);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {  // đọc lỗi thì bỏ qua, không gửi dữ liệu rác
    snprintf(buf, sizeof(buf), "{\"t\":%.1f,\"h\":%.1f}", t, h);
    netPublish(TOPIC_DHT, buf);
    Serial.println(buf);
  } else {
    Serial.println("DHT doc loi");
  }
}