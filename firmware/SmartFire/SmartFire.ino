// SmartFire.ino
#include "net.h"
#include "sensors.h"

void setup() {
  Serial.begin(115200);
  sensorsSetup();
  netSetup();
}

void loop() {
  netLoop();
  sensorsLoop();
  // Bảo thêm actuatorsLoop() tại đây
}