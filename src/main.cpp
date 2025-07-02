#include <Arduino.h>
#include <WiFi.h>

// 基本显示函数
void initScreen();
void showUTF8ChineseMessage(const char* message);
void showUTF8Status(const char* title, const char* message);

// 摄像头功能
void initCamera();
void captureImage();
String captureAndAnalyze(const String& question);

// AI视觉分析功能
String resolveImage(const String& imageBase64, const String& question);
String resolveImageFromURL(const String& imageUrl, const String& question);
bool testAPIConnection();

// 网络功能
void initWifi();
bool testInternetConnectivity();
bool testDashScopeConnectivity();
void showNetworkStatus();

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
        showUTF8Status("Ready:", "API连接正常");
    } else {
        showUTF8Status("Warning:", "API连接异常");
    }
    delay(2000);
    
    // 初始化摄像头
    initCamera();
    delay(2000);
    
    showUTF8Status("System:", "初始化完成");
    Serial.println("System initialization complete!");
}

void loop() {
    Serial.println("=== AI Vision Demo ===");
    
    // 检查网络状态
    if (WiFi.status() != WL_CONNECTED) {
        showUTF8Status("Error:", "WiFi未连接");
        delay(5000);
        return;
    }
    
    // 先测试API连接
    if (!testAPIConnection()) {
        showUTF8Status("Error:", "API连接失败");
        delay(10000);
        return;
    }
    
    // 测试拍照并分析（需要API密钥）
    showUTF8Status("AI:", "图像分析测试");
    String analysisResult = captureAndAnalyze("图中描绘的是什么景象？请用中文简短回答。");
    
    if (analysisResult.length() > 0 && !analysisResult.startsWith("错误")) {
        Serial.println("分析结果: " + analysisResult);
        // 结果会在showUTF8ChineseMessage中显示
    } else {
        Serial.println("分析失败: " + analysisResult);
        showUTF8Status("Error:", "AI分析失败");
    }
    
    delay(5000);
    
    // 测试使用网络图片分析（演示用）
    showUTF8Status("AI:", "网络图片分析");
    String urlResult = resolveImageFromURL(
        "https://help-static-aliyun-doc.aliyuncs.com/file-manage-files/zh-CN/20241022/emyrja/dog_and_girl.jpeg", 
        "图中描绘的是什么景象？请用中文简短回答。"
    );
    
    if (urlResult.length() > 0 && !urlResult.startsWith("错误")) {
        Serial.println("网络图片分析结果: " + urlResult);
    } else {
        Serial.println("网络图片分析失败: " + urlResult);
        showUTF8Status("Error:", "网络分析失败");
    }
    
    delay(10000);
    
    Serial.println("=== Demo Cycle Complete ===");
    delay(3000);
}

void resolveImage(String image) {
    Serial.println("Resolving image...");
    Serial.println(image);
}