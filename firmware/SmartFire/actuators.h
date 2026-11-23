// actuators.h – Bảo: relay bơm (ID 2) và servo cửa thoát hiểm (ID 5)
#pragma once
void actuatorsSetup();
void actuatorsLoop();
void actuatorsHandleCommand(const char* topic, const char* payload);  // gắn vào netOnCommand
void actuatorsPublishState();                                          // gắn vào netOnConnect
