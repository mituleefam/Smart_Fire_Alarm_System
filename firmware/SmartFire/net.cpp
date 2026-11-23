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
static NetCommandHandler commandHandler = nullptr;
static NetConnectHandler connectHandler = nullptr;

void netOnCommand(NetCommandHandler handler) { commandHandler = handler; }
void netOnConnect(NetConnectHandler handler) { connectHandler = handler; }

// PubSubClient gọi hàm này khi có tin ở topic đã subscribe.
// topic và payload trỏ vào buffer nội bộ của PubSubClient, sẽ bị ghi đè nếu
// trong lúc xử lý lại publish (gửi ack), nên chép cả hai ra trước.
static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  char topicBuf[64];
  strncpy(topicBuf, topic, sizeof(topicBuf) - 1);
  topicBuf[sizeof(topicBuf) - 1] = '\0';
  char buf[128];
  unsigned int n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';  // payload không có ký tự kết thúc chuỗi
  Serial.printf("MQTT nhan [%s] %s\n", topicBuf, buf);
  if (commandHandler) commandHandler(topicBuf, buf);
}

void netSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  // không chặn, netLoop sẽ theo dõi
  tlsClient.setInsecure();           // mã hóa nhưng không xác minh chứng chỉ
  tlsClient.setHandshakeTimeout(8);  // giây
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setSocketTimeout(5);
  mqtt.setCallback(onMqttMessage);
  snprintf(clientId, sizeof(clientId), "esp32-%llx", ESP.getEfuseMac());  // ID không trùng
}

static void tryConnectMqtt() {
  // Tham số LWT: broker tự gửi "offline" (retain) khi ESP32 mất kết nối
  bool ok = mqtt.connect(clientId, MQTT_USER, MQTT_PASS, TOPIC_STATUS, 0, true, "offline");
  if (ok) {
    mqtt.publish(TOPIC_STATUS, "online", true);
    Serial.println("MQTT da ket noi");
    // QoS 1: lệnh bơm/cửa chắc chắn tới ESP32 (broker gửi lại nếu chưa nhận)
    mqtt.subscribe(TOPIC_CMD_PUMP, 1);
    mqtt.subscribe(TOPIC_CMD_DOOR, 1);
    if (connectHandler) connectHandler();  // báo lại trạng thái bơm/cửa cho web
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