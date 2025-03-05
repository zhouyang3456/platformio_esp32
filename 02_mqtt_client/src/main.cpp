#include "my_include.h"

void setup() {
  // 启动串口
  Serial.begin(115200);

  led_init();

  // wifi 配网
  wifiAutoConnect();
  // mqttClientInit();
  max98357Setup();


  wifiManagerProcessStart();
  // mqttClientLoopStart();

  asyncMqttClientInit();
}

void loop() {


}