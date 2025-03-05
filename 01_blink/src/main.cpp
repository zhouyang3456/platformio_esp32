/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <YAMLDuino.h>

// esp32-wroom-32 开发板 led pin 脚为 GPIO2
#define LED_BUILTIN 2

struct PROFILE
{
  /* data */
  char name[20];
  char version[56];
};

// 定义一个 YAML 字符串
const char* yaml = R"(
name: Arduino
version: 1.0.0
features:
  - WiFi
  - Bluetooth
)";

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // 初始化串口
  Serial.begin(115200);

  // 初始化 SPIFFS 文件系统
  if (!SPIFFS.begin(true)) {  // true 表示如果文件系统未挂载，则格式化
    Serial.println("SPIFFS Mount Failed!");
    return;
  }

  Serial.println("SPIFFS Mount Success!");

  PROFILE pf = {
    "spiffs test",
    "1.0.0"
  };

  // 创建或打开文件，并写入内容
  File file = SPIFFS.open("/profile.txt", "w");
  if (file) {
    file.write((uint8_t *)&pf, sizeof(pf));
    file.close();
    Serial.println("File written successfully.");
  } else {
    Serial.println("Failed to open file for writing.");
  }

  PROFILE temp = {0};
  file = SPIFFS.open("/profile.txt", "r");
  if (file) {
    file.read((uint8_t*)&temp, sizeof(temp));
    file.close();
    Serial.println("File read successfully.");
  } else {
    Serial.println("Failed to open file for read.");
  }

  // 打印结构体数据
  Serial.print("Name: ");
  Serial.println(temp.name);
  Serial.print("Version: ");
  Serial.println(temp.version);
  
  // 创建或打开文件，并写入内容
  file = SPIFFS.open("/hello.txt", "a");
  if (file) {
    file.println("Hello, SPIFFS on ESP32!");
    file.close();
    Serial.println("File written successfully.");
  } else {
    Serial.println("Failed to open file for writing.");
  }

  // 打开文件并读取内容
  file = SPIFFS.open("/hello.txt", "r");
  if (file) {
    Serial.println("File content:");
    while (file.available()) {
      Serial.write(file.read());  // 逐字节读取文件内容并输出到串口
    }
    file.close();
  } else {
    Serial.println("Failed to open file for reading.");
  }

  Serial.printf("%s", yaml);

  // 打开 YAML 文件
  file = SPIFFS.open("/config.yaml", "r");
  if (!file) {
    Serial.println("无法打开 YAML 文件");
    return;
  }

  // 创建 YAML 解析器对象
  YAMLDocument doc(file);
  
  // 解析 name 和 version
  String name = doc["name"];
  String version = doc["version"];

  // 输出解析结果
  Serial.print("Name: ");
  Serial.println(name);
  Serial.print("Version: ");
  Serial.println(version);

  file.close();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}
