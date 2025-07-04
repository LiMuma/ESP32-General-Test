#include <Arduino.h>
#include <WiFi.h>

// 基本显示函数
void initScreen();
void showStaticMessage(const char* message);
void showTwoLineMessage(const char* line1, const char* line2);


// 摄像头功能
void initCamera();
void captureImage();
String captureAndAnalyze(const String& question);
void captureAndDisplayOnOled();  // 新增：拍照并显示到OLED
void continuousCaptureDisplay(); // 新增：连续拍照显示功能

// 麦克风功能
bool initMicrophone();
void testMicrophoneRecording(int durationMs);
void monitorAudioLevel(int durationMs);
void deinitMicrophone();
bool isMicrophoneReady();
int getMicrophoneSampleRate();

// AI视觉分析功能
String resolveImage(const String& imageBase64, const String& question);
String resolveImageFromURL(const String& imageUrl, const String& question);
bool testAPIConnection();

// 网络功能
void initWifi();
bool testInternetConnectivity();
bool testDashScopeConnectivity();
void showNetworkStatus();

// 音频播放功能
bool initAudio();
void playRecording();
void playRecordingWithGain(float gain);
bool isAudioPlayerReady();
void deinitAudio();
void playSineTone(float frequency, int duration_ms);

// 录音存储功能
void recordAudio(int durationMs);
bool hasRecordingData();
void clearRecording();

void testImageResolution();
void testImageDisplayFeatures(); // 新增：图像显示功能测试
void testMicrophoneFeatures(); // 新增：麦克风功能测试
void testRecordingPlayback(); // 新增：录音播放功能测试

void setup() {
    Serial.begin(115200);
    Serial.println("=== ESP32 AI Vision System ===");
    
    // 初始化屏幕
    initScreen();
    delay(2000);

    // 初始化音频播放器
    showStaticMessage("初始化音频播放器...");
    if (initAudio()) {
        showStaticMessage("播放器初始化成功");
        Serial.println("音频播放器初始化成功");
    } else {
        showStaticMessage("播放器初始化失败");
        Serial.println("音频播放器初始化失败");
    }
    delay(2000);

    // 初始化麦克风
    showStaticMessage("初始化麦克风...");
    if (initMicrophone()) {
        showStaticMessage("麦克风初始化成功");
        Serial.println("麦克风初始化成功");
    } else {
        showStaticMessage("麦克风初始化失败");
        Serial.println("麦克风初始化失败");
    }
    delay(2000);

    
    // // 初始化WiFi（包含连通性测试）
    // initWifi();
    // delay(3000);
    
    // // 显示详细网络状态
    // showNetworkStatus();
    // delay(2000);
    
    // // 测试API连接
    // if (testAPIConnection()) {
    //     showStaticMessage("API连接正常");
    // } else {
    //     showStaticMessage("API连接异常");
    // }
    // delay(2000);
    
    // showStaticMessage("初始化摄像头");
    // // 初始化摄像头
    // initCamera();
    // delay(2000);
    
    // showStaticMessage("摄像头初始化完成");
    // Serial.println("System initialization complete!");
}

void loop() {
    // 主菜单循环
    // showStaticMessage("AI视觉系统");
    // delay(2000);
    
    // showStaticMessage("准备就绪");
    // delay(2000);
    
    // 测试录音播放功能
    testRecordingPlayback();
    
    // // 测试麦克风功能
    // testMicrophoneFeatures();
    
    // // 测试图像显示功能
    // testImageDisplayFeatures();
    
    // 长暂停，避免快速循环
    delay(5000);
}

// 新增：图像显示功能综合测试
void testImageDisplayFeatures() {
    Serial.println("=== Image Display Features Test ===");
    
    // 测试1：单张图像显示
    showTwoLineMessage("测试1:", "单张图像显示");
    delay(2000);
    captureAndDisplayOnOled();
    delay(2000);
    
    // 测试2：连续拍照显示
    showTwoLineMessage("测试2:", "连续拍照显示");
    delay(2000);
    continuousCaptureDisplay();
    delay(2000);
    
    // 测试3：再次单张显示（验证稳定性）
    showTwoLineMessage("测试3:", "稳定性测试");
    delay(2000);
    captureAndDisplayOnOled();
    delay(2000);
    
    showTwoLineMessage("所有测试完成", "系统正常");
    delay(3000);
    
    Serial.println("=== Image Display Test Complete ===");
}

// 麦克风功能测试
void testMicrophoneFeatures() {
    Serial.println("=== Microphone Features Test ===");
    
    // 检查麦克风是否就绪
    if (!isMicrophoneReady()) {
        showStaticMessage("麦克风未就绪");
        Serial.println("麦克风未就绪，跳过测试");
        delay(2000);
        return;
    }
    
    // 测试1：音量监测
    showTwoLineMessage("测试1:", "音量监测");
    delay(2000);
    monitorAudioLevel(5000);  // 监测5秒
    delay(2000);
    
    // 测试2：录音测试
    showTwoLineMessage("测试2:", "录音测试");
    delay(2000);
    testMicrophoneRecording(3000);  // 录音3秒
    delay(2000);
    
    // 测试3：再次音量监测（验证稳定性）
    showTwoLineMessage("测试3:", "稳定性测试");
    delay(2000);
    monitorAudioLevel(3000);  // 监测3秒
    delay(2000);
    
    showTwoLineMessage("麦克风测试完成", "系统正常");
    delay(3000);
    
    Serial.println("=== Microphone Test Complete ===");
}

// 录音播放功能测试
void testRecordingPlayback() {
    Serial.println("=== Recording Playback Test ===");
    
    // 检查麦克风和播放器是否就绪
    if (!isMicrophoneReady()) {
        showStaticMessage("麦克风未就绪");
        Serial.println("麦克风未就绪，跳过测试");
        delay(2000);
        return;
    }
    
    if (!isAudioPlayerReady()) {
        showStaticMessage("播放器未就绪");
        Serial.println("播放器未就绪，跳过测试");
        delay(2000);
        return;
    }
    
    // 测试0：首先测试播放器功能
    showTwoLineMessage("测试0:", "播放器音调测试");
    delay(2000);
    playSineTone(440.0, 1000); // Play a 440 Hz sine tone for 1 second
    delay(500);
    playSineTone(880.0, 500);  // Play a 880 Hz sine tone for 0.5 second
    delay(2000);
    
    // 测试1：录音
    showTwoLineMessage("测试1:", "开始录音");
    delay(2000);
    recordAudio(3000);  // 录音3秒
    delay(2000);
    
    // 测试2：播放录音
    if (hasRecordingData()) {
        showTwoLineMessage("测试2:", "播放录音");
        delay(2000);
        playRecording();
        delay(2000);
        
        // 测试3：增强音量播放
        showTwoLineMessage("测试3:", "增强音量播放");
        delay(2000);
        playRecordingWithGain(3.0);  // 增强音量3倍
        delay(2000);
    } else {
        showTwoLineMessage("警告:", "没有录音数据");
        delay(2000);
    }
    
    // 测试4：清除录音并再次录音
    showTwoLineMessage("测试4:", "清除并重新录音");
    delay(2000);
    clearRecording();
    delay(1000);
    
    recordAudio(2000);  // 再次录音2秒
    delay(2000);
    
    // 播放新录音
    if (hasRecordingData()) {
        showTwoLineMessage("播放新录音", "");
        delay(2000);
        playRecording();
        delay(2000);
    }
    
    showTwoLineMessage("录音播放测试完成", "系统正常");
    delay(3000);
    
    Serial.println("=== Recording Playback Test Complete ===");
}

void resolveImage(String image) {
    Serial.println("Resolving image...");
    Serial.println(image);
}

void testImageResolution() {
    Serial.println("=== AI Vision Demo ===");
  // 测试拍照并分析（需要API密钥）
    showStaticMessage("图像分析测试");
    String analysisResult = captureAndAnalyze("图中描绘的是什么景象？请用中文简短回答。");
    
    if (analysisResult.length() > 0 && !analysisResult.startsWith("错误")) {
        Serial.println("分析结果: " + analysisResult);
        // 结果会在showStaticMessage中显示
    } else {
        Serial.println("分析失败: " + analysisResult);
        showStaticMessage("AI分析失败");
    }
    
    delay(5000);
    
    // 测试使用网络图片分析（演示用）
    showStaticMessage("网络图片分析");
    String urlResult = resolveImageFromURL(
        "https://help-static-aliyun-doc.aliyuncs.com/file-manage-files/zh-CN/20241022/emyrja/dog_and_girl.jpeg", 
        "图中描绘的是什么景象？请用中文简短回答。"
    );
    
    if (urlResult.length() > 0 && !urlResult.startsWith("错误")) {
        Serial.println("网络图片分析结果: " + urlResult);
    } else {
        Serial.println("网络图片分析失败: " + urlResult);
        showStaticMessage("网络分析失败");
    }
    
    Serial.println("=== Demo Cycle Complete ===");
    delay(10000);
  }