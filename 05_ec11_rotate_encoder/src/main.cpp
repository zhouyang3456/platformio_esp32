/*
依赖 OneButton 与 ESP32Encoder 库，需要安装后引用
*/
#include "Arduino.h"
#include "OneButton.h"
#include "ESP32Encoder.h"

/*
connecting Rotary encoder
CLK (A pin) - to any microcontroler intput pin with interrupt -> in this example pin 26
DT (B pin) - to any microcontroler intput pin with interrupt -> in this example pin 27
SW (button pin) - to any microcontroler intput pin -> in this example pin 14
VCC - to microcontroler VCC (then set ROTARY_ENCODER_VCC_PIN -1) or in this example 3.3V
GND - to microcontroler GND
*/

#define ROTARY_ENCODER_A_PIN 26
#define ROTARY_ENCODER_B_PIN 27
#define ROTARY_ENCODER_BUTTON_PIN 14

// 按键
OneButton SW(ROTARY_ENCODER_BUTTON_PIN, true);

// 旋转编码器
ESP32Encoder encoder;
volatile long lastPosition = 0;

void __idle() {
  Serial.println("key idle");
}

void __click() {
  Serial.println("key click");
  Serial.println("Reset encoder position");
  encoder.setCount(0);
}

void __doubleClick() {
  Serial.println("key doubleClick");
}

void __multiClick() {
  Serial.println("key multiClick");
}

void __press() {
  Serial.println("key press");
}

void __longPressStart() {
  Serial.println("key longPressStart");
}

void __DuringLongPress() {
  Serial.println("...");
}

void __longPressStop() {
  Serial.println("key longPressStop");
}

// 定义一个函数来检查编码器位置是否改变
void checkEncoderPosition() {
  long newPosition = encoder.getCount();
  if (newPosition != lastPosition) {
    Serial.printf("New Position: %ld\r\n", newPosition);
    lastPosition = newPosition; // 更新最后的位置
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // 初始化按键
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // 消抖时长 ms
  SW.setDebounceMs(20);

  // click 计数超时时间
  SW.setClickMs(200);

  // 空闲
  SW.attachIdle(__idle);
  // 点击
  SW.attachClick(__click);
  SW.attachDoubleClick(__doubleClick);
  SW.attachMultiClick(__multiClick);
  // 按压
  SW.attachPress(__press);
  SW.attachLongPressStart(__longPressStart);
  SW.attachDuringLongPress(__DuringLongPress);
  SW.attachLongPressStop(__longPressStop);

  // 编码器单边沿检测
  // encoder.attachSingleEdge(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN);
  // 编码器双边沿检测，建议使用
  encoder.attachHalfQuad(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN);
  encoder.setCount(0);
}

void loop() {
  SW.tick(); // 按键状态机
  checkEncoderPosition();
}


