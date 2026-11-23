// config.h
#pragma once
#define GROUP_ID "nhom20"
#define TOPIC_SMOKE  GROUP_ID "/fire/smoke"
#define TOPIC_DHT    GROUP_ID "/fire/dht"
#define TOPIC_STATUS GROUP_ID "/fire/status"

// Bảo: lệnh từ web xuống ESP32 và phản hồi trạng thái thật
#define TOPIC_CMD_PUMP GROUP_ID "/fire/cmd/pump"  // {"state":"ON"} / {"state":"OFF"}
#define TOPIC_CMD_DOOR GROUP_ID "/fire/cmd/door"  // {"state":"OPEN"} / {"state":"CLOSED"}
#define TOPIC_ACK      GROUP_ID "/fire/ack"       // {"dev":"pump","state":"ON"}, {"dev":"door","state":"OPEN"}

const int PIN_MQ2 = 34;
const int PIN_DHT = 33;

const unsigned long SENSOR_PERIOD_MS = 5000;  // chu kỳ gửi
const unsigned long WARMUP_MS = 60000;        // thời gian nung nóng sau khi bật

// Ngưỡng TẠM, đơn vị mV tại chân 34 (không khí sạch đo được khoảng 140-155 mV).
// THAY bằng số đo ở Giai đoạn 2.
const int WARN_MV = 600;
const int DANGER_MV = 1200;
const int HYST_MV = 50;  // vùng trễ, tránh nhảy trạng thái liên tục quanh ngưỡng

// ---- Bảo: relay bơm (ID 2) và servo cửa (ID 5) ----
const int PIN_RELAY = 26;
const bool RELAY_ACTIVE_HIGH = true;          // jumper relay ở H: HIGH = đóng. Đặt false nếu để jumper ở L
const unsigned long PUMP_MAX_ON_MS = 30000;   // an toàn: bơm tự tắt sau 30 s, tránh chạy khô

const int PIN_SERVO = 13;

// Loại servo: true = SG90 360° (quay liên tục, chỉ điều khiển tốc độ/chiều)
//             false = SG90 180° (điều khiển theo góc)
const bool SERVO_CONTINUOUS = true;

// -- Dùng khi SERVO_CONTINUOUS = false (servo 180°) --
const int DOOR_CLOSED_DEG = 0;                // góc khi đóng cửa
const int DOOR_OPEN_DEG = 90;                 // góc khi mở cửa (chỉnh theo mô hình)

// -- Dùng khi SERVO_CONTINUOUS = true (servo 360°): mở/đóng = quay một khoảng thời gian --
// Dò bằng firmware/tests/servo360_test rồi điền vào đây.
const int SERVO_STOP_US = 1500;               // độ rộng xung (µs) làm servo đứng yên
const int SERVO_RUN_DELTA_US = 300;           // lệch khỏi điểm dừng khi quay: lớn = nhanh
const int SERVO_OPEN_DIR = 1;                 // 1 hoặc -1: đổi dấu nếu mở cửa quay sai chiều
const unsigned long DOOR_MOVE_MS = 400;       // thời gian quay để đi từ đóng sang mở (và ngược lại)