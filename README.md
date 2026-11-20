# Smart Fire Alarm System (nhóm 20)

ESP32 + MQ-2 + DHT11 → MQTT (EMQX) → Node-RED → Dashboard, MongoDB Atlas, Discord.

## Cấu trúc

```
docs/                     Quy định đồ án, plan nhóm, hướng dẫn, ghi chú
firmware/
  SmartFire/              Firmware chính (mở SmartFire.ino bằng Arduino IDE)
  tests/
    doc_mq2_dht11/        Thử đọc MQ-2 và DHT11 qua Serial (giai đoạn 1)
    check_wifis/          Quét các mạng WiFi 2.4 GHz mà ESP32 nhìn thấy
node-red/
  flows.json              Flow Node-RED (tab Cảm biến, Cảnh báo, Cloud)
```

## Cài đặt nhanh

**Firmware**
1. Copy `firmware/SmartFire/secrets.example.h` thành `secrets.h`, điền WiFi và tài khoản MQTT.
2. `secrets.h` đã nằm trong `.gitignore`, không commit.

**Node-RED**
1. Cài node: `@flowfuse/node-red-dashboard`, `node-red-contrib-mongodb4`.
2. Menu ☰ → Import → chọn `node-red/flows.json`.
3. File này **không chứa mật khẩu**. Sau khi import phải tự nhập lại:
   - Node broker `EMQX`: username và password MQTT.
   - Node `mongodb4-client`: username và password database Atlas.
   - Biến môi trường `DISCORD_WEBHOOK` (URL webhook Discord) trước khi chạy Node-RED.
4. Tab `Cloud` chỉ bật trên **một máy**, máy còn lại Disable tab để tránh ghi trùng.

Mỗi khi sửa flow: Export lại toàn bộ flow, ghi đè `node-red/flows.json`, rồi commit.
