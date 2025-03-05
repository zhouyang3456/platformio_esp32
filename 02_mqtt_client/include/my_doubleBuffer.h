#ifndef __MY_DOUBLEBUFFER__
#define __MY_DOUBLEBUFFER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define BUFFER_SIZE 1024 // 初始缓冲区大小
#define DATA_TYPE int16_t // 数据类型

// 定义双缓冲结构体
typedef struct {
    DATA_TYPE *buffer1; // 动态分配的第一个缓冲区
    DATA_TYPE *buffer2; // 动态分配的第二个缓冲区
    size_t writeIndex; // 写入索引
    size_t bufferSize; // 缓冲区大小
    int currentBuffer; // 当前写入缓冲区: 0 表示 buffer1, 1 表示 buffer2
} DoubleBuffer;

// 初始化双缓冲区
void initDoubleBuffer(DoubleBuffer *db, size_t initialSize) {
    db->bufferSize = initialSize;
    db->buffer1 = (DATA_TYPE *)malloc(initialSize * sizeof(DATA_TYPE));
    if (db->buffer1 == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer1.\n");
        exit(EXIT_FAILURE);
    }
    db->buffer2 = (DATA_TYPE *)malloc(initialSize * sizeof(DATA_TYPE));
    if (db->buffer2 == NULL) {
        free(db->buffer1);
        fprintf(stderr, "Failed to allocate memory for buffer2.\n");
        exit(EXIT_FAILURE);
    }
    db->writeIndex = 0;
    db->currentBuffer = 0; // 默认从第一个缓冲区开始写入
}

// 向缓冲区添加数据
int addDataToBuffer(DoubleBuffer *db, const DATA_TYPE *data, size_t dataLength) {
    if (dataLength > db->bufferSize - db->writeIndex) {
        return -1; // 没有足够的空间
    }

    // 根据当前缓冲区选择目标缓冲区
    DATA_TYPE *targetBuffer = db->currentBuffer ? db->buffer2 : db->buffer1;

    // 将数据复制到缓冲区
    memcpy(targetBuffer + db->writeIndex, data, dataLength * sizeof(DATA_TYPE));

    // 更新写入索引
    db->writeIndex += dataLength;

    return 0;
}

// 检查当前缓冲区是否已满
int isBufferFull(DoubleBuffer *db) {
    return db->writeIndex >= db->bufferSize;
}

// 返回当前使用的缓冲区的地址，并切换缓冲区
DATA_TYPE* getCurrentBufferAndSwap(DoubleBuffer *db, size_t *size) {
    if (isBufferFull(db)) {
        // 返回当前使用的缓冲区地址
        DATA_TYPE *currentBuffer = db->currentBuffer ? db->buffer1 : db->buffer2;
        *size = db->writeIndex;

        // 重置写入索引
        db->writeIndex = 0;

        // 切换当前缓冲区
        db->currentBuffer = !db->currentBuffer;

        return currentBuffer;
    } else {
        return NULL;
    }
}

// 清理资源
void cleanup(DoubleBuffer *db) {
    free(db->buffer1);
    free(db->buffer2);
}

#if 0
// 示例：如何使用上述接口
int main() {
    DoubleBuffer db;
    initDoubleBuffer(&db, BUFFER_SIZE);

    // 准备一些要添加的数据
    DATA_TYPE dataToAdd[] = {1, 2, 3, 4, 5};
    size_t dataLength = sizeof(dataToAdd) / sizeof(DATA_TYPE);

    // 向缓冲区添加数据
    if (addDataToBuffer(&db, dataToAdd, dataLength) != 0) {
        printf("Failed to add data to buffer.\n");
        cleanup(&db);
        return EXIT_FAILURE;
    }

    size_t readableSize;
    const DATA_TYPE *readableBuffer = getCurrentBufferAndSwap(&db, &readableSize);
    if (readableBuffer != NULL) {
        // 打印当前可读取的缓冲区内容
        for (size_t i = 0; i < readableSize; ++i) {
            printf("%d ", readableBuffer[i]);
        }
        printf("\n");
    } else {
        printf("Buffer is not full yet.\n");
    }

    cleanup(&db); // 清理资源
    return EXIT_SUCCESS;
}
#endif

#endif // __MY_DOUBLEBUFFER__