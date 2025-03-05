#ifndef __MY_RINGBUFFER__
#define __MY_RINGBUFFER__

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // 引入stdint.h以使用uint8_t

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h> // 引入 FreeRTOS 信号量头文件

typedef struct {
    uint8_t *buffer;     // 数据存储区域，类型改为uint8_t
    size_t head;         // 写入位置
    size_t tail;         // 读取位置
    size_t max;          // 缓冲区最大容量
    int full;            // 标识是否已满
    size_t currentLength;// 当前存储的元素长度
    SemaphoreHandle_t mutex; // 互斥量用于保护缓冲区
} CircularBuffer;

// 初始化环形缓冲区
void initCircularBuffer(CircularBuffer *cb, size_t size) {
    cb->buffer = (uint8_t *)malloc(size * sizeof(uint8_t));
    if (cb->buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer.\n");
        exit(EXIT_FAILURE);
    }
    cb->max = size;
    cb->head = 0;
    cb->tail = 0;
    cb->full = 0;
    cb->currentLength = 0;

    // 创建互斥量
    cb->mutex = xSemaphoreCreateMutex();
    if (cb->mutex == NULL) {
        fprintf(stderr, "Failed to create mutex.\n");
        free(cb->buffer);
        exit(EXIT_FAILURE);
    }
}

// 清理环形缓冲区
void cleanupCircularBuffer(CircularBuffer *cb) {
    vSemaphoreDelete(cb->mutex); // 删除互斥量
    free(cb->buffer);
}

// 将单个数据写入环形缓冲区
int writeCircularBuffer(CircularBuffer *cb, uint8_t data) {
    if (cb->full) {
        // 如果缓冲区已满，则无法写入新数据
        return -1;
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % cb->max; // 使用模运算来处理回绕

    if (cb->head == cb->tail) {
        cb->full = 1; // 标记为满
    } else {
        cb->currentLength++; // 更新当前长度
    }

    return 0;
}

// 批量写入数据到环形缓冲区
int writeBatchCircularBuffer(CircularBuffer *cb, const uint8_t *data, size_t length) {
    // 获取互斥量
    if (xSemaphoreTake(cb->mutex, portMAX_DELAY) != pdTRUE) {
        return -1; // 如果无法获取互斥量，则返回错误
    }

    for (size_t i = 0; i < length; ++i) {
        if (writeCircularBuffer(cb, data[i]) != 0) {
            // 释放互斥量
            xSemaphoreGive(cb->mutex);
            return -1; // 如果无法写入更多数据，返回错误
        }
    }

    // 释放互斥量
    xSemaphoreGive(cb->mutex);

    return 0;
}

// 批量读取数据从环形缓冲区
int readBatchCircularBuffer(CircularBuffer *cb, uint8_t *data, size_t length) {
    // 获取互斥量
    if (xSemaphoreTake(cb->mutex, portMAX_DELAY) != pdTRUE) {
        return -1; // 如果无法获取互斥量，则返回错误
    }

    size_t available = cb->full ? cb->max : (cb->head >= cb->tail ? cb->head - cb->tail : cb->max - (cb->tail - cb->head));

    if (available < length) {
        // 释放互斥量
        xSemaphoreGive(cb->mutex);
        return -1; // 如果请求的数据长度超过可用数据量，则返回错误
    }

    for (size_t i = 0; i < length; ++i) {
        data[i] = cb->buffer[cb->tail];
        cb->tail = (cb->tail + 1) % cb->max; // 使用模运算来处理回绕
    }

    cb->full = 0; // 只要读取了数据，就不可能是满的
    cb->currentLength -= length; // 更新当前长度

    // 释放互斥量
    xSemaphoreGive(cb->mutex);

    return 0;
}

// 获取环形缓冲区当前元素长度
size_t getCurrentLength(const CircularBuffer *cb) {
    // 获取互斥量
    if (xSemaphoreTake(cb->mutex, portMAX_DELAY) != pdTRUE) {
        return 0; // 如果无法获取互斥量，则返回错误值
    }

    size_t length = cb->currentLength;

    // 释放互斥量
    xSemaphoreGive(cb->mutex);

    return length;
}

// 打印环形缓冲区相关信息
void printCircularBufferInfo(const CircularBuffer *cb) {
    // Serial.printf("Circular Buffer Information:\r\n");
    // Serial.printf("Max Size: %zu\r\n", cb->max);
    // Serial.printf("Head Position: %zu\r\n", cb->head);
    // Serial.printf("Tail Position: %zu\r\n", cb->tail);
    // Serial.printf("Is Full: %s\r\n", cb->full ? "Yes" : "No");

    // size_t count;
    // if (cb->full) {
    //     count = cb->max;
    // } else if (cb->head >= cb->tail) {
    //     count = cb->head - cb->tail;
    // } else {
    //     count = cb->max - (cb->tail - cb->head);
    // }
    // printf("Current Data Count: %zu\n", count);
    Serial.printf("Max Size[%zu] Current Data Count[%zu]\r\n", cb->max, cb->currentLength);

    // // 打印缓冲区内容（可选）
    // printf("Buffer Contents:\n");
    // for (size_t i = 0; i < cb->max; ++i) {
    //     size_t index = (cb->tail + i) % cb->max;
    //     printf("%u ", cb->buffer[index]);
    //     if ((i + 1) % 32 == 0) { // 每32个元素换行一次，便于查看
    //         printf("\n");
    //     }
    // }
    // printf("\n");
}

#if 0
// 示例：如何使用上述接口
int main() {
    CircularBuffer cb;
    initCircularBuffer(&cb, BUFFER_SIZE);

    // 准备一些要批量写入的数据
    uint8_t dataToAdd[BUFFER_SIZE / 2];
    for (size_t i = 0; i < BUFFER_SIZE / 2; ++i) {
        dataToAdd[i] = (uint8_t)i;
    }

    // 批量写入数据
    if (writeBatchCircularBuffer(&cb, dataToAdd, BUFFER_SIZE / 2) != 0) {
        printf("Failed to write batch data to buffer.\n");
    } else {
        printf("Successfully wrote batch data to buffer.\n");
    }

    // 打印缓冲区信息
    printCircularBufferInfo(&cb);

    // 批量读取数据
    uint8_t readData[BUFFER_SIZE / 4]; // 假设我们只读取四分之一的数据
    if (readBatchCircularBuffer(&cb, readData, BUFFER_SIZE / 4) != 0) {
        printf("Failed to read batch data from buffer.\n");
    } else {
        printf("Successfully read batch data from buffer.\n");
        for (size_t i = 0; i < BUFFER_SIZE / 4; ++i) {
            printf("Read %u from buffer\n", readData[i]);
        }
    }

    // 再次打印缓冲区信息
    printCircularBufferInfo(&cb);

    cleanupCircularBuffer(&cb); // 清理资源
    return 0;
}

#endif


#endif // __MY_RINGBUFFER__