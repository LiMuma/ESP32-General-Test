#include <Arduino.h>
#include <U8g2lib.h>

// 外部引用显示对象
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

// UTF-8中文测试文本
const char* utf8_test_texts[] = {
  "你好世界",
  "欢迎使用",
  "系统启动",
  "连接网络",
  "运行正常",
  "测试完成"
};

// 测试不同的UTF-8中文字体
void testAllUTF8Fonts() {
  Serial.println("Testing all UTF-8 Chinese fonts...");
  
  const uint8_t* fonts[] = {
    u8g2_font_wqy12_t_chinese1,
    u8g2_font_wqy12_t_chinese2,
    u8g2_font_wqy12_t_chinese3
  };
  
  const char* fontNames[] = {
    "Chinese1",
    "Chinese2", 
    "Chinese3"
  };
  
  for (int fontIndex = 0; fontIndex < 3; fontIndex++) {
    Serial.print("Testing font: ");
    Serial.println(fontNames[fontIndex]);
    
    // 显示字体名称
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(2, 10);
    u8g2.print("Font: ");
    u8g2.print(fontNames[fontIndex]);
    u8g2.sendBuffer();
    delay(1500);
    
    // 测试当前字体显示中文
    for (int textIndex = 0; textIndex < 6; textIndex++) {
      u8g2.clearBuffer();
      u8g2.setFont(fonts[fontIndex]);
      
      // 计算文本居中位置
      int textWidth = u8g2.getStrWidth(utf8_test_texts[textIndex]);
      int x = (128 - textWidth) / 2;
      if (x < 0) x = 2;
      
      u8g2.setCursor(x, 20);
      u8g2.print(utf8_test_texts[textIndex]);
      u8g2.sendBuffer();
      delay(1000);
    }
  }
}

// 测试UTF-8编码的常用中文字符
void testCommonChineseChars() {
  Serial.println("Testing common Chinese characters...");
  
  const char* commonChars[] = {
    "一二三四五",
    "上下左右中", 
    "开关启停",
    "连接断开",
    "成功失败",
    "正常错误"
  };
  
  u8g2.setFont(u8g2_font_wqy12_t_chinese1);
  
  for (int i = 0; i < 6; i++) {
    u8g2.clearBuffer();
    u8g2.setCursor(2, 15);
    u8g2.print(commonChars[i]);
    u8g2.sendBuffer();
    delay(1500);
  }
}

// 显示UTF-8状态信息
void showUTF8Status(const char* title, const char* message) {
  u8g2.clearBuffer();
  
  // 标题用英文，确保显示
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(2, 10);
  u8g2.print(title);
  
  // 消息用UTF-8中文
  u8g2.setFont(u8g2_font_wqy12_t_chinese1);
  u8g2.setCursor(2, 28);
  u8g2.print(message);
  
  u8g2.sendBuffer();
} 