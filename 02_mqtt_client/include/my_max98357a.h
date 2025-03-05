#ifndef __MY_MAX98357A__
#define __MY_MAX98357A__

#include <Arduino.h>
#include <driver/i2s.h>
#include <ArduinoJson.h>

#include "my_led.h"
#include "my_ringBuffer.h"
#include "my_async_mqtt_client.h"

// I2S 引脚配置
#define I2S_DATA_OUT_PIN    27   // DIN - 数据输入（I2S）
#define I2S_BCK_PIN         26   // BCK - 时钟信号（I2S）
#define I2S_LRCK_PIN        25   // LRCLK - 左右声道时钟信号（I2S）

// I2S 配置
#define I2S_NUM             I2S_NUM_0  // 使用 I2S 设备 0
#define SAMPLE_RATE         8000  // 采样率（Hz）
#define CHANNELS            2         // 双声道（Stereo）
#define BITS_PER_SAMPLE     I2S_BITS_PER_SAMPLE_16BIT  // 16 位 PCM 数据

CircularBuffer audioStreamBuf;
static uint8_t gain = 4;

#if 0
void i2s_task(void *pvParameter) {
  // int16_t pcm_data[BUFFER_SIZE];
  int16_t *pcm_data;
  // int16_t **pcm_data;
  size_t bytes_written;
  static int total_bytes = 0;

  while (1) {
    // 从队列获取 PCM 数据
    if (xQueueReceive(pcmQueue, &pcm_data, portMAX_DELAY)) {
      // 将 PCM 数据写入 I2S 总线
      // printHexArray(pcm_data, BUFFER_SIZE);
      // 调整音频数据的幅度
      adjustAmplitude(pcm_data, 2048, 0.01 * audio_gain);
      // printHexArray(pcm_data, BUFFER_SIZE);
      // i2s_write(I2S_NUM, pcm_data, sizeof(pcm_data), &bytes_written, portMAX_DELAY);
      i2s_write(I2S_NUM, pcm_data, 4096, &bytes_written, portMAX_DELAY);
      // total_bytes += bytes_written;
      // Serial.printf("total written bytes[%d]\r\n", total_bytes);
      // checkQueueStatus();
      // 延时 100 毫秒，避免过于频繁的调用，影响其他任务
      vTaskDelay(1 / portTICK_PERIOD_MS);  // 每 100 毫秒调用一次 client.loop()
    }
  }
}
#endif

/**
 * 交换给定数组中每两个字节的位置
 * @param data 指向要处理的数据的指针
 * @param length 数据的长度（以字节为单位）
 */
void swapBytes(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length - 1; i += 2) {
        // 交换当前对的字节
        uint8_t temp = data[i];
        data[i] = data[i + 1];
        data[i + 1] = temp;
    }
}

/**
 * 将 uint8_t 数组转换为 int16_t 数组，并应用增益系数
 * @param input 输入的 uint8_t 数组
 * @param output 输出的 int16_t 数组
 * @param length 输入数组的长度（以字节为单位）
 * @param gain 增益系数
 */
void convertAndApplyGain(const uint8_t* input, int16_t* output, size_t length, float gain) {
    for (size_t i = 0; i < length; i += 2) {
        // 创建一个临时变量来存储两个字节
        uint8_t tempBytes[2];
        
        // 如果输入数组长度不是偶数，则忽略最后一个字节
        if (i + 1 >= length) break;
        
        // 复制两个字节到临时变量
        tempBytes[0] = input[i];
        tempBytes[1] = input[i + 1];

        // 将两个字节转换为 int16_t
        int16_t sample;
        memcpy(&sample, tempBytes, sizeof(sample));
        
        // 应用增益系数
        sample = static_cast<int16_t>(sample * gain);

        // 存储结果
        output[i / 2] = sample;
    }
}

void i2s_task2(void *pvParameter) 
{
  uint8_t readData[512];
  int16_t pcmData[256];
  size_t bytes_written;
  while (1)
  {
    if (getCurrentLength(&audioStreamBuf) > 512) 
    {
      // 批量读取数据
      if (readBatchCircularBuffer(&audioStreamBuf, readData, 512) < 0) {
        continue;
      } 

      // swapBytes(readData, 512);

      convertAndApplyGain(readData, pcmData, 512, 0.01 * gain);

      i2s_write(I2S_NUM, pcmData, 512, &bytes_written, portMAX_DELAY);
      // Serial.printf("Bytes write[%zu]\r\n", bytes_written);
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 每 100 毫秒调用一次 client.loop()
  }
}

void audioPlayCb(char* topic, char* payload, size_t len)
{ 
    if (payload[0] % 2 == 0) {
        led_off();
    } else {
        led_on();
    }
    // static size_t totalRecvLength = 0;
    // totalRecvLength += len;

    writeBatchCircularBuffer(&audioStreamBuf, (const uint8_t *)payload, len);
    // 打印缓冲区信息
    // Serial.printf("total size recv[%zu]  ", totalRecvLength);
    // printCircularBufferInfo(&audioStreamBuf);
}

void audioGainCb(char* topic, char* payload, size_t len)
{
  gain = payload[0] % 20;
  Serial.printf("audio gain[%hhu]\r\n", gain);
}

i2s_config_t configure_i2s(uint32_t sample_rate, i2s_bits_per_sample_t bits_per_sample);

void audioInfoCb(char* topic, char* payload, size_t len)
{
// 创建一个 DynamicJsonDocument 对象
    const size_t capacity = JSON_OBJECT_SIZE(3); // 根据 JSON 对象大小调整容量
    DynamicJsonDocument doc(capacity);

    // 解析 JSON 字符串
    DeserializationError error = deserializeJson(doc, payload, len);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    // 提取 JSON 数据
    uint32_t sample_rate = doc["sample_rate"];      // 获取采样率
    uint8_t sample_width = doc["sample_width"];     // 获取采样深度（位数）
    uint8_t channels = doc["channels"];             // 获取声道数

    // 打印提取的数据
    Serial.println("JSON 数据解析结果:");
    Serial.printf("采样率: %d Hz\n", sample_rate);
    Serial.printf("采样深度: %d 位\n", sample_width);
    Serial.printf("声道数: %d\n", channels);

    configure_i2s(sample_rate, (i2s_bits_per_sample_t)sample_width);
}

/**
 * 设置 I2S 配置结构体
 * @param sample_rate 采样率
 * @param bits_per_sample 采样深度（位数）
 * @return i2s_config_t 配置好的 I2S 配置结构体
 */
i2s_config_t configure_i2s(uint32_t sample_rate, i2s_bits_per_sample_t bits_per_sample) {
    i2s_driver_uninstall(I2S_NUM);

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // 使用 OR 组合配置模式
        .sample_rate = sample_rate,
        .bits_per_sample = bits_per_sample,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 16,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
    };

  // 配置 I2S 引脚，确保字段顺序按定义顺序初始化
  i2s_pin_config_t pin_config = 
  {
    .mck_io_num =   I2S_PIN_NO_CHANGE,   // MCK 引脚
    .bck_io_num =   I2S_BCK_PIN,   // 位时钟引脚
    .ws_io_num =    I2S_LRCK_PIN,   // 左右声道时钟引脚
    .data_out_num = I2S_DATA_OUT_PIN,  // 数据输出引脚
    .data_in_num =  I2S_PIN_NO_CHANGE  // 数据输入引脚
  };

  // 安装 I2S 驱动
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
    
  return i2s_config;
}

void max98357Setup() {
  // 配置 I2S
  configure_i2s(SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT);

  initCircularBuffer(&audioStreamBuf, 10240);
  registerMqttItem("audio/play", audioPlayCb);
  registerMqttItem("audio/gain", audioGainCb);
  registerMqttItem("audio/info", audioInfoCb);

  // 创建 I2S 播放任务
  xTaskCreate(i2s_task2, "I2S Task", 4096, NULL, 4, NULL);

  Serial.println("I2S initialized!");
}

#endif // __MY_MAX98357A__