#include <Arduino.h>
#include <U8g2lib.h>

// 外部引用显示对象
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

// 真正的中文显示函数（使用UTF-8字体）
void showRealChineseMessage(const char* message) {
  u8g2.clearBuffer();
  
  // 使用支持UTF-8编码的中文字体
  u8g2.setFont(u8g2_font_wqy12_t_chinese1);  // UTF-8中文字体
  
  u8g2.setCursor(2, 15);
  u8g2.print(message);
  u8g2.sendBuffer();
}

// 显示中文状态信息（使用UTF-8字体）
void showChineseStatus(const char* title, const char* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_chinese2);  // UTF-8中文字体
  
  u8g2.setCursor(2, 12);
  u8g2.print(title);
  u8g2.setCursor(2, 28);
  u8g2.print(status);
  u8g2.sendBuffer();
}

// UTF-8编码的中文字符串常量
const char* CHINESE_HELLO = "你好世界";
const char* CHINESE_WIFI_CONNECTING = "WiFi连接中";
const char* CHINESE_WIFI_CONNECTED = "WiFi已连接";
const char* CHINESE_READY = "系统准备就绪";
const char* CHINESE_ERROR = "系统错误";

// 安全的中文显示函数（使用UTF-8字体）
void showSafeChineseMessage(int messageId) {
  const char* message = "";
  
  switch(messageId) {
    case 1: message = CHINESE_HELLO; break;
    case 2: message = CHINESE_WIFI_CONNECTING; break;
    case 3: message = CHINESE_WIFI_CONNECTED; break;
    case 4: message = CHINESE_READY; break;
    case 5: message = CHINESE_ERROR; break;
    default: message = "Unknown"; break;
  }
  
  showRealChineseMessage(message);
}

// 新增：使用不同UTF-8字体的测试函数
void testUTF8Fonts() {
  const char* testText = "测试UTF8";
  
  // 测试字体1
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_chinese1);
  u8g2.setCursor(2, 10);
  u8g2.print("Font1:");
  u8g2.setCursor(2, 22);
  u8g2.print(testText);
  u8g2.sendBuffer();
  delay(2000);
  
  // 测试字体2
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_chinese2);
  u8g2.setCursor(2, 10);
  u8g2.print("Font2:");
  u8g2.setCursor(2, 22);
  u8g2.print(testText);
  u8g2.sendBuffer();
  delay(2000);
  
  // 测试字体3
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_chinese3);
  u8g2.setCursor(2, 10);
  u8g2.print("Font3:");
  u8g2.setCursor(2, 22);
  u8g2.print(testText);
  u8g2.sendBuffer();
  delay(2000);
} 