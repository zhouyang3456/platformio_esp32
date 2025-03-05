#include <Arduino.h>
#include <driver/i2s.h>
#include <inmp441_mic.h>

// I2S 引脚配置
#define I2S_DATA_PIN    27   // DIN - 数据输入（I2S）
#define I2S_BCK_PIN         26   // BCK - 时钟信号（I2S）
#define I2S_LRCK_PIN        25   // LRCLK - 左右声道时钟信号（I2S）

// I2S 配置
#define I2S_NUM             I2S_NUM_0  // 使用 I2S 设备 0
#define SAMPLE_RATE         44100  // 采样率（Hz）
#define CHANNELS            2         // 双声道（Stereo）
#define BITS_PER_SAMPLE     I2S_BITS_PER_SAMPLE_16BIT  // 16 位 PCM 数据

#define bufferLen   640

void inmp441_loop(void *pvParameter) 
{
    int16_t sBuffer[bufferLen];

    while (1)
    {
        size_t bytesIn = 0;
        esp_err_t result = i2s_read(I2S_NUM, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
        if (result == ESP_OK)
        {
            int samples_read = bytesIn / 2;
            if (samples_read > 0) {
                float mean = 0;
                for (int i = 0; i < samples_read; ++i) {
                    mean += (sBuffer[i]);
                }
                mean /= samples_read;
                Serial.println(mean);
                // Serial.write((const uint8_t*) sBuffer, bytesIn);
            }
        }
        vTaskDelay(0.5 / portTICK_PERIOD_MS);
    }
}

void inmp441_mic_init()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // 使用 OR 组合配置模式
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 64,
        .use_apll = false,
        // .tx_desc_auto_clear = true,
    };

  // 配置 I2S 引脚，确保字段顺序按定义顺序初始化
  i2s_pin_config_t pin_config = 
  {
    .mck_io_num =   I2S_PIN_NO_CHANGE,   // MCK 引脚
    .bck_io_num =   I2S_BCK_PIN,   // 位时钟引脚
    .ws_io_num =    I2S_LRCK_PIN,   // 左右声道时钟引脚
    .data_out_num = I2S_PIN_NO_CHANGE,  // 数据输出引脚
    .data_in_num =  I2S_DATA_PIN  // 数据输入引脚
  };

  // 安装 I2S 驱动
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);

  // 创建 I2S 播放任务
  xTaskCreate(inmp441_loop, "inmp441_loop", 4096, NULL, 4, NULL);
}