// net.cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"
#include "secrets.h"
#include "net.h"

static WiFiClientSecure tlsClient;
static PubSubClient mqtt(tlsClient);
static unsigned long lastWifiTryMs = 0;
static unsigned long lastMqttTryMs = 0;
static char clientId[32];

void netSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  // không chặn, netLoop sẽ theo dõi
  tlsClient.setInsecure();           // mã hóa nhưng không xác minh chứng chỉ
  tlsClient.setHandshakeTimeout(8);  // giây
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setSocketTimeout(5);
  snprintf(clientId, sizeof(clientId), "esp32-%llx", ESP.getEfuseMac());  // ID không trùng
}

static void tryConnectMqtt() {
  // Tham số LWT: broker tự gửi "offline" (retain) khi ESP32 mất kết nối
  bool ok = mqtt.connect(clientId, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 0, true, "offline");
  if (ok) {
    mqtt.publish(TOPIC_STATUS, "online", true);
    Serial.println("MQTT da ket noi");
    // Bảo thêm mqtt.subscribe(...) tại đây
  } else {
    Serial.print("MQTT loi, state=");
    Serial.println(mqtt.state());
  }
}

void netLoop() {
  unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWifiTryMs > 10000) {
      lastWifiTryMs = now;
      WiFi.reconnect();
    }
    return;  // mất WiFi: thoát ngay, không làm treo vòng lặp
  }
  if (!mqtt.connected()) {
    if (now - lastMqttTryMs > 5000) {
      lastMqttTryMs = now;
      tryConnectMqtt();  // có thể chặn vài giây: xem lưu ý cho Bảo ở mục A3
    }
    return;
  }
  mqtt.loop();
}

bool netPublish(const char* topic, const char* payload) {
  if (!mqtt.connected()) return false;
  return mqtt.publish(topic, payload);
}