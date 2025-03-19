#include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "my_max98357a.h"

#define FRAMES_SIZE 128
drwav wav;
int16_t *pcmBuffer;
uint64_t read_frames = 0;

#if 0
void audio_play_task(void *pvParameter) 
{
    // 使用 dr_wav 解析 WAV 文件
    // drwav wav;
    if (!drwav_init_file(&wav, "/spiffs/16000_2_16.wav", NULL)) {
        Serial.println("WAV 解析失败！");
        vTaskDelay(NULL);
    }

    Serial.println("WAV 解析成功！");

    Serial.printf("采样率: %d Hz\n", wav.sampleRate);
    Serial.printf("声道数: %d\n", wav.channels);
    Serial.printf("位深度: %d-bit\n", wav.bitsPerSample);
    Serial.printf("总帧数: %llu\n", wav.totalPCMFrameCount);


    // 读取 PCM 数据
    // size_t dataSize = wav.totalPCMFrameCount * wav.channels * (wav.bitsPerSample / 8);
    int16_t *pcmBuffer = (int16_t *)malloc(FRAMES_SIZE * wav.channels * (wav.bitsPerSample / 8));
    if (!pcmBuffer) {
        Serial.println("内存分配失败！");
        drwav_uninit(&wav);
        vTaskDelay(NULL);
    }

    uint64_t read_frames = 0;
    while (1)
    {
        uint64_t temp = 0;
        temp = drwav_read_pcm_frames_s16(&wav, 128, pcmBuffer);
        if (temp > 0) {
            // Serial.printf("read %llu frames\r\n", temp);
            writeBatchCircularBuffer(&audioStreamBuf, (const uint8_t *)pcmBuffer, temp * wav.channels * (wav.bitsPerSample / 8));
            read_frames += temp;
        } else {
            break;
        }
        // Serial.printf("🟢 剩余栈空间: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
    
    Serial.printf("PCM 数据读取完成！Total read frames: %llu \r\n", read_frames);

    // 清理资源
    free(pcmBuffer);
    drwav_uninit(&wav);
    // vTaskDelay(NULL);
    while(1) {
        Serial.printf("🟢 剩余栈空间: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskSuspend(NULL);
    }
}
#endif

void on_i2s_send_callback() {

    if (read_frames < wav.totalPCMFrameCount) {
        uint64_t temp = 0;
        temp = drwav_read_pcm_frames_s16(&wav, FRAMES_SIZE, pcmBuffer);
        if (temp > 0) {
            // Serial.printf("read %llu frames\r\n", temp);
            writeBatchCircularBuffer(&audioStreamBuf, (const uint8_t *)pcmBuffer, temp * wav.channels * (wav.bitsPerSample / 8));
            read_frames += temp;
        } else {
            Serial.println("播放结束");
        }
    }
}

void audio_play_task2(void *pvParameter)
{
    // 使用 dr_wav 解析 WAV 文件
    // drwav wav;
    if (!drwav_init_file(&wav, "/spiffs/16000_2_16.wav", NULL)) {
        Serial.println("WAV 解析失败！");
        return;
    }

    Serial.println("WAV 解析成功！");

    Serial.printf("采样率: %d Hz\n", wav.sampleRate);
    Serial.printf("声道数: %d\n", wav.channels);
    Serial.printf("位深度: %d-bit\n", wav.bitsPerSample);
    Serial.printf("总帧数: %llu\n", wav.totalPCMFrameCount);

    // 读取 PCM 数据
    size_t dataSize = wav.totalPCMFrameCount * wav.channels * (wav.bitsPerSample / 8);
    pcmBuffer = (int16_t *)malloc(FRAMES_SIZE * wav.channels * (wav.bitsPerSample / 8));
    if (!pcmBuffer) {
        Serial.println("内存分配失败！");
        drwav_uninit(&wav);
        return;
    }

    // on_i2s_send_callback();

    Serial.println("内存分配成功！");

    uint64_t temp = 0;
    temp = drwav_read_pcm_frames_s16(&wav, FRAMES_SIZE, pcmBuffer);
    if (temp > 0) {
        // Serial.printf("read %llu frames\r\n", temp);
        writeBatchCircularBuffer(&audioStreamBuf, (const uint8_t *)pcmBuffer, temp * wav.channels * (wav.bitsPerSample / 8));
        read_frames += temp;
    } else {
        return;
    }

    while(1) {
        Serial.printf("🟢 剩余栈空间: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskSuspend(NULL);
    }
} 

void setup() {
    Serial.begin(115200);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // 初始化 SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS 初始化失败！");
        return;
    }

    max98357Setup(on_i2s_send_callback);

    // xTaskCreate(audio_play_task, 
    //             "Audio Play Task", 
    //             14240, 
    //             NULL, 
    //             4, 
    //             NULL);

    max98357Start();

    xTaskCreate(audio_play_task2, 
                "Audio Play Task", 
                14240, 
                NULL, 
                4, 
                NULL);


#if 0
    // 使用 dr_wav 解析 WAV 文件
    // drwav wav;
    if (!drwav_init_file(&wav, "/spiffs/16000_2_16.wav", NULL)) {
        Serial.println("WAV 解析失败！");
        return;
    }

    Serial.println("WAV 解析成功！");

    Serial.printf("采样率: %d Hz\n", wav.sampleRate);
    Serial.printf("声道数: %d\n", wav.channels);
    Serial.printf("位深度: %d-bit\n", wav.bitsPerSample);
    Serial.printf("总帧数: %llu\n", wav.totalPCMFrameCount);

    // 读取 PCM 数据
    // size_t dataSize = wav.totalPCMFrameCount * wav.channels * (wav.bitsPerSample / 8);
    int16_t *pcmBuffer = (int16_t *)malloc(FRAMES_SIZE * wav.channels * (wav.bitsPerSample / 8));
    if (!pcmBuffer) {
        Serial.println("内存分配失败！");
        drwav_uninit(&wav);
        return;
    }

    Serial.println("内存分配成功！");

    uint64_t temp = 0;
    temp = drwav_read_pcm_frames_s16(&wav, FRAMES_SIZE, pcmBuffer);
    if (temp > 0) {
        // Serial.printf("read %llu frames\r\n", temp);
        writeBatchCircularBuffer(&audioStreamBuf, (const uint8_t *)pcmBuffer, temp * wav.channels * (wav.bitsPerSample / 8));
        read_frames += temp;
    } else {
        return;
    }

    // drwav_read_pcm_frames_s16(&wav, 128, pcmBuffer);

    // uint64_t read_frames = 0;
    // while (1)
    // {
    //     uint64_t temp = 0;
    //     temp = drwav_read_pcm_frames_s16(&wav, 512, pcmBuffer);
    //     if (temp > 0) {
    //         Serial.printf("read %llu frames\r\n", temp);
    //         read_frames += temp;
    //     }
    //     vTaskDelay(1 / portTICK_PERIOD_MS);
    // }

#endif
}

void loop() {
    // 空循环
}
