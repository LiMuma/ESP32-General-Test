#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// 定义OLED显示屏参数
#define SCREEN_WIDTH 128    // OLED显示宽度，单位：像素
#define SCREEN_HEIGHT 32    // OLED显示高度，单位：像素
#define OLED_RESET -1       // 重置引脚（如果OLED没有重置引脚，设为-1）
#define SCREEN_ADDRESS 0x3C // I2C地址，通常是0x3C或0x3D

// 定义I2C引脚
#define SDA_PIN 19
#define SCL_PIN 20

// 创建U8g2显示对象 - 使用SSD1306 128x32 I2C显示屏（全局访问）
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL_PIN, /* data=*/ SDA_PIN);

void initScreen() {
  Serial.println("Initializing OLED Display...");
  
  // 初始化U8g2显示屏
  u8g2.begin();
  u8g2.enableUTF8Print();		// enable UTF8 support for the Arduino print() function
  
  Serial.println("OLED Display Initialized Successfully!");
  
  // 先测试基本的ASCII字符
  u8g2.setFont(u8g2_font_unifont_t_gb2312); // 标准ASCII字体
  u8g2.clearBuffer();
  u8g2.setCursor(5, 15);
  u8g2.print("云谷一号");
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
  
  u8g2.setCursor(2, 15);
  u8g2.print(message);
  u8g2.sendBuffer();
}

void showTwoLineMessage(const char* line1, const char* line2) {  
  u8g2.clearBuffer();
  u8g2.setCursor(2, 14);
  u8g2.print(line1);
  u8g2.setCursor(2, 30);
  u8g2.print(line2);
  u8g2.sendBuffer();
  }

void showScrollMessage(const char* message) {  
  int textWidth = u8g2.getUTF8Width(message);
  int screenWidth = u8g2.getDisplayWidth();
  
  Serial.printf("Text width: %d, Screen width: %d\n", textWidth, screenWidth);
  
  if (textWidth > screenWidth) {
    Serial.println("Using scroll animation");
    for (int x = screenWidth; x > -textWidth; x -= 2) {
      u8g2.clearBuffer();
      u8g2.setCursor(x, 15);
      u8g2.print(message);
      u8g2.sendBuffer();
      delay(50);
    }
  } else {
    Serial.println("Using centered display");
    int x = (screenWidth - textWidth) / 2;
    u8g2.clearBuffer();
    u8g2.setCursor(x, 15);
    u8g2.print(message);
    u8g2.sendBuffer();
    
    // 添加停留时间，确保消息可见
    delay(1000);
  }
  Serial.println("showScrollMessage completed");
}
