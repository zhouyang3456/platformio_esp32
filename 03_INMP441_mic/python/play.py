import serial
import pyaudio
import numpy as np

# 串口参数
SERIAL_PORT = 'COM3'  # Windows: 'COM3'，Linux/Mac: '/dev/ttyUSB0'
BAUD_RATE = 921600    # ESP32 串口波特率

# 音频参数
SAMPLE_RATE = 44100  # 采样率 8kHz
CHANNELS = 1        # 单声道
SAMPLE_WIDTH = 2    # 16-bit 音频（2字节）
CHUNK_SIZE = 1024   # 每次读取的字节数
VOLUME_GAIN = 10.0   # 音量放大倍数

# 初始化串口
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# 发送 "start" 命令，通知 ESP32 开始发送音频数据
ser.write(b'start\n')

# 等待 ESP32 回复 "OK"
while True:
    response = ser.readline().decode().strip()
    if response == "OK":
        print("ESP32 准备好，开始接收音频...")
        break

# 初始化音频播放
p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16,
                channels=CHANNELS,
                rate=SAMPLE_RATE,
                output=True)

print("开始播放音频...")

try:
    while True:
        data = ser.read(CHUNK_SIZE)  # 从串口读取数据
        if len(data) == CHUNK_SIZE:
            # 解析 16-bit PCM 数据
            audio_data = np.frombuffer(data, dtype=np.int16)
            
            # 放大音量
            audio_data = np.clip(audio_data * VOLUME_GAIN, -32768, 32767).astype(np.int16)
            
            # 播放放大后的音频
            stream.write(audio_data.tobytes())
except KeyboardInterrupt:
    print("停止播放")

# 发送 "stop" 命令，通知 ESP32 停止发送音频数据
ser.write(b'stop\n')

# 关闭资源
stream.stop_stream()
stream.close()
p.terminate()
ser.close()
