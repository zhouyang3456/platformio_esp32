#ifndef __MY_ASYNC_MQTT_CLIENT__
#define __MY_ASYNC_MQTT_CLIENT__

/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <vector>

#include "my_ringBuffer.h"

// extern CircularBuffer audioStreamBuf;

#define MQTT_HOST IPAddress(1, 94, 142, 116)
#define MQTT_PORT 1883

typedef void (*mqttItemCb)(char* topic, char* payload, size_t len);

typedef struct {
    char *topic;
    mqttItemCb cb;
} mqttItem;

std::vector<mqttItem> mqttItems;

AsyncMqttClient mqttClient;

/**
 * 注册新的 mqttItem 到 mqttItems 向量中
 * @param topic 主题字符串
 * @param cb 回调函数指针
 */
void registerMqttItem(const char* topic, mqttItemCb cb) {
    // 分配并复制主题字符串
    char* topicCopy = new char[strlen(topic) + 1];
    strcpy(topicCopy, topic);

    // 创建 mqttItem 并添加到向量中
    mqttItem newItem = {topicCopy, cb};
    mqttItems.push_back(newItem);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
    Serial.println("success Connected to MQTT server");
    for (const auto& item : mqttItems) {
        mqttClient.subscribe(item.topic, 0);
    }    
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
}

void onMqttUnsubscribe(uint16_t packetId) {
}

void onMqttPublish(uint16_t packetId) {
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    for (const auto& item : mqttItems) {
            if (strcmp(item.topic, topic) == 0) {
                // 调用匹配的回调函数
                item.cb(const_cast<char*>(item.topic), const_cast<char*>(payload), len);
            }
        }
}

void asyncMqttClientInit() {
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}











#endif
