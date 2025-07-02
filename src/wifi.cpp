#include <Arduino.h>
#include <WiFi.h>

#define WIFI_SSID "Yungu HS CS Lab"
#define WIFI_PASSWORD "mujianglaoshi"

void showWifiStatus(const char* status);
void showSafeChineseMessage(int messageId);
void showUTF8ChineseMessage(const char* message);

void initWifi() {
  Serial.begin(115200);
  Serial.println("WiFi Initializing...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // 使用UTF-8中文显示WiFi连接状态
  showUTF8ChineseMessage("WiFi连接中...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    // 显示中文连接状态
    showSafeChineseMessage(2); // "WiFi连接中"
  }
  
  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // 使用UTF-8中文显示连接成功
  showUTF8ChineseMessage("WiFi连接成功!");
  delay(2000);
  
  // 也可以显示IP地址
  String ipMessage = "IP:" + WiFi.localIP().toString();
  showWifiStatus(ipMessage.c_str());
  delay(2000);
}





