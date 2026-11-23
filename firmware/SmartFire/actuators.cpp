// actuators.cpp – Bảo
// Luồng (2) ID 2: Web → Node-RED → MQTT cmd/pump → ESP32 → relay → bơm → ack
// Luồng ID 5:     Web → Node-RED → MQTT cmd/door → ESP32 → servo → ack
// Cần thư viện "ESP32Servo" (Kevin Harrington) trong Library Manager.
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "net.h"
#include "actuators.h"

static Servo doorServo;
static bool pumpOn = false;
static unsigned long pumpOnSinceMs = 0;
static bool doorOpen = false;
// Servo 360°: đang quay để mở/đóng, dừng khi hết DOOR_MOVE_MS (không dùng delay)
static bool doorMoving = false;
static bool doorTarget = false;
static unsigned long doorMoveStartMs = 0;

// Lấy giá trị của khóa "state" trong JSON nhỏ, vd {"state":"ON"} → so với "ON".
// Không dùng thư viện JSON vì payload chỉ có 1 khóa, cố định.
static bool stateIs(const char* payload, const char* value) {
  const char* p = strstr(payload, "\"state\"");
  if (!p) return false;
  p = strchr(p + 7, ':');
  if (!p) return false;
  p++;
  while (*p == ' ' || *p == '\t') p++;
  if (*p != '"') return false;
  p++;
  size_t n = strlen(value);
  return strncmp(p, value, n) == 0 && p[n] == '"';
}

static void publishAck(const char* dev, const char* state) {
  char buf[48];
  snprintf(buf, sizeof(buf), "{\"dev\":\"%s\",\"state\":\"%s\"}", dev, state);
  netPublish(TOPIC_ACK, buf);  // mất mạng thì bỏ qua, lần kết nối sau sẽ báo lại
}

static void relayWrite(bool on) {
  digitalWrite(PIN_RELAY, (on == RELAY_ACTIVE_HIGH) ? HIGH : LOW);
}

static void setPump(bool on) {
  pumpOn = on;
  if (on) pumpOnSinceMs = millis();
  relayWrite(on);
  Serial.println(on ? "Bom: BAT" : "Bom: TAT");
  publishAck("pump", on ? "ON" : "OFF");
}

static void setDoor(bool open) {
  if (!SERVO_CONTINUOUS) {
    // Servo 180°: đi thẳng tới góc và tự giữ góc
    doorOpen = open;
    doorServo.write(open ? DOOR_OPEN_DEG : DOOR_CLOSED_DEG);
    Serial.println(open ? "Cua: MO" : "Cua: DONG");
    publishAck("door", open ? "OPEN" : "CLOSED");
    return;
  }
  // Servo 360°: không biết góc, chỉ quay theo chiều trong một khoảng thời gian
  if (doorMoving) {
    Serial.println("Cua dang quay, bo qua lenh moi");
    return;
  }
  if (open == doorOpen) {
    // Đã ở đúng trạng thái: không quay thêm, tránh vặn quá đà
    publishAck("door", open ? "OPEN" : "CLOSED");
    return;
  }
  int dir = open ? SERVO_OPEN_DIR : -SERVO_OPEN_DIR;
  doorServo.writeMicroseconds(SERVO_STOP_US + dir * SERVO_RUN_DELTA_US);
  doorMoving = true;
  doorTarget = open;
  doorMoveStartMs = millis();
  Serial.println(open ? "Cua: dang mo..." : "Cua: dang dong...");
}

static void stopDoorIfDone() {
  if (!doorMoving || millis() - doorMoveStartMs < DOOR_MOVE_MS) return;
  doorServo.writeMicroseconds(SERVO_STOP_US);
  doorMoving = false;
  doorOpen = doorTarget;
  Serial.println(doorOpen ? "Cua: MO" : "Cua: DONG");
  publishAck("door", doorOpen ? "OPEN" : "CLOSED");  // ack khi đã quay xong
}

void actuatorsSetup() {
  // Relay tắt ngay khi khởi động, bơm không tự chạy lúc cắm điện
  pinMode(PIN_RELAY, OUTPUT);
  relayWrite(false);

  // SG90: xung 50 Hz, độ rộng 500–2400 µs ứng với 0–180°
  doorServo.setPeriodHertz(50);
  doorServo.attach(PIN_SERVO, 500, 2400);
  if (SERVO_CONTINUOUS) {
    // Servo 360° không biết vị trí: đứng yên, coi như cửa đang ĐÓNG.
    // Trước khi cắm điện, đặt cánh cửa ở vị trí đóng.
    doorServo.writeMicroseconds(SERVO_STOP_US);
  } else {
    doorServo.write(DOOR_CLOSED_DEG);
  }
}

void actuatorsLoop() {
  stopDoorIfDone();

  // An toàn: bơm không được chạy quá PUMP_MAX_ON_MS dù không có lệnh tắt
  if (pumpOn && millis() - pumpOnSinceMs > PUMP_MAX_ON_MS) {
    Serial.println("Bom chay qua lau, tu tat");
    setPump(false);
  }
}

void actuatorsHandleCommand(const char* topic, const char* payload) {
  if (strcmp(topic, TOPIC_CMD_PUMP) == 0) {
    if (stateIs(payload, "ON")) setPump(true);
    else if (stateIs(payload, "OFF")) setPump(false);
    else Serial.println("Lenh bom khong hop le");
  } else if (strcmp(topic, TOPIC_CMD_DOOR) == 0) {
    if (stateIs(payload, "OPEN")) setDoor(true);
    else if (stateIs(payload, "CLOSED")) setDoor(false);
    else Serial.println("Lenh cua khong hop le");
  }
}

void actuatorsPublishState() {
  // Vừa (re)connect: báo trạng thái thật để web hiển thị đúng
  publishAck("pump", pumpOn ? "ON" : "OFF");
  publishAck("door", doorOpen ? "OPEN" : "CLOSED");
}
