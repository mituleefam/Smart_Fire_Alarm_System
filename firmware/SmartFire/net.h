// net.h
#pragma once
void netSetup();
void netLoop();
bool netPublish(const char* topic, const char* payload);

// Bảo: nhận lệnh MQTT và báo khi vừa kết nối
typedef void (*NetCommandHandler)(const char* topic, const char* payload);
typedef void (*NetConnectHandler)();
void netOnCommand(NetCommandHandler handler);   // gọi khi có tin ở topic đã subscribe
void netOnConnect(NetConnectHandler handler);   // gọi mỗi lần (re)connect MQTT thành công
