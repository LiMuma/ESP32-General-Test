#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "H3C_D4EBAB"
#define WIFI_PASSWORD "abcd1234"

void showWifiStatus(const char* status);
void showSafeChineseMessage(int messageId);
void showStaticMessage(const char* message);

bool testBaiduConnectivity() {
  
  HTTPClient http;

  // 测试连接百度（中国大陆常用）
  Serial.println("Testing connectivity to Baidu...");
  http.begin("http://www.baidu.com");
  http.setTimeout(5000); // 5秒超时
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.printf("Baidu HTTP response: %d\n", httpCode);
    if (httpCode == 200 || httpCode == 302) {
      showStaticMessage("百度连接成功");
      return true;
    }
  } else {
    Serial.printf("Baidu connection failed: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return false;
}

bool testGoogleDNSConnectivity() {
  Serial.println("Testing connectivity to Google DNS...");
  HTTPClient http;
    http.begin("http://8.8.8.8");
    http.setTimeout(5000);

    int httpCode2 = http.GET();
    if (httpCode2 > 0) {
        Serial.printf("Google DNS HTTP response: %d\n", httpCode2);
        if (httpCode2 >= 200 && httpCode2 < 400) {
        showStaticMessage("谷歌DNS连接成功");
        return true;
        }
    } else {
        Serial.printf("Google DNS connection failed: %s\n", http.errorToString(httpCode2).c_str());
    }
    http.end();
    return false;
}

bool testHttpbinConnectivity() {
    HTTPClient http;
    Serial.println("Testing connectivity to httpbin...");
    http.begin("http://httpbin.org/get");
    http.setTimeout(5000);
    
    int httpCode3 = http.GET();
    if (httpCode3 > 0) {
      Serial.printf("Httpbin HTTP response: %d\n", httpCode3);
      if (httpCode3 == 200) {
        showStaticMessage("Httpbin连接成功");
        return true;
      }
    } else {
      Serial.printf("Httpbin connection failed: %s\n", http.errorToString(httpCode3).c_str());
    }
    http.end();
    return false;
}

// 测试公网连通性
bool testInternetConnectivity() {
  showStaticMessage("测试公网连通性");
  
  HTTPClient http;
  bool isConnected = testBaiduConnectivity();
  
  // 如果百度失败，尝试连接谷歌DNS（备用测试）
  if (!isConnected) {
    isConnected = testGoogleDNSConnectivity();
  }
  
  // 最后尝试httpbin（国际测试服务）
  if (!isConnected) {
    isConnected = testHttpbinConnectivity();
  }
  
  if (isConnected) {
    Serial.println("Internet connectivity: OK");
    showStaticMessage("公网连接正常");
    return true;
  } else {
    Serial.println("Internet connectivity: FAILED");
    showStaticMessage("公网连接失败");
    return false;
  }
}

// 测试阿里云API连通性
bool testDashScopeConnectivity() {
  showStaticMessage("测试阿里云连通性");
  
  HTTPClient http;
  http.begin("https://dashscope.aliyuncs.com");
  http.setTimeout(10000); // 10秒超时，HTTPS可能较慢
  
  // 设置用户代理
  http.addHeader("User-Agent", "ESP32-Client");
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.printf("DashScope connectivity test: %d\n", httpCode);
    // 404是正常的，说明域名可达但根路径不存在
    if (httpCode == 200 || httpCode == 404 || httpCode == 403) {
      showStaticMessage("阿里云连接成功");
      Serial.println("DashScope domain is reachable (200/404/403 are all valid)");
      http.end();
      return true;
    } else if (httpCode >= 400 && httpCode < 500) {
      showStaticMessage("阿里云域名可达");
      Serial.printf("DashScope domain reachable but got client error %d\n", httpCode);
      http.end();
      return true;
    }
  } else {
    Serial.printf("DashScope connection failed: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  showStaticMessage("阿里云连接失败");
  return false;
}

void initWifi() {
  Serial.begin(115200);
  Serial.println("WiFi Initializing...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // 使用UTF-8中文显示WiFi连接状态
  showStaticMessage("WiFi连接中...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
    
         // 每5秒更新一次显示
     if (attempts % 5 == 0) {
       String statusMsg = "连接中..." + String(attempts) + "s";
       showStaticMessage(statusMsg.c_str());
     }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // 使用UTF-8中文显示连接成功
    showStaticMessage("WiFi连接成功!");
    delay(2000);
    
    // 显示IP地址
    String ipMessage = "IP:" + WiFi.localIP().toString();
    showStaticMessage(ipMessage.c_str());
    delay(2000);
    
    // 测试公网连通性
    bool internetOK = testInternetConnectivity();
    delay(2000);
    
    // 测试阿里云API连通性
    if (internetOK) {
      bool apiOK = testDashScopeConnectivity();
      delay(2000);
      
      if (apiOK) {
        showStaticMessage("网络测试通过");
      } else {
        showStaticMessage("API连接异常");
      }
    } else {
      showStaticMessage("网络连接异常");
    }
    
  } else {
    Serial.println();
    Serial.println("WiFi Connection Failed!");
    showStaticMessage("WiFi连接失败");
  }
}

void showNetworkStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("=== 网络状态详情 ===");
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("网关: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("信号强度: %d dBm\n", WiFi.RSSI());
    
    // 在屏幕上显示关键信息
    String statusMsg = "RSSI:" + String(WiFi.RSSI()) + "dBm";
    showStaticMessage(statusMsg.c_str());
  } else {
    Serial.println("WiFi未连接");
    showStaticMessage("WiFi未连接");
  }
}





