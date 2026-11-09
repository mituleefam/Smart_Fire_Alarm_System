// test_sensors.ino
#include <DHT.h>

const int PIN_MQ2 = 34;
const int PIN_DHT = 33;
DHT dht(PIN_DHT, DHT11);
unsigned long lastMs = 0;

int readMq2Raw() {
  long sum = 0;
  for (int i = 0; i < 16; i++) sum += analogRead(PIN_MQ2);  // trung bình 16 lần để giảm nhiễu
  return sum / 16;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);                  // 0-4095
  analogSetPinAttenuation(PIN_MQ2, ADC_11db); // đo được tới khoảng 3.1-3.3V
  dht.begin();
}

void loop() {
  if (millis() - lastMs < 2000) return;  // không dùng delay()
  lastMs = millis();

  int raw = readMq2Raw();
  int mv = analogReadMilliVolts(PIN_MQ2);
  float vAo = mv * 1.5 / 1000.0;  // quy ngược qua cầu chia áp: 30k/20k = 1.5
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("raw="); Serial.print(raw);
  Serial.print(" V_ADC(mV)="); Serial.print(mv);
  Serial.print(" V_AO(V)="); Serial.print(vAo, 2);
  if (isnan(t) || isnan(h)) {
    Serial.println(" DHT LOI");
  } else {
    Serial.print(" t="); Serial.print(t);
    Serial.print(" h="); Serial.println(h);
  }
}