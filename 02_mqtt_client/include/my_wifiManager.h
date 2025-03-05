#ifndef __MY_WIFI_MANAGER__
#define __MY_WIFI_MANAGER__

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>  // 引入 WiFiManager 库

// WiFiManager 实例
WiFiManager wifiManager;

void wifiAutoConnect()
{
    // 配置 ESP32 为 AP 模式
    Serial.println("ESP32 配置为 AP 模式，等待手机连接...");


// 设置配置门户超时时间为 120 秒
      wifiManager.setConfigPortalTimeout(3);
    // 启动 WiFi 配置界面（Captive Portal）
    // WiFiManager 会自动开启一个 AP（默认 SSID 为 "ESP" 开头），并提供一个网页用于 Wi-Fi 配网
    if (!wifiManager.autoConnect("ESP32-Config")) {  // 提供一个名称为 "ESP32-Config" 的 AP
      Serial.println("WiFi 配网失败，重启...");
      delay(1000);
      ESP.restart();  // 如果配网失败，重启 ESP32
    }

    // 配网成功后
    Serial.println("连接成功!");
    Serial.print("ESP32 已连接到 WiFi: ");
    Serial.println(WiFi.SSID());  // 打印连接的 Wi-Fi 名称
}

void wifiManagerProcess(void *param)
{
    while (true) {
      // WiFiManager 会在后台运行 Captive Portal，处理用户的网页请求
      wifiManager.process();

      // 延时 100 毫秒，避免过于频繁的调用，影响其他任务
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void wifiManagerProcessStart()
{
  xTaskCreate(wifiManagerProcess, "wifiManagerProcess", 2048, NULL, 1, NULL); // 第二个参数是任务名称
}


#endif // __MY_WIFI_MANAGER__
