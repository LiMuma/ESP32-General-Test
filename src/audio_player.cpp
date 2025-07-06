#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h> 

#define I2S_SAMPLE_RATE 44100 // Sample rate in Hz
#define I2S_DOUT 7
#define I2S_BCK 15
#define I2S_LRC 16

// 外部函数声明
void showStaticMessage(const char* message);
void showTwoLineMessage(const char* line1, const char* line2);

// 来自microphone.cpp的外部函数
extern const int16_t* getRecordingData();
extern size_t getRecordingLength();
extern bool hasRecordingData();

// 播放器状态
static bool audioPlayerInitialized = false;

void playSineTone(float frequency, int duration_ms) {
    if (!audioPlayerInitialized) {
        Serial.println("音频播放器未初始化");
        return;
    }
    
    const int amplitude = 3000;
    const int samples = I2S_SAMPLE_RATE * duration_ms / 1000;
    const float increment = (2 * M_PI * frequency) / I2S_SAMPLE_RATE;
    
    for (int i = 0; i < samples; i++) {
        double sample = sin(i * increment);
        int16_t value = (int16_t)(amplitude * sample);
        int32_t sample_stereo = (value << 16) | (value & 0xFFFF); // Create stereo sample
        size_t bytes_written;
        i2s_write(I2S_NUM_0, &sample_stereo, sizeof(sample_stereo), &bytes_written, portMAX_DELAY);
    }
}

bool initAudio() {
    Serial.println("Audio module initialization...");
    showStaticMessage("初始化音频播放器");
    
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0, // Default interrupt flags
        .dma_buf_count = 8, // Number of DMA buffers
        .dma_buf_len = 1024, // Length of each DMA buffer
        .use_apll = false, // Do not use APLL
        .tx_desc_auto_clear = true, // Auto clear TX descriptor on underflow
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK, // Bit Clock pin
        .ws_io_num = I2S_LRC, // Word Select pin
        .data_out_num = I2S_DOUT, // Data Out pin
        .data_in_num = I2S_PIN_NO_CHANGE, // No Data In pin
    };

    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("I2S播放器驱动安装失败: %s\n", esp_err_to_name(err));
        showStaticMessage("播放器初始化失败");
        return false;
    }
    
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("I2S播放器引脚设置失败: %s\n", esp_err_to_name(err));
        showStaticMessage("播放器初始化失败");
        return false;
    }
    
    err = i2s_start(I2S_NUM_0);
    if (err != ESP_OK) {
        Serial.printf("I2S播放器启动失败: %s\n", esp_err_to_name(err));
        showStaticMessage("播放器初始化失败");
        return false;
    }
    
    audioPlayerInitialized = true;
    
    // 播放测试音调
    Serial.println("播放测试音调...");
    playSineTone(440.0, 1000); // Play a 440 Hz sine tone for 1 second
    delay(500);
    playSineTone(880.0, 500);  // Play a 880 Hz sine tone for 0.5 second
    
    Serial.println("音频播放器初始化成功");
    showStaticMessage("播放器初始化成功");
    return true;
}

void playAudio() {
    Serial.println("Playing audio...");
    showStaticMessage("播放中...");
    
    // 播放音频代码将在此处添加
    // 目前只是占位符
    
    delay(1000);
    showStaticMessage("播放完成");
}

// 播放录音数据
void playRecording() {
    if (!audioPlayerInitialized) {
        Serial.println("音频播放器未初始化");
        showStaticMessage("播放器未初始化");
        return;
    }
    
    if (!hasRecordingData()) {
        Serial.println("没有录音数据可播放");
        showStaticMessage("没有录音数据");
        return;
    }
    
    const int16_t* recordingData = getRecordingData();
    size_t recordingLength = getRecordingLength();
    
    if (recordingData == nullptr || recordingLength == 0) {
        Serial.println("录音数据无效");
        showStaticMessage("录音数据无效");
        return;
    }
    
    Serial.printf("开始播放录音，长度: %zu 样本\n", recordingLength);
    
    // 调试：检查录音数据的前几个样本
    Serial.println("录音数据前10个样本:");
    for (size_t i = 0; i < min(recordingLength, (size_t)10); i++) {
        Serial.printf("  样本[%zu]: %d\n", i, recordingData[i]);
    }
    
    // 检查录音数据是否有有效信号
    float maxAmplitude = 0;
    float avgAmplitude = 0;
    for (size_t i = 0; i < recordingLength; i++) {
        float amplitude = abs(recordingData[i]) / 32768.0;
        avgAmplitude += amplitude;
        if (amplitude > maxAmplitude) {
            maxAmplitude = amplitude;
        }
    }
    avgAmplitude /= recordingLength;
    
    Serial.printf("录音统计 - 最大音量: %.4f, 平均音量: %.4f\n", maxAmplitude, avgAmplitude);
    
    if (maxAmplitude < 0.001) {
        Serial.println("警告: 录音数据音量极低，可能没有录到有效声音");
        showTwoLineMessage("警告:", "录音音量极低");
        delay(2000);
    } else if (maxAmplitude > 0.8) {
        Serial.println("警告: 录音数据可能有削波失真");
        showTwoLineMessage("警告:", "录音可能失真");
        delay(2000);
    }
    
    showTwoLineMessage("播放录音中...", "请听");
    
    // 播放录音数据
    for (size_t i = 0; i < recordingLength; i++) {
        int16_t sample = recordingData[i];
        
        // 创建立体声样本 (左右声道相同)
        int32_t sample_stereo = (sample << 16) | (sample & 0xFFFF);
        
        size_t bytes_written;
        esp_err_t err = i2s_write(I2S_NUM_0, &sample_stereo, sizeof(sample_stereo), &bytes_written, portMAX_DELAY);
        
        if (err != ESP_OK) {
            Serial.printf("播放失败: %s\n", esp_err_to_name(err));
            showStaticMessage("播放失败");
            return;
        }
        
        // 每1000个样本显示一次进度
        if (i % (I2S_SAMPLE_RATE / 2) == 0) {  // 每0.5秒显示一次
            float progress = (float)i / recordingLength * 100.0;
            char line1[32], line2[32];
            sprintf(line1, "播放中...");
            sprintf(line2, "进度: %.1f%%", progress);
            showTwoLineMessage(line1, line2);
        }
    }
    
    Serial.println("录音播放完成");
    showStaticMessage("播放完成");
}

// 播放录音数据（带音量增强）
void playRecordingWithGain(float gain) {
    if (!audioPlayerInitialized) {
        Serial.println("音频播放器未初始化");
        showStaticMessage("播放器未初始化");
        return;
    }
    
    if (!hasRecordingData()) {
        Serial.println("没有录音数据可播放");
        showStaticMessage("没有录音数据");
        return;
    }
    
    const int16_t* recordingData = getRecordingData();
    size_t recordingLength = getRecordingLength();
    
    if (recordingData == nullptr || recordingLength == 0) {
        Serial.println("录音数据无效");
        showStaticMessage("录音数据无效");
        return;
    }
    
    Serial.printf("开始播放录音(增益: %.2f)，长度: %zu 样本\n", gain, recordingLength);
    showTwoLineMessage("播放录音中...", "增强音量");
    
    // 播放录音数据
    for (size_t i = 0; i < recordingLength; i++) {
        int16_t sample = recordingData[i];
        
        // 应用增益
        float amplifiedSample = sample * gain;
        
        // 防止溢出
        if (amplifiedSample > 32767) amplifiedSample = 32767;
        if (amplifiedSample < -32768) amplifiedSample = -32768;
        
        int16_t finalSample = (int16_t)amplifiedSample;
        
        // 创建立体声样本 (左右声道相同)
        int32_t sample_stereo = (finalSample << 16) | (finalSample & 0xFFFF);
        
        size_t bytes_written;
        esp_err_t err = i2s_write(I2S_NUM_0, &sample_stereo, sizeof(sample_stereo), &bytes_written, portMAX_DELAY);
        
        if (err != ESP_OK) {
            Serial.printf("播放失败: %s\n", esp_err_to_name(err));
            showStaticMessage("播放失败");
            return;
        }
        
        // 每1000个样本显示一次进度
        if (i % (I2S_SAMPLE_RATE / 2) == 0) {  // 每0.5秒显示一次
            float progress = (float)i / recordingLength * 100.0;
            char line1[32], line2[32];
            sprintf(line1, "播放中...");
            sprintf(line2, "进度: %.1f%%", progress);
            showTwoLineMessage(line1, line2);
        }
    }
    
    Serial.println("录音播放完成");
    showStaticMessage("播放完成");
}

// 检查播放器状态
bool isAudioPlayerReady() {
    return audioPlayerInitialized;
}

// 关闭播放器
void deinitAudio() {
    if (audioPlayerInitialized) {
        i2s_stop(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_0);
        audioPlayerInitialized = false;
        Serial.println("音频播放器已关闭");
    }
}
