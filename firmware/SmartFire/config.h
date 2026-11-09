// config.h
#pragma once
#define GROUP_ID "nhom20"
#define TOPIC_SMOKE  GROUP_ID "/fire/smoke"
#define TOPIC_DHT    GROUP_ID "/fire/dht"
#define TOPIC_STATUS GROUP_ID "/fire/status"

const int PIN_MQ2 = 34;
const int PIN_DHT = 33;

const unsigned long SENSOR_PERIOD_MS = 5000;  // chu kỳ gửi
const unsigned long WARMUP_MS = 60000;        // thời gian nung nóng sau khi bật

// Ngưỡng TẠM, đơn vị mV tại chân 34 (không khí sạch đo được khoảng 140-155 mV).
// THAY bằng số đo ở Giai đoạn 2.
const int WARN_MV = 600;
const int DANGER_MV = 1200;
const int HYST_MV = 50;  // vùng trễ, tránh nhảy trạng thái liên tục quanh ngưỡng