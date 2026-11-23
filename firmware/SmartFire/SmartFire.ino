// SmartFire.ino
#include "net.h"
#include "sensors.h"
#include "actuators.h"

void setup() {
  Serial.begin(115200);
  actuatorsSetup();  // trước tiên: tắt relay, đưa cửa về vị trí đóng
  sensorsSetup();
  netOnCommand(actuatorsHandleCommand);
  netOnConnect(actuatorsPublishState);
  netSetup();
}

void loop() {
  netLoop();
  sensorsLoop();
  actuatorsLoop();
}
