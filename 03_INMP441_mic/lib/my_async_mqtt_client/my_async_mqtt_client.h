#ifndef __MY_ASYNC_MQTT_CLIENT__
#define __MY_ASYNC_MQTT_CLIENT__

#include <Arduino.h>

#define MQTT_HOST IPAddress(1, 94, 142, 116)
#define MQTT_PORT 1883

typedef void (*mqttItemCb)(char* topic, char* payload, size_t len);

typedef struct {
    char *topic;
    mqttItemCb cb;
} mqttItem;

void asyncMqttClientInit();

/**
 * 注册新的 mqttItem 到 mqttItems 向量中
 * @param topic 主题字符串
 * @param cb 回调函数指针
 */
void registerMqttItem(const char* topic, mqttItemCb cb);

#endif