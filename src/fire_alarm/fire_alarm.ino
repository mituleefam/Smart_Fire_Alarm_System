#include "flame_alarm.h"
void setup() { 
  Serial.begin(115200); 
  flameSetup(); 
}
void loop()  { 
  flameLoop(); 
}