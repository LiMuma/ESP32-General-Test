#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===========================================
// 重要提醒：使用前请设置您的API密钥！
// ===========================================
// 1. 注册阿里云账号并开通DashScope服务
// 2. 获取API密钥
// 3. 将下面的"YOUR_API_KEY_HERE"替换为您的实际API密钥
// 4. 确保账户有足够的余额或免费额度

// 阿里云API配置
const char* DASHSCOPE_API_URL = "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
const char* DASHSCOPE_API_KEY = "sk-deef87410c114822b24f54cc0ffeb406"; // ⚠️ 请替换为您的实际API密钥

// 外部函数声明
void showStaticMessage(const char* message);


String resolveImage(const String& imageBase64, const String& question) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi未连接");
    showStaticMessage("WiFi未连接");
    return "错误：WiFi未连接";
  }

  // 检查API密钥
  if (String(DASHSCOPE_API_KEY) == "YOUR_API_KEY_HERE") {
    Serial.println("请设置API密钥");
    showStaticMessage("API密钥未设置");
    return "错误：API密钥未设置";
  }

  // 检查图像大小，如果太大则提示
  Serial.printf("图像Base64大小: %d 字符\n", imageBase64.length());
  if (imageBase64.length() > 100000) { // 约75KB原始图像
    Serial.println("警告：图像可能过大");
    showStaticMessage("图像较大");
  }

  HTTPClient http;
  http.begin(DASHSCOPE_API_URL);
  
  // 设置请求头
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(DASHSCOPE_API_KEY));
  http.addHeader("User-Agent", "ESP32-Client/1.0");

  // 构建JSON请求体 - 使用更简洁的格式
  DynamicJsonDocument doc(16384); // 增加缓冲区大小
  doc["model"] = "qwen-vl-max";
  
  JsonArray messages = doc.createNestedArray("messages");
  
  // 用户消息
  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";
  JsonArray userContent = userMsg.createNestedArray("content");
  
  // 图像内容
  JsonObject imageContent = userContent.createNestedObject();
  imageContent["type"] = "image_url";
  JsonObject imageUrl = imageContent.createNestedObject("image_url");
  imageUrl["url"] = "data:image/jpeg;base64," + imageBase64;
  
  // 文本内容
  JsonObject textContent = userContent.createNestedObject();
  textContent["type"] = "text";
  textContent["text"] = question;

  // 序列化JSON
  String jsonString;
  serializeJson(doc, jsonString);
  
  // 打印请求信息用于调试
  Serial.printf("请求JSON大小: %d 字节\n", jsonString.length());
  Serial.println("请求头信息:");
  Serial.println("Content-Type: application/json");
  Serial.println("Authorization: Bearer " + String(DASHSCOPE_API_KEY).substring(0, 10) + "...");
  
  Serial.println("发送API请求...");
  showStaticMessage("分析图像中...");
  
  // 发送POST请求
  int httpResponseCode = http.POST(jsonString);
  String response = "";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    Serial.println("API响应码: " + String(httpResponseCode));
    
    // 打印部分响应用于调试
    if (response.length() > 100) {
      Serial.println("响应前100字符: " + response.substring(0, 100));
    } else {
      Serial.println("完整响应: " + response);
    }
    
    if (httpResponseCode == 200) {
      // 解析响应
      DynamicJsonDocument responseDoc(8192);
      DeserializationError error = deserializeJson(responseDoc, response);
      
      if (error) {
        Serial.println("JSON解析失败: " + String(error.c_str()));
        showStaticMessage("响应解析失败");
        return "错误：响应解析失败";
      }
      
      if (responseDoc.containsKey("choices") && responseDoc["choices"].size() > 0) {
        String result = responseDoc["choices"][0]["message"]["content"];
        Serial.println("AI分析结果: " + result);
        showStaticMessage(result.c_str());
        return result;
      } else {
        Serial.println("响应格式错误，缺少choices字段");
        showStaticMessage("响应格式错误");
        return "错误：响应格式错误";
      }
    } else if (httpResponseCode == 400) {
      Serial.println("400错误 - 请求格式问题");
      Serial.println("可能原因：1.API密钥错误 2.图像格式问题 3.请求参数错误");
      showStaticMessage("请求格式错误");
      return "错误：请求格式错误(400)";
    } else if (httpResponseCode == 401) {
      Serial.println("401错误 - 认证失败，请检查API密钥");
      showStaticMessage("API密钥错误");
      return "错误：API密钥错误(401)";
    } else if (httpResponseCode == 429) {
      Serial.println("429错误 - 请求过于频繁");
      showStaticMessage("请求过于频繁");
      return "错误：请求过于频繁(429)";
    } else {
      Serial.println("API请求失败: " + String(httpResponseCode));
      showStaticMessage("API请求失败");
      return "错误：API请求失败(" + String(httpResponseCode) + ")";
    }
  } else {
    Serial.println("HTTP请求失败: " + http.errorToString(httpResponseCode));
    showStaticMessage("网络请求失败");
    return "错误：网络请求失败";
  }
  
  http.end();
  return response;
}

// 使用图像URL的版本
String resolveImageFromURL(const String& imageUrl, const String& question) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi未连接");
    return "错误：WiFi未连接";
  }

  HTTPClient http;
  http.begin(DASHSCOPE_API_URL);
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(DASHSCOPE_API_KEY));

  // 构建JSON请求体（使用URL）
  DynamicJsonDocument doc(4096);
  doc["model"] = "qwen-vl-max";
  
  JsonArray messages = doc.createNestedArray("messages");
  
  // 系统消息
  JsonObject systemMsg = messages.createNestedObject();
  systemMsg["role"] = "system";
  JsonArray systemContent = systemMsg.createNestedArray("content");
  JsonObject systemText = systemContent.createNestedObject();
  systemText["type"] = "text";
  systemText["text"] = "You are a helpful assistant that analyzes images in Chinese.";
  
  // 用户消息
  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";
  JsonArray userContent = userMsg.createNestedArray("content");
  
  // 图像URL
  JsonObject imageContent = userContent.createNestedObject();
  imageContent["type"] = "image_url";
  JsonObject imageUrlObj = imageContent.createNestedObject("image_url");
  imageUrlObj["url"] = imageUrl;
  
  // 文本问题
  JsonObject textContent = userContent.createNestedObject();
  textContent["type"] = "text";
  textContent["text"] = question;

  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.println("发送API请求...");
  showStaticMessage("分析图像中...");
  
  int httpResponseCode = http.POST(jsonString);
  String response = "";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    
    if (httpResponseCode == 200) {
      DynamicJsonDocument responseDoc(4096);
      deserializeJson(responseDoc, response);
      
      if (responseDoc.containsKey("choices") && responseDoc["choices"].size() > 0) {
        String result = responseDoc["choices"][0]["message"]["content"];
        Serial.println("AI分析结果: " + result);
        showStaticMessage(result.c_str());
        return result;
      }
    }
  }
  
  http.end();
  return "分析失败";
}

// 测试API连接（不使用图像，只测试文本）
bool testAPIConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi未连接");
    showStaticMessage("WiFi未连接");
    return false;
  }

  if (String(DASHSCOPE_API_KEY) == "YOUR_API_KEY_HERE") {
    Serial.println("请设置API密钥");
    showStaticMessage("API密钥未设置");
    return false;
  }

  HTTPClient http;
  http.begin(DASHSCOPE_API_URL);
  
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(DASHSCOPE_API_KEY));
  http.addHeader("User-Agent", "ESP32-Client/1.0");

  // 构建简单的文本测试请求
  DynamicJsonDocument doc(2048);
  doc["model"] = "qwen-max";  // 使用纯文本模型
  
  JsonArray messages = doc.createNestedArray("messages");
  
  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";
  userMsg["content"] = "你好，请回复：API测试成功";

  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.println("测试API连接...");
  showStaticMessage("测试API连接");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("API测试响应码: %d\n", httpResponseCode);
    
    if (httpResponseCode == 200) {
      Serial.println("API连接测试成功");
      showStaticMessage("连接测试成功");
      http.end();
      return true;
    } else {
      Serial.printf("API测试失败: %d\n", httpResponseCode);
      Serial.println("响应: " + response);
      showStaticMessage("API测试失败");
    }
  } else {
    Serial.println("HTTP请求失败");
    showStaticMessage("网络请求失败");
  }
  
  http.end();
  return false;
}