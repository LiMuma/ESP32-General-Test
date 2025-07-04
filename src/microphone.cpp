#include <Arduino.h>
#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// I2S麦克风引脚定义
#define I2S_MIC_SCK_PIN     18  // SCK (Serial Clock)
#define I2S_MIC_WS_PIN      42   // WS (Word Select/LRC)
#define I2S_MIC_SD_PIN      17  // SD (Serial Data)

// I2S配置参数
#define I2S_SAMPLE_RATE     44100  // 采样率 44.1kHz (与播放器统一)
#define I2S_BUFFER_SIZE     1024   // 缓冲区大小
#define I2S_DMA_BUF_COUNT   8      // DMA缓冲区数量
#define I2S_DMA_BUF_LEN     512    // DMA缓冲区长度

// 录音存储相关
#define MAX_RECORDING_SAMPLES (I2S_SAMPLE_RATE * 3)  // 最大录音3秒
static int16_t recordingBuffer[MAX_RECORDING_SAMPLES];
static size_t recordingLength = 0;
static bool hasRecording = false;

// 全局变量
static bool microphoneInitialized = false;
static int16_t audioBuffer[I2S_BUFFER_SIZE];

// 外部函数声明
void showStaticMessage(const char* message);
void showTwoLineMessage(const char* line1, const char* line2);

// I2S麦克风初始化
bool initMicrophone() {
    Serial.println("=== 初始化I2S麦克风 ===");
    showStaticMessage("初始化麦克风...");
    
    // I2S配置
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // 主模式，接收
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // 使用32位接收I2S麦克风数据
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // 只使用左声道
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    // I2S引脚配置
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_MIC_SCK_PIN,    // SCK
        .ws_io_num = I2S_MIC_WS_PIN,      // WS
        .data_out_num = I2S_PIN_NO_CHANGE, // 不使用输出
        .data_in_num = I2S_MIC_SD_PIN     // SD (数据输入)
    };
    
    // 安装I2S驱动
    esp_err_t err = i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("I2S驱动安装失败: %s\n", esp_err_to_name(err));
        showStaticMessage("麦克风初始化失败");
        return false;
    }
    
    // 设置I2S引脚
    err = i2s_set_pin(I2S_NUM_1, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("I2S引脚设置失败: %s\n", esp_err_to_name(err));
        showStaticMessage("麦克风初始化失败");
        return false;
    }
    
    // 开始I2S
    err = i2s_start(I2S_NUM_1);
    if (err != ESP_OK) {
        Serial.printf("I2S启动失败: %s\n", esp_err_to_name(err));
        showStaticMessage("麦克风初始化失败");
        return false;
    }
    
    microphoneInitialized = true;
    Serial.println("麦克风初始化成功");
    showStaticMessage("麦克风初始化成功");
    
    return true;
}

// 读取音频数据
size_t readAudioData(int16_t* buffer, size_t bufferSize) {
    if (!microphoneInitialized) {
        Serial.println("麦克风未初始化");
        return 0;
    }
    
    // 使用32位缓冲区接收I2S数据
    int32_t rawBuffer[bufferSize];
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_NUM_1, rawBuffer, bufferSize * sizeof(int32_t), &bytesRead, portMAX_DELAY);
    
    if (err != ESP_OK) {
        Serial.printf("I2S读取失败: %s\n", esp_err_to_name(err));
        return 0;
    }
    
    size_t samplesRead = bytesRead / sizeof(int32_t);
    
    // 将32位数据转换为16位数据
    for (size_t i = 0; i < samplesRead && i < bufferSize; i++) {
        // 尝试不同的位移量，根据麦克风类型调整
        // 大多数I2S麦克风使用高16位
        int32_t sample32 = rawBuffer[i];
        
        // 方法1: 取高16位 (最常用)
        int16_t sample16 = (int16_t)(sample32 >> 16);
        
        // 简单的噪声抑制：如果信号很小，设为0
        if (abs(sample16) < 100) {  // 调整这个阈值来减少背景噪声
            sample16 = 0;
        }
        
        buffer[i] = sample16;
        
        // 如果音质不好，可以尝试其他方法：
        // 方法2: 取低16位
        // buffer[i] = (int16_t)(sample32 & 0xFFFF);
        
        // 方法3: 取中间16位  
        // buffer[i] = (int16_t)(sample32 >> 8);
        
        // 方法4: 24位到16位转换
        // buffer[i] = (int16_t)(sample32 >> 8);
    }
    
    return samplesRead;  // 返回样本数量
}

// 录音并存储
void recordAudio(int durationMs) {
    if (!microphoneInitialized) {
        Serial.println("麦克风未初始化，无法录音");
        showStaticMessage("麦克风未初始化");
        return;
    }
    
    recordingLength = 0;
    hasRecording = false;
    
    Serial.printf("开始录音，持续 %d 毫秒\n", durationMs);
    showTwoLineMessage("录音中...", "请说话");
    
    unsigned long startTime = millis();
    unsigned long sampleCount = 0;
    
    while (millis() - startTime < durationMs && recordingLength < MAX_RECORDING_SAMPLES) {
        size_t samplesRead = readAudioData(audioBuffer, I2S_BUFFER_SIZE);
        
        if (samplesRead > 0) {
            // 存储录音数据
            size_t samplesToStore = min(samplesRead, MAX_RECORDING_SAMPLES - recordingLength);
            
            for (size_t i = 0; i < samplesToStore; i++) {
                recordingBuffer[recordingLength + i] = audioBuffer[i];
            }
            
            recordingLength += samplesToStore;
            sampleCount += samplesRead;
            
            // 如果存储空间用完了，退出录音
            if (recordingLength >= MAX_RECORDING_SAMPLES) {
                Serial.println("录音存储空间已满，停止录音");
                break;
            }
        }
        
        vTaskDelay(1);  // 给其他任务让出CPU时间
    }
    
    hasRecording = (recordingLength > 0);
    
    Serial.printf("录音完成:\n");
    Serial.printf("  录音时长: %.2f 秒\n", (float)recordingLength / I2S_SAMPLE_RATE);
    Serial.printf("  采样数量: %zu\n", recordingLength);
    Serial.printf("  存储状态: %s\n", hasRecording ? "成功" : "失败");
    
    // 调试：显示录音数据的前几个样本
    if (hasRecording && recordingLength > 0) {
        Serial.println("录音数据前10个样本:");
        for (size_t i = 0; i < min(recordingLength, (size_t)10); i++) {
            Serial.printf("  样本[%zu]: %d\n", i, recordingBuffer[i]);
        }
        
        // 计算录音的音频统计
        float maxAmp = 0, avgAmp = 0;
        for (size_t i = 0; i < recordingLength; i++) {
            float amp = abs(recordingBuffer[i]) / 32768.0;
            avgAmp += amp;
            if (amp > maxAmp) maxAmp = amp;
        }
        avgAmp /= recordingLength;
        
        Serial.printf("录音统计 - 最大音量: %.4f, 平均音量: %.4f\n", maxAmp, avgAmp);
        
        if (maxAmp < 0.001) {
            Serial.println("警告: 录音音量极低，可能麦克风未工作");
        } else if (maxAmp > 0.9) {
            Serial.println("警告: 录音可能有削波失真");
        } else {
            Serial.println("录音音量正常");
        }
    }
    
    char line1[32], line2[32];
    sprintf(line1, "录音完成");
    sprintf(line2, "时长:%.1fs", (float)recordingLength / I2S_SAMPLE_RATE);
    showTwoLineMessage(line1, line2);
}

// 录音测试函数
void testMicrophoneRecording(int durationMs) {
    if (!microphoneInitialized) {
        Serial.println("麦克风未初始化，无法录音");
        showStaticMessage("麦克风未初始化");
        return;
    }
    
    Serial.printf("开始录音测试，持续 %d 毫秒\n", durationMs);
    showTwoLineMessage("录音测试中...", "请说话");
    
    unsigned long startTime = millis();
    unsigned long sampleCount = 0;
    float maxAmplitude = 0;
    float totalAmplitude = 0;
    
    while (millis() - startTime < durationMs) {
        size_t samplesRead = readAudioData(audioBuffer, I2S_BUFFER_SIZE);
        
        if (samplesRead > 0) {
            // 计算音频统计信息
            for (size_t i = 0; i < samplesRead; i++) {
                float amplitude = abs(audioBuffer[i]) / 32768.0;  // 标准化到0-1
                totalAmplitude += amplitude;
                if (amplitude > maxAmplitude) {
                    maxAmplitude = amplitude;
                }
            }
            sampleCount += samplesRead;
        }
        
        vTaskDelay(1);  // 给其他任务让出CPU时间
    }
    
    // 计算平均音量
    float averageAmplitude = (sampleCount > 0) ? (totalAmplitude / sampleCount) : 0;
    
    Serial.printf("录音测试完成:\n");
    Serial.printf("  采样数量: %lu\n", sampleCount);
    Serial.printf("  平均音量: %.4f\n", averageAmplitude);
    Serial.printf("  最大音量: %.4f\n", maxAmplitude);
    
    char line1[32], line2[32];
    sprintf(line1, "录音完成");
    sprintf(line2, "音量:%.2f", averageAmplitude);
    showTwoLineMessage(line1, line2);
}

// 实时音量监测
void monitorAudioLevel(int durationMs) {
    if (!microphoneInitialized) {
        Serial.println("麦克风未初始化");
        return;
    }
    
    Serial.printf("开始音量监测，持续 %d 毫秒\n", durationMs);
    showTwoLineMessage("音量监测中...", "");
    
    unsigned long startTime = millis();
    unsigned long lastDisplayTime = 0;
    
    while (millis() - startTime < durationMs) {
        size_t samplesRead = readAudioData(audioBuffer, I2S_BUFFER_SIZE);
        
        if (samplesRead > 0) {
            // 计算当前音量
            float currentAmplitude = 0;
            for (size_t i = 0; i < samplesRead; i++) {
                currentAmplitude += abs(audioBuffer[i]) / 32768.0;
            }
            currentAmplitude /= samplesRead;
            
            // 每500ms更新一次显示
            if (millis() - lastDisplayTime > 500) {
                char line1[32], line2[32];
                sprintf(line1, "音量监测");
                sprintf(line2, "当前: %.3f", currentAmplitude);
                showTwoLineMessage(line1, line2);
                
                Serial.printf("当前音量: %.3f\n", currentAmplitude);
                lastDisplayTime = millis();
            }
        }
        
        vTaskDelay(10);
    }
    
    showStaticMessage("监测完成");
}

// 获取录音数据
const int16_t* getRecordingData() {
    return hasRecording ? recordingBuffer : nullptr;
}

// 获取录音长度
size_t getRecordingLength() {
    return hasRecording ? recordingLength : 0;
}

// 检查是否有录音
bool hasRecordingData() {
    return hasRecording;
}

// 清除录音数据
void clearRecording() {
    hasRecording = false;
    recordingLength = 0;
    Serial.println("录音数据已清除");
}

// 麦克风去初始化
void deinitMicrophone() {
    if (microphoneInitialized) {
        i2s_stop(I2S_NUM_1);
        i2s_driver_uninstall(I2S_NUM_1);
        microphoneInitialized = false;
        Serial.println("麦克风已关闭");
    }
}

// 检查麦克风状态
bool isMicrophoneReady() {
    return microphoneInitialized;
}

// 获取麦克风采样率
int getMicrophoneSampleRate() {
    return I2S_SAMPLE_RATE;
} 