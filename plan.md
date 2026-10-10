# Kế hoạch đồ án cuối kỳ: Hệ thống báo cháy ESP32 (bản 2, web bằng Node-RED)

Căn cứ: *Quy định đồ án cuối kỳ, Vật lý cho CNTT, HK1/2026-2027*.
Hai thành viên: **Bảo** và **Thu**.

Thay đổi so với bản 1: web làm hoàn toàn bằng **Node-RED** (bỏ #10), chỉ làm nâng cao **1, 2, 3, 4, 5, 6, 9, 11**, phân công lại theo ý nhóm.

---

## 1. Tóm tắt phân công và điểm

| | Bảo | Thu |
|---|---|---|
| **Luồng căn bản** | Web → Node-RED → MQTT → ESP32 → **Relay → bơm nước** | **MQ-2** → ESP32 → MQTT → Node-RED → Web |
| **Nâng cao** | #1 Cảm biến lửa → Buzzer (1.5) | #2 DHT11 → Web (1.5) |
| | #3 Web → Servo mở cửa thoát hiểm (1.5) | #4 Lưu dữ liệu lên Cloud theo thời gian (1.5) |
| | #5 Lấy dữ liệu Cloud vẽ biểu đồ (1.5) | #11 AI/DS tự xây dựng (2.0) |
| | #6 Thông báo cháy về điện thoại (1.0) | |
| | #9 Đăng ký/đăng nhập, tài khoản trong DB (1.5) | |
| **Tổng nâng cao** | **7.0** (dư 3.0) | **5.0** (dư 1.0) |

Điểm tối đa vẫn đủ 10: Căn bản 4 + Làm việc nhóm 1 + Nâng cao 4 (mỗi người) + Báo cáo 1, (+1 nếu sản phẩm thật), nhân hệ số hiểu bài K.


## 2. Thiết bị Input / Output (giữ nguyên)

| Thiết bị | Loại | Yêu cầu | Người | Vai trò |
|---|---|---|---|---|
| **MQ-2** (khói/gas, analog) | Input | Căn bản (1) | Thu | Đo khói liên tục, gửi lên web |
| **Relay 1 kênh** (+ bơm nước mini làm tải) | Output | Căn bản (2) | Bảo | Bấm nút trên web → bơm phun nước |
| **Cảm biến lửa IR** | Input | #1 | Bảo | Phát hiện lửa tại chỗ |
| **Buzzer active** | Output | #1 | Bảo | Hú còi tại chỗ, chạy cả khi mất mạng |
| **DHT11** | Input | #2 | Thu | Nhiệt độ, độ ẩm lên web |
| **Servo SG90** | Output | #3 | Bảo | Web điều khiển mở/đóng cửa thoát hiểm |

- Chỉ có **bơm** nằm sau relay. Servo và buzzer nối thẳng chân ESP32, nên không vi phạm quy tắc "2 thiết bị sau relay chỉ tính 1".
- Web **không** điều khiển buzzer. Cảm biến lửa **không** gửi lên web.
- #4, #5, #6, #9, #11 không cần thêm thiết bị.

---

## 4. Danh sách mua hàng (giữ nguyên, ≈ 350–400k)

| Linh kiện | SL | Giá ước tính |
|---|---|---|
| ESP32 DevKit V1 | 1 | 100.000 – 120.000 |
| Module MQ-2 | 1 | 25.000 |
| Module DHT11 | 1 | 25.000 |
| Module cảm biến lửa IR | 1 | 12.000 |
| Module buzzer active | 1 | 10.000 |
| Servo SG90 | 1 | 25.000 |
| Module relay 1 kênh 5V | 1 | 12.000 |
| Bơm nước mini 3–6V + ống | 1 | 25.000 |
| Breadboard 830 lỗ | 1 | 25.000 |
| Dây cắm đực-đực, đực-cái | 2 bó | 30.000 |
| Điện trở 10kΩ + 20kΩ (chia áp MQ-2) | vài con | 2.000 |
| Nguồn 5V 2A | 1 | 30.000 |
| Hộp đóng gói | 1 | 30.000 – 50.000 |

Giá tham khảo, có thể chênh ±20%.

---

## 5. Kiến trúc hệ thống

```
[MQ-2]  ─┐
[DHT11] ─┤                    Internet (MQTT)
          ├─> ESP32 <──────> Broker cloud (HiveMQ Cloud / EMQX) <──────> Node-RED (laptop)
[Relay] <┤                                                               │
[Servo] <┤                                                               ├─> Dashboard 2.0 (giao diện web)
          │                                                               ├─> MongoDB Atlas (dữ liệu cảm biến + tài khoản)
[Lửa] ──> ESP32 ──> [Buzzer]  (xử lý tại chỗ)                            ├─> Telegram Bot (thông báo)
                                                                          └─> Function node AI (#11)
```

### Công cụ
| Thành phần | Lựa chọn |
|---|---|
| Firmware ESP32 | Arduino IDE; thư viện `PubSubClient`, `ArduinoJson`, `DHT sensor library`, `ESP32Servo` |
| MQTT broker | **HiveMQ Cloud** gói free (có user/pass, TLS 8883). Dự phòng: `broker.emqx.io` |
| Web (Back-end + Front-end) | **Node-RED** chạy trên laptop. Giao diện dùng **`@flowfuse/node-red-dashboard`** (Dashboard 2.0). Bản cũ `node-red-dashboard` đã ngừng phát triển |
| Cloud DB | **MongoDB Atlas** gói M0 free, dùng node `node-red-node-mongodb` (hỗ trợ kết nối `mongodb+srv://`). Dự phòng: Firebase Realtime Database gọi bằng node `http request` |
| Thông báo | `node-red-contrib-telegrambot` (hoặc node `http request` gọi thẳng Telegram API). Nếu chọn email thì dùng `node-red-node-email` |

### Ai làm phần nào trong Node-RED
Chia flow Node-RED thành nhiều **tab**, mỗi tab có một người phụ trách, cuối cùng export chung 1 file `flows.json`:

| Tab | Người | Nội dung |
|---|---|---|
| `Cảm biến` | Thu | mqtt in (smoke, dht) → xử lý → gauge/text trên dashboard |
| `Cloud` | Thu | Ghi từng bản tin vào MongoDB Atlas (#4) |
| `AI` | Thu | Dự đoán, phát hiện bất thường (#11) |
| `Điều khiển` | Bảo | Nút bơm, slider servo → mqtt out |
| `Lịch sử` | Bảo | Truy vấn Atlas → biểu đồ (#5) |
| `Cảnh báo` | Bảo | Ngưỡng → Telegram (#6) |
| `Tài khoản` | Bảo | Đăng ký, đăng nhập (#9) |

Chương trình ESP32 cũng là một project chung: Thu viết phần đọc MQ-2, DHT11 và publish; Bảo viết phần subscribe lệnh relay, servo và phần cảm biến lửa → buzzer.

### MQTT topic (đổi `nhomXX` thành mã nhóm)
| Topic | Hướng | Payload | Yêu cầu |
|---|---|---|---|
| `nhomXX/fire/smoke` | ESP → Node-RED | `{"raw": 1830, "level": "WARN"}` | Căn bản (1) – Thu |
| `nhomXX/fire/dht` | ESP → Node-RED | `{"t": 31.5, "h": 62}` | #2 – Thu |
| `nhomXX/fire/cmd/pump` | Node-RED → ESP | `{"state": "ON"}` | Căn bản (2) – Bảo |
| `nhomXX/fire/cmd/door` | Node-RED → ESP | `{"angle": 90}` | #3 – Bảo |
| `nhomXX/fire/ack` | ESP → Node-RED | `{"dev": "pump", "state": "ON"}` | Xác nhận lệnh – Bảo |

### Sơ đồ chân ESP32 (giữ nguyên)
| Thiết bị | Chân | Nguồn | Lưu ý |
|---|---|---|---|
| MQ-2 AO | GPIO34 (ADC1) | 5V | **Bắt buộc chia áp** 10k/20k trước khi vào ESP32. Nung nóng 1–2 phút |
| DHT11 | GPIO4 | 3.3V | Đọc cách nhau ≥ 2 giây |
| Cảm biến lửa DO | GPIO27 | 3.3V | Chỉnh biến trở để đặt ngưỡng |
| Buzzer | GPIO25 | 3.3V/5V | Buzzer active |
| Servo | GPIO18 | 5V nguồn ngoài | Không cấp từ chân 3.3V |
| Relay IN | GPIO26 | 5V | Bơm dùng nguồn riêng qua tiếp điểm relay |

Chỉ dùng chân ADC1 (GPIO32–39) cho analog vì ADC2 không chạy khi bật WiFi. Nối GND chung.

---

## 6. Chi tiết từng phần

### Phần của Bảo

**Căn bản (2): Web → Node-RED → MQTT → ESP32 → Relay/bơm**
- Dashboard: `ui-switch` "Phun nước" → node `change` tạo `{"state":"ON"/"OFF"}` → `mqtt out` topic `cmd/pump`.
- ESP32 subscribe `cmd/pump`, `digitalWrite(26, ...)`, rồi publish `ack`. Node-RED nhận `ack` và cập nhật đèn trạng thái trên web, để web hiện trạng thái thật của bơm.

**#1: Cảm biến lửa → ESP32 → Buzzer (1.5)**
- Xử lý hoàn toàn trong ESP: lửa → buzzer hú. Dùng `millis()`, không dùng `delay()` để không làm treo MQTT.
- Khi trình bày: rút WiFi vẫn báo cháy được.

**#3: Web → Servo (1.5)**
- `ui-button` "Mở cửa"/"Đóng cửa" (hoặc `ui-slider` 0–180°) → `mqtt out` topic `cmd/door` → ESP `servo.write(angle)`.

**#5: Lấy dữ liệu Cloud vẽ biểu đồ (1.5)**
- `ui-dropdown` chọn khoảng thời gian (1 giờ / 24 giờ) → node `function` tạo truy vấn `{ts: {$gte: ...}}` → node `mongodb in` (find, sort theo `ts`) → `function` đổi sang định dạng `ui-chart` → **line chart** khói và nhiệt độ.
- Có nút "Tải lại" để thấy rõ dữ liệu được lấy từ Atlas.
- Phụ thuộc #4 của Thu: thống nhất cấu trúc document ngay tuần đầu.

**#6: Thông báo cháy về điện thoại (1.0)**
- Nhận `smoke` và `dht` → node `function` so ngưỡng (khói ở mức DANGER hoặc nhiệt độ ≥ 50°C) → node `delay` ở chế độ **rate limit** (1 tin / 60 giây, chống spam) → Telegram sender.
- Tạo bot qua @BotFather, lấy token và chat_id.

**#9: Đăng ký / đăng nhập, tài khoản lưu trong MongoDB (1.5)**
Đây là phần khó nhất trên Node-RED, vì Dashboard **không có sẵn chức năng đăng ký tài khoản**. Tính năng `adminAuth`/`httpNodeAuth` trong `settings.js` chỉ là mật khẩu cố định, **không** tính là đăng ký. Cách làm:
- Trang "Đăng ký": `ui-form` (username, password) → `function` băm mật khẩu bằng module `crypto` có sẵn của Node.js (`scrypt` hoặc `pbkdf2` + salt) → `mongodb out` insert vào collection `users`. Kiểm tra trùng username trước khi thêm.
- Trang "Đăng nhập": `ui-form` → tìm user trong `users` → băm lại mật khẩu và so sánh.
- Ẩn/hiện trang: lúc đầu chỉ hiện trang Đăng nhập và Đăng ký. Đăng nhập đúng thì dùng `ui-control` để hiện các trang Dashboard, Điều khiển, Lịch sử. Nút "Đăng xuất" thì ẩn lại.
- Bật **Client Constraints** trong cấu hình Dashboard 2.0 để việc ẩn/hiện chỉ áp dụng cho trình duyệt vừa đăng nhập, không áp dụng cho mọi người đang mở web.
- **Làm thử phần này ngay tuần 2**, vì đây là chỗ dễ vướng nhất. Nếu Dashboard 2.0 gây khó, phương án dự phòng là làm trang đăng nhập bằng `http in` + `template` + `http response` và lưu phiên bằng cookie.

### Phần của Thu

**Căn bản (1): MQ-2 → ESP32 → MQTT → Node-RED → Web**
- ESP đọc `analogRead(34)` mỗi 2–5 giây, phân mức `SAFE/WARN/DANGER`, publish `smoke`.
- Node-RED: `mqtt in` → `json` → `ui-gauge` + `ui-text` trạng thái đổi màu.

**#2: DHT11 → Web (1.5)**
- Publish `dht` → Node-RED → 2 gauge nhiệt độ và độ ẩm.

**#4: Lưu lên Cloud theo thời gian (1.5)**
- Mỗi bản tin `smoke`/`dht` → `function` thêm `ts: new Date()` → `mongodb out` (insert) vào collection `sensor_logs`.
- Mỗi lần đọc là **một bản ghi mới**, không ghi đè. Cấu trúc document: `{ ts, type: "smoke"|"dht", raw, level, t, h }`.
- Vào giao diện Atlas để chỉ cho thầy thấy dữ liệu tăng dần theo thời gian.

**#11: AI/DS tự xây dựng (2.0)**
Tự viết thuật toán trong node `function` (JavaScript), không gọi dịch vụ ngoài:
- **Dự đoán**: lấy 30 giá trị nhiệt độ/khói gần nhất từ Atlas, tự tính **hồi quy tuyến tính** (công thức bình phương nhỏ nhất), dự đoán giá trị 5–10 phút tới.
- **Phát hiện bất thường**: tính trung bình và độ lệch chuẩn trượt, cảnh báo khi **z-score > 3**.
- Gộp lại thành "Nguy cơ cháy: Thấp / Trung bình / Cao", hiển thị trên dashboard và vẽ đường dự đoán.
- Phương án nâng cao (nếu còn thời gian): tự huấn luyện mô hình (ví dụ logistic regression bằng Python/scikit-learn) trên dữ liệu đã thu thập, rồi đưa hệ số vào node `function`.
- Đề bài không yêu cầu tính mới hay độ chính xác, nhưng **phải giải thích được từng công thức**.


---

## 7. Lộ trình (7 tuần, co giãn theo hạn nộp)

| Tuần | Bảo | Thu | Cả nhóm |
|---|---|---|---|
| 1 | Cài Node-RED + Dashboard 2.0; cảm biến lửa → buzzer (#1) | Đọc MQ-2, DHT11 trên ESP | Mua linh kiện, tạo repo, tạo broker HiveMQ Cloud + Atlas, chốt topic và cấu trúc document |
| 2 | Relay/bơm từ web (căn bản 2); **làm thử #9** | Publish MQ-2, DHT lên broker; gauge trên web (căn bản 1, #2) | **Mốc 1: 2 luồng căn bản chạy qua Internet** |
| 3 | Servo (#3); hoàn thiện #9 | Lưu Atlas theo thời gian (#4) | Ghép code ESP thành một chương trình |
| 4 | Biểu đồ từ Atlas (#5) | Hồi quy tuyến tính, z-score (#11) | |
| 5 | Telegram (#6) | Hoàn thiện #11 (+ #8 nếu làm) | **Mốc 2: đủ nâng cao** |
| 6 | Đóng hộp, đi dây | Đóng hộp, kiểm tra nguồn | Export `flows.json`, quay video demo, viết báo cáo |
| 7 | Luyện vấn đáp chéo | Luyện vấn đáp chéo | Nộp báo cáo **sớm ít nhất 1 ngày** |

---

## 8. Kịch bản demo

1. Mở web, đăng ký tài khoản mới, đăng nhập (#9). Trước khi đăng nhập không vào được dashboard.
2. Dashboard hiện khói, nhiệt độ, độ ẩm realtime (căn bản 1, #2).
3. Xả gas bật lửa (không đánh lửa) gần MQ-2 → khói tăng, trạng thái đỏ, điện thoại nhận Telegram (#6).
4. Chĩa remote hồng ngoại hoặc bật lửa gần cảm biến lửa → buzzer hú (#1). Rút WiFi rồi làm lại vẫn hú.
5. Bấm "Phun nước" → bơm chạy (căn bản 2). Bấm "Mở cửa thoát hiểm" → servo quay (#3).
6. Mở giao diện MongoDB Atlas cho thấy dữ liệu lưu theo thời gian (#4). Mở trang Lịch sử xem biểu đồ (#5).
7. Trang AI hiển thị dự đoán và mức nguy cơ (#11).

Chuẩn bị sẵn video demo phòng khi mạng phòng thi chặn MQTT.

---

## 9. Chuẩn bị vấn đáp

**Câu hỏi chung cho cả hai:** MQTT là gì? Broker, publish/subscribe, topic, QoS khác nhau thế nào? Vì sao dùng broker trên cloud? Node-RED là gì, flow, node, `msg.payload` là gì? Dữ liệu đi qua những node nào?

**Bảo:**
- Relay hoạt động thế nào, vì sao ESP không cấp điện trực tiếp cho bơm? PWM điều khiển góc servo ra sao? Vì sao servo cần nguồn 5V riêng?
- Cảm biến lửa phát hiện bằng gì (hồng ngoại 760–1100 nm)? Vì sao dùng `millis()` thay `delay()`?
- Từ lúc bấm nút trên web đến lúc bơm chạy, `msg` đi qua những node nào? Web biết bơm đã bật nhờ đâu (ack)?
- Biểu đồ lấy dữ liệu từ đâu, câu truy vấn MongoDB viết thế nào?
- Mật khẩu được lưu thế nào, vì sao phải băm, salt là gì? Làm sao chặn người chưa đăng nhập?
- Chống spam Telegram bằng cách nào?

**Thu:**
- MQ-2 hoạt động theo nguyên lý gì, vì sao phải nung nóng và chia áp? ADC 12-bit (0–4095) là gì?
- DHT11 giao tiếp 1 dây thế nào?
- Dữ liệu được ghi vào Atlas ở node nào, cấu trúc document ra sao? Vì sao đây là "lưu theo thời gian"?
- Hồi quy tuyến tính tính hệ số a, b thế nào? Z-score là gì, vì sao chọn ngưỡng 3?

Mỗi người phải **mở flow Node-RED và giải thích từng node trong tab của mình**.

---

## 10. Rủi ro và cách phòng

| Rủi ro | Cách phòng |
|---|---|
| Thu chỉ dư 1.0 điểm nâng cao | Chuyển #5 sang Thu, hoặc thêm #8 dự phòng (mục 1) |
| #9 khó làm trên Node-RED | Làm thử từ tuần 2; có phương án dự phòng `http in` + cookie |
| Mạng trường chặn cổng MQTT 8883 | Dùng MQTT qua WebSocket của HiveMQ Cloud (cổng 8884), hoặc phát WiFi từ điện thoại |
| Biểu đồ bị cho là không lấy từ Cloud | Biểu đồ chỉ lấy dữ liệu từ truy vấn Atlas, có nút "Tải lại" |
| ESP32 reset khi servo/bơm chạy | Nguồn 5V 2A riêng, GND chung, tụ 470µF gần servo |
| Hai người sửa chung `flows.json` bị ghi đè | Mỗi người chỉ sửa tab của mình; export từng tab rồi import gộp; commit thường xuyên |
| Nộp báo cáo trễ | Nộp trước hạn 1 ngày |
