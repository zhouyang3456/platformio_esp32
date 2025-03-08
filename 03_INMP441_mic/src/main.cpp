#include <Arduino.h>
#include <inmp441_mic.h>
#include <my_wifiManager.h>
#include <my_async_mqtt_client.h>

void setup() {
  Serial.begin(921600);

  // put your setup code here, to run once:
  // inmp441_mic_init();

  my_wifiAutoConnect(120);

  asyncMqttClientInit();
}

void loop() {
    // 检查串口是否收到 "start" 指令
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');  // 读取串口命令
        command.trim();  // 去除首尾空格
        if (command == "start") {
            Serial.println("OK");
            inmp441_mic_init();
            // Serial.println("Audio streaming started");
        } else if (command == "stop") {
            // Serial.println("Audio streaming stopped");
        }
    }
}