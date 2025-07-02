#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// 定义OLED显示屏参数
#define SCREEN_WIDTH 128    // OLED显示宽度，单位：像素
#define SCREEN_HEIGHT 32    // OLED显示高度，单位：像素
#define OLED_RESET -1       // 重置引脚（如果OLED没有重置引脚，设为-1）
#define SCREEN_ADDRESS 0x3C // I2C地址，通常是0x3C或0x3D

// 定义I2C引脚
#define SDA_PIN 5
#define SCL_PIN 4

// 创建U8g2显示对象 - 使用SSD1306 128x32 I2C显示屏（全局访问）
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL_PIN, /* data=*/ SDA_PIN);

void initScreen() {
  Serial.begin(115200);
  Serial.println("Initializing OLED Display...");
  
  // 初始化U8g2显示屏
  u8g2.begin();
  
  Serial.println("OLED Display Initialized Successfully!");
  
  // 先测试基本的ASCII字符
  u8g2.setFont(u8g2_font_ncenB08_tr); // 标准ASCII字体
  u8g2.clearBuffer();
  u8g2.setCursor(5, 15);
  u8g2.print("Hello ESP32!");
  u8g2.setCursor(5, 28);
  u8g2.print("Screen Ready");
  u8g2.sendBuffer();
}

void showStaticMessage(const char* message) {
  u8g2.clearBuffer();
  
  // 先尝试检测是否包含中文字符
  bool hasChinese = false;
  const char* p = message;
  while (*p) {
    if ((*p & 0x80) != 0) { // 检测非ASCII字符
      hasChinese = true;
      break;
    }
    p++;
  }
  
  if (hasChinese) {
    // 使用UTF-8编码的中文字体
    u8g2.setFont(u8g2_font_wqy12_t_chinese1); // UTF-8中文字体
  } else {
    u8g2.setFont(u8g2_font_ncenB08_tr); // ASCII字体
  }
  
  u8g2.setCursor(2, 15);
  u8g2.print(message);
  u8g2.sendBuffer();
}

void showTwoLineMessage(const char* line1, const char* line2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); // 使用ASCII字体更稳定
  u8g2.setCursor(2, 12);
  u8g2.print(line1);
  u8g2.setCursor(2, 28);
  u8g2.print(line2);
  u8g2.sendBuffer();
}

void showScrollMessage(const char* message) {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int textWidth = u8g2.getStrWidth(message);
  int screenWidth = u8g2.getDisplayWidth();
  
  if (textWidth > screenWidth) {
    for (int x = screenWidth; x > -textWidth; x -= 2) {
      u8g2.clearBuffer();
      u8g2.setCursor(x, 15);
      u8g2.print(message);
      u8g2.sendBuffer();
      delay(50);
    }
  } else {
    int x = (screenWidth - textWidth) / 2;
    u8g2.clearBuffer();
    u8g2.setCursor(x, 15);
    u8g2.print(message);
    u8g2.sendBuffer();
  }
}

// 显示中文的专用函数（使用拼音）
void showChineseMessage(const char* pinyin, const char* english) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(2, 12);
  u8g2.print(pinyin);    // 显示拼音
  u8g2.setCursor(2, 28);
  u8g2.print(english);   // 显示英文
  u8g2.sendBuffer();
}

// 显示WiFi状态
void showWifiStatus(const char* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(2, 12);
  u8g2.print("WiFi:");
  u8g2.setCursor(2, 28);
  u8g2.print(status);
  u8g2.sendBuffer();
}

// 新增：UTF-8中文显示函数
void showUTF8ChineseMessage(const char* message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_chinese1); // UTF-8中文字体
  u8g2.setCursor(2, 15);
  u8g2.print(message);
  u8g2.sendBuffer();
}

// void loop() {
//   Serial.println("Hello, World! (显示在屏幕上)");
//   delay(1000);
// }