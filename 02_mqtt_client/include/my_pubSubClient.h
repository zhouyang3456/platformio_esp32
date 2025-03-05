#ifndef __MY_PUBSUBCLIENT__
#define __MY_PUBSUBCLIENT__

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "my_led.h"
#include "my_max98357a.h"
#include "esp_system.h"
#include <vector>
#include <stdlib.h>
#include <my_doubleBuffer.h>
#include <my_ringBuffer.h>

DoubleBuffer db;

CircularBuffer cb;

#define bufferSize 2048

// Wi-Fi 和 MQTT 配置
const char* mqtt_server = "1.94.142.116";  // 替换为你的 MQTT 服务器地址
const int mqtt_port = 1883;                    // 默认端口
const char* mqtt_user = "mqtt_user";           // MQTT 用户名
const char* mqtt_pass = "mqtt_password";       // MQTT 密码

typedef void (*topicMsgHanlder)(byte* payload, unsigned int length);

typedef struct pubsubtopicItem
{
  char *topic;
  topicMsgHanlder handler;
} pubsubtopicItem;

void ledMsgHandler(byte* payload, unsigned int length)
{
  if (payload[0] == '1') {
    Serial.println("led on");
    led_on();
  } else if (payload[0] == '0') {
    Serial.println("led off");
    led_off();
  } else {
    Serial.println("Unknown");
  }
}

void audioStreamMsgHandler(byte* payload, unsigned int length)
{
  static int count = 0;
  Serial.printf("audio stream PACKAGE[%d]\r\n", ++count);
}

void audioGainMsgHandler(byte* payload, unsigned int length)
{
  setAudioGain((int)payload[0]);
  Serial.printf("set audio gain[%d]\r\n", (int)payload[0]);
}

void audioPlayMsgHandler(byte* payload, unsigned int length) {
  playFlag = true;

  writeBatchCircularBuffer(&cb, payload, length);
      // 打印缓冲区信息
    printCircularBufferInfo(&cb);

}

pubsubtopicItem topics[] = 
{
  {.topic = (char *)"test/led", .handler = ledMsgHandler},
  {.topic = (char *)"audio/stream", .handler = audioStreamMsgHandler},
  {.topic = (char *)"audio/play", .handler = audioPlayMsgHandler},
  {.topic = (char *)"audio/gain", .handler = audioGainMsgHandler}
};

WiFiClient espClient;     // Wi-Fi 客户端
PubSubClient client(espClient);  // MQTT 客户端

// 连接到 MQTT 服务器
void reconnect() {
  while (!client.connected()) {
    Serial.print("连接到 MQTT 服务器...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("连接成功");
      // 订阅一个主题
      client.subscribe("test/topic");
    } else {
      Serial.print("连接失败，状态码：");
      Serial.print(client.state());
      vTaskDelay(5000 / portTICK_PERIOD_MS);  // 每 100 毫秒调用一次 client.loop()
    }
  }
}

static void printMsg(char* topic, byte* payload, unsigned int length)
{
  Serial.print("收到消息 [");
  Serial.print(topic);
  Serial.print("] ");
  // 打印消息内容
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

static void printMsgWithHex(char* topic, byte* payload, unsigned int length)
{
  Serial.print("收到消息 [");
  Serial.print(topic);
  Serial.print("] ");

  // 打印消息内容（以 HEX 格式打印）
  for (int i = 0; i < length; i++) {
    // 使用 %02X 打印每个字节为2位的十六进制，%02X表示格式化为大写的2位十六进制数
    Serial.print("0x");
    Serial.print(payload[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// MQTT 消息处理回调
void callback(char* topic, byte* payload, unsigned int length) {

  // printMsgWithHex(topic, payload, length);

  for (int i = 0; i < sizeof(topics)/sizeof(pubsubtopicItem); i++) {
    if (strcmp(topic, topics[i].topic) == 0) {
      topics[i].handler(payload, length);
      break;
    }
  }

}

void mqttClientInit()
{
  // 设置 MQTT 服务器和回调函数
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // 确保 MQTT 客户端连接
  if (!client.connected()) {
    reconnect();  // 如果没有连接，重新连接 MQTT 服务器
  }

  client.setBufferSize(2048);

  // 订阅一个主题
  client.subscribe("test/led");
  client.subscribe("audio/stream");
  client.subscribe("audio/play");
  client.subscribe("audio/gain");
  // for (int i = 0; i < sizeof(topics)/sizeof(pubsubtopicItem); i++) {
  //   client.subscribe(topics[i].topic);
  // }

  // initDoubleBuffer(&db, 2048);
  initCircularBuffer(&cb, 10240);
}

void mqttClientLoop(void *param)
{
  while (true) {
    // 确保 MQTT 客户端连接
    // if (!client.connected()) {
    //   reconnect();  // 如果没有连接，重新连接 MQTT 服务器
    // }

    // 保持 MQTT 客户端循环
    client.loop();

    // 延时 100 毫秒，避免过于频繁的调用，影响其他任务
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 每 100 毫秒调用一次 client.loop()
  }
}

void mqttClientLoopStart()
{
  xTaskCreate(mqttClientLoop, "mqttClientTask", 10240, NULL, 5, NULL); // 第二个参数是任务名称
}

#endif // __MY_PUBSUBCLIENT__