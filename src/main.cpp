#include <Arduino.h>

// 基本显示函数
void initScreen();
void showStaticMessage(const char* message);
void showTwoLineMessage(const char* line1, const char* line2);
void showScrollMessage(const char* message);
void showChineseMessage(const char* pinyin, const char* english);
void showWifiStatus(const char* status);
void showUTF8ChineseMessage(const char* message);

// 中文支持函数（UTF-8）
void showRealChineseMessage(const char* message);
void showChineseStatus(const char* title, const char* status);
void showSafeChineseMessage(int messageId);
void testUTF8Fonts();

// UTF-8测试函数
void testAllUTF8Fonts();
void testCommonChineseChars();
void showUTF8Status(const char* title, const char* message);

// 其他模块
void initAudio();
void playAudio();
void initWifi();

void setup() {
  initScreen();
  initWifi();
//   initAudio();
}

void loop() {
  Serial.println("=== UTF-8 Chinese Display Test ===");
  
  // 基本UTF-8中文显示测试
  showUTF8ChineseMessage("UTF8测试");
  delay(2000);
  
  // 显示状态信息
  showUTF8Status("Status:", "系统正常");
  delay(2000);
  
  // 测试常用中文字符
  testCommonChineseChars();
  
  // 测试所有UTF-8字体（这个会比较长）
  // testAllUTF8Fonts();
  
  // 传统测试
  showStaticMessage("UTF-8 Test Complete");
  delay(2000);
  
  showChineseMessage("Ce Shi Wan Cheng", "Test Complete");
  delay(2000);
  
  Serial.println("=== Test Cycle Complete ===");
  delay(3000);
}