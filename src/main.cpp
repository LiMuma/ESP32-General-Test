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

// AI视觉分析功能
String resolveImage(const String& imageBase64, const String& question);
String resolveImageFromURL(const String& imageUrl, const String& question);
bool testAPIConnection();

// 网络功能
void initWifi();
bool testInternetConnectivity();
bool testDashScopeConnectivity();
void showNetworkStatus();

void testImageResolution();
void testImageDisplayFeatures(); // 新增：图像显示功能测试

void setup() {
    Serial.begin(115200);
    Serial.println("=== ESP32 AI Vision System ===");
    
    // 初始化屏幕
    initScreen();
    delay(2000);
    
    // 初始化WiFi（包含连通性测试）
    initWifi();
    delay(3000);
    
    // 显示详细网络状态
    showNetworkStatus();
    delay(2000);
    
    // 测试API连接
    if (testAPIConnection()) {
        showStaticMessage("API连接正常");
    } else {
        showStaticMessage("API连接异常");
    }
    delay(2000);
    
    showStaticMessage("初始化摄像头");
    // 初始化摄像头
    initCamera();
    delay(2000);
    
    showStaticMessage("摄像头初始化完成");
    Serial.println("System initialization complete!");
}

void loop() {
    // 主菜单循环
    showStaticMessage("AI视觉系统");
    delay(2000);
    
    showStaticMessage("准备就绪");
    delay(2000);
    
    // 测试图像显示功能
    testImageDisplayFeatures();
    
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