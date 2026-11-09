#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
}

void loop() {
  int n = WiFi.scanNetworks();
  Serial.printf("Tim thay %d mang 2.4 GHz:\n", n);
  for (int i = 0; i < n; i++) {
    Serial.printf("  %s  (kenh %d, song %d dBm)\n",
                  WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
  }
  Serial.println("----");
  delay(10000);  // quét lại mỗi 10 giây
}