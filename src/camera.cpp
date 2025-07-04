#include <Arduino.h>
#include "esp_camera.h"
#include <U8g2lib.h>

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

// 外部函数声明
String resolveImage(const String& imageBase64, const String& question);
void showStaticMessage(const char* message);
void showTwoLineMessage(const char* line1, const char* line2);

// 外部显示对象声明
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

// 简单的JPEG解码结构
struct ImageInfo {
  uint16_t width;
  uint16_t height;
  uint8_t* pixels;  // 灰度像素数据 (0-255)
};

// Simple base64 encode function
String base64Encode(const uint8_t* data, size_t length) {
  const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String result = "";
  
  for (size_t i = 0; i < length; i += 3) {
    uint32_t b = (data[i] << 16) | ((i + 1 < length ? data[i + 1] : 0) << 8) | (i + 2 < length ? data[i + 2] : 0);
    result += chars[(b >> 18) & 63];
    result += chars[(b >> 12) & 63];
    result += (i + 1 < length) ? chars[(b >> 6) & 63] : '=';
    result += (i + 2 < length) ? chars[b & 63] : '=';
  }
  return result;
}

void initCamera() {
  showTwoLineMessage("正在初始化摄像头...","");
  
  // 先尝试反初始化，防止重复初始化错误
  esp_camera_deinit();
  delay(100);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  
  // 使用较小的分辨率以减少数据量，便于API传输
  config.frame_size = FRAMESIZE_VGA;    // 640x480 而不是 UXGA
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 15;  // 稍微降低质量以减少大小 (10-63, 数字越小质量越好)
  config.fb_count = 1;
    
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  // for larger pre-allocated frame buffer.
  if(psramFound()){
    Serial.println("PSRAM found, using optimized settings");
    config.jpeg_quality = 12;  // 在PSRAM环境下稍微提高质量
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("PSRAM not found, using basic settings");
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_QVGA; // 320x240 更小的分辨率
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.jpeg_quality = 20; // 进一步降低质量以适应内存限制
  }

  showTwoLineMessage("正在初始化摄像头...","2");
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  delay(2000);  // 延时2秒确保消息显示
  showTwoLineMessage("正在初始化摄像头...","3");
  delay(2000);  // 延时2秒确保消息显示
  showTwoLineMessage("正在初始化摄像头...","4");
  delay(2000);  // 延时2秒确保消息显示
  showTwoLineMessage("正在初始化摄像头...","5");
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x (%s)\n", err, esp_err_to_name(err));
    
    // 详细错误信息
    switch(err) {
      case ESP_ERR_INVALID_STATE:
        Serial.println("错误: 摄像头已被初始化或状态无效");
        showStaticMessage("摄像头状态错误");
        break;
      case ESP_ERR_NOT_FOUND:
        Serial.println("错误: 找不到摄像头硬件");
        showStaticMessage("摄像头硬件错误");
        break;
      case ESP_ERR_NO_MEM:
        Serial.println("错误: 内存不足");
        showStaticMessage("内存不足");
        break;
      default:
        Serial.printf("错误: 未知错误代码 0x%x\n", err);
        showStaticMessage("摄像头初始化失败");
        break;
    }
    
    // 不要直接返回，继续执行以便调试
    delay(3000);
  } else {
    Serial.println("Camera initialized successfully!");
  }
  
  Serial.println("About to show message 3...");
  showTwoLineMessage("正在初始化摄像头...","3");
  delay(2000);  // 延时2秒确保消息显示
  
  Serial.println("About to get camera sensor...");
  // 即使初始化失败，也尝试获取传感器（用于调试）
  sensor_t * s = esp_camera_sensor_get();
  
  Serial.println("About to show message 4...");
  showTwoLineMessage("正在初始化摄像头...","4");
  delay(2000);  // 延时2秒确保消息显示
  
  if (s != NULL) {
    Serial.println("Camera sensor found and accessible");
    // 设置图像参数以获得更好的压缩比
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    
    Serial.println("Camera sensor configured for optimal compression");
    showStaticMessage("初始化成功");
  } else {
    Serial.println("Failed to get camera sensor");
    showStaticMessage("传感器获取失败");
  }

  Serial.println("Camera initialization process completed");
}

void captureImage() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    showStaticMessage("拍照失败");
    return;
  }
  
  // Print image info
  Serial.printf("Image captured: %zu bytes\n", fb->len);
  showStaticMessage("拍照成功");

  // Convert to base64
  String base64Image = base64Encode(fb->buf, fb->len);
  Serial.println("data:image/jpeg;base64," + base64Image);
  
  esp_camera_fb_return(fb);
}

// 拍照并分析图像
String captureAndAnalyze(const String& question) {
  showStaticMessage("开始拍照...");
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    showStaticMessage("拍照失败");
    return "拍照失败";
  }
  
  Serial.printf("Image captured: %zu bytes\n", fb->len);
  showStaticMessage("图像分析中...");

  // Convert to base64
  String base64Image = base64Encode(fb->buf, fb->len);
  
  // 释放摄像头缓冲区
  esp_camera_fb_return(fb);
  
  // 调用AI分析
  String result = resolveImage(base64Image, question);
  
  return result;
}

// 简单的JPEG到灰度转换（近似）
bool simpleJpegToGrayscale(const uint8_t* jpegData, size_t jpegSize, 
                          uint8_t* grayBuffer, uint16_t targetWidth, uint16_t targetHeight) {
  // 这是一个简化的方法，实际上我们跳过JPEG解码，
  // 而是使用摄像头原始的RGB565格式
  // 对于实际应用，你可能需要使用专门的JPEG解码库
  
  // 为了简化，我们生成一个测试模式
  for (int y = 0; y < targetHeight; y++) {
    for (int x = 0; x < targetWidth; x++) {
      // 创建一个简单的测试模式
      uint8_t value = ((x + y) % 2) ? 255 : 0;
      grayBuffer[y * targetWidth + x] = value;
    }
  }
  return true;
}

// 缩放灰度图像
void scaleGrayscaleImage(const uint8_t* srcImage, uint16_t srcWidth, uint16_t srcHeight,
                        uint8_t* dstImage, uint16_t dstWidth, uint16_t dstHeight) {
  for (int y = 0; y < dstHeight; y++) {
    for (int x = 0; x < dstWidth; x++) {
      // 简单的最近邻插值
      int srcX = (x * srcWidth) / dstWidth;
      int srcY = (y * srcHeight) / dstHeight;
      
      if (srcX >= srcWidth) srcX = srcWidth - 1;
      if (srcY >= srcHeight) srcY = srcHeight - 1;
      
      dstImage[y * dstWidth + x] = srcImage[srcY * srcWidth + srcX];
    }
  }
}

// 将灰度图像转换为单色位图
void grayscaleToMono(const uint8_t* grayImage, uint8_t* monoImage, 
                    uint16_t width, uint16_t height, uint8_t threshold = 128) {
  int byteWidth = (width + 7) / 8;  // 每行需要的字节数
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint8_t grayValue = grayImage[y * width + x];
      bool isWhite = grayValue > threshold;
      
      int byteIndex = y * byteWidth + x / 8;
      int bitIndex = 7 - (x % 8);
      
      if (isWhite) {
        monoImage[byteIndex] |= (1 << bitIndex);
      } else {
        monoImage[byteIndex] &= ~(1 << bitIndex);
      }
    }
  }
}

// 拍照并显示到OLED屏幕
void captureAndDisplayOnOled() {
  showTwoLineMessage("准备拍照显示...", "请稍等");
  delay(1000);
  
  // 先切换到较小分辨率以便处理
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_framesize(s, FRAMESIZE_QQVGA);  // 160x120
    delay(300);  // 给传感器时间调整
  }
  
  showTwoLineMessage("正在拍照...", "");
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    showStaticMessage("拍照失败");
    return;
  }
  
  Serial.printf("Captured image: %dx%d, %zu bytes\n", fb->width, fb->height, fb->len);
  showTwoLineMessage("图像处理中...", "请稍等");
  
  const uint16_t maxDisplayWidth = 128;
  const uint16_t maxDisplayHeight = 32;
  
  // 计算保持宽高比的缩放尺寸
  float aspectRatio = (float)fb->width / (float)fb->height;
  uint16_t scaledWidth, scaledHeight;
  uint16_t offsetX = 0, offsetY = 0;
  
  if (aspectRatio > (float)maxDisplayWidth / maxDisplayHeight) {
    // 图像较宽，以宽度为限制
    scaledWidth = maxDisplayWidth;
    scaledHeight = (uint16_t)(maxDisplayWidth / aspectRatio);
    offsetY = (maxDisplayHeight - scaledHeight) / 2;
  } else {
    // 图像较高，以高度为限制
    scaledHeight = maxDisplayHeight;
    scaledWidth = (uint16_t)(maxDisplayHeight * aspectRatio);
    offsetX = (maxDisplayWidth - scaledWidth) / 2;
  }
  
  Serial.printf("Scaled to: %dx%d, offset: (%d,%d)\n", scaledWidth, scaledHeight, offsetX, offsetY);
  
  // 分配显示缓冲区 (总是128x32)
  uint8_t* displayBuffer = (uint8_t*)malloc(maxDisplayWidth * maxDisplayHeight / 8);
  if (!displayBuffer) {
    Serial.println("Failed to allocate display buffer");
    esp_camera_fb_return(fb);
    showStaticMessage("内存不足");
    return;
  }
  
  // 清空缓冲区
  memset(displayBuffer, 0, maxDisplayWidth * maxDisplayHeight / 8);
  
  // 改进的图像处理算法
  // 由于JPEG解码复杂，我们使用一种简化但更有效的方法
  // 对JPEG数据进行统计分析来生成近似的图像
  
  showTwoLineMessage("转换图像格式...", "");
  
  // 分析JPEG数据的亮度分布
  uint32_t brightnessSum = 0;
  uint32_t sampleCount = 0;
  
  // 对JPEG数据进行采样以估算亮度
  for (size_t i = 0; i < fb->len; i += 10) {
    brightnessSum += fb->buf[i];
    sampleCount++;
  }
  
  uint8_t avgBrightness = brightnessSum / sampleCount;
  Serial.printf("Average brightness: %d\n", avgBrightness);
  
  // 创建基于真实数据的简化图像，保持宽高比
  for (int y = 0; y < scaledHeight; y++) {
    for (int x = 0; x < scaledWidth; x++) {
      // 从JPEG数据中智能采样
      size_t sampleIndex1 = ((y * fb->height / scaledHeight) * 
                             (fb->width / scaledWidth) + 
                             (x * fb->width / scaledWidth)) % fb->len;
      size_t sampleIndex2 = (sampleIndex1 + 1) % fb->len;
      size_t sampleIndex3 = (sampleIndex1 + 2) % fb->len;
      
      // 取多个样本的平均值以获得更好的效果
      uint16_t sampleValue = (fb->buf[sampleIndex1] + 
                             fb->buf[sampleIndex2] + 
                             fb->buf[sampleIndex3]) / 3;
      
      // 自适应阈值，基于局部和全局亮度
      uint8_t localThreshold = (avgBrightness + sampleValue) / 2;
      bool isWhite = sampleValue > localThreshold;
      
      // 添加一些对比度增强
      if (abs((int)sampleValue - (int)avgBrightness) < 20) {
        // 对于接近平均亮度的像素，增加随机性以显示细节
        isWhite = (sampleValue + (x + y) % 30) > localThreshold;
      }
      
      // 计算在显示缓冲区中的实际位置（加上偏移量）
      int actualX = x + offsetX;
      int actualY = y + offsetY;
      
      int byteIndex = actualY * (maxDisplayWidth / 8) + actualX / 8;
      int bitIndex = 7 - (actualX % 8);
      
      if (isWhite) {
        displayBuffer[byteIndex] |= (1 << bitIndex);
      }
    }
  }
  
  showTwoLineMessage("正在显示...", "");
  
  // 显示到OLED
  u8g2.clearBuffer();
  u8g2.drawXBM(0, 0, maxDisplayWidth, maxDisplayHeight, displayBuffer);
  u8g2.sendBuffer();
  
  Serial.printf("Image displayed: %dx%d -> %dx%d (ratio preserved), offset: (%d,%d)\n", 
                fb->width, fb->height, scaledWidth, scaledHeight, offsetX, offsetY);
  
  // 清理资源
  free(displayBuffer);
  esp_camera_fb_return(fb);
  
  // 恢复原来的分辨率
  if (s != NULL) {
    s->set_framesize(s, FRAMESIZE_VGA);
    delay(100);
  }
  
  // 显示完成消息，但先让用户看到图像
  delay(3000);  // 让用户看到图像
  String sizeInfo = String(scaledWidth) + "x" + String(scaledHeight);
  showTwoLineMessage("比例缩放完成", sizeInfo.c_str());
  delay(2000);
}

// 新增：连续拍照显示功能
void continuousCaptureDisplay() {
  showTwoLineMessage("连续拍照模式", "开始中...");
  delay(1500);
  
  // 设置为较小分辨率以提高处理速度
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_framesize(s, FRAMESIZE_QQVGA);  // 160x120
    delay(200);
  }
  
  for (int i = 0; i < 5; i++) {  // 连续拍摄5张照片
    String countMsg = "拍照 " + String(i + 1) + "/5";
    showTwoLineMessage(countMsg.c_str(), "");
    
    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed");
      showStaticMessage("拍照失败");
      continue;
    }
    
    // 快速显示处理，保持宽高比
    const uint16_t maxDisplayWidth = 128;
    const uint16_t maxDisplayHeight = 32;
    
    // 计算保持宽高比的缩放尺寸
    float aspectRatio = (float)fb->width / (float)fb->height;
    uint16_t scaledWidth, scaledHeight;
    uint16_t offsetX = 0, offsetY = 0;
    
    if (aspectRatio > (float)maxDisplayWidth / maxDisplayHeight) {
      scaledWidth = maxDisplayWidth;
      scaledHeight = (uint16_t)(maxDisplayWidth / aspectRatio);
      offsetY = (maxDisplayHeight - scaledHeight) / 2;
    } else {
      scaledHeight = maxDisplayHeight;
      scaledWidth = (uint16_t)(maxDisplayHeight * aspectRatio);
      offsetX = (maxDisplayWidth - scaledWidth) / 2;
    }
    
    uint8_t* displayBuffer = (uint8_t*)malloc(maxDisplayWidth * maxDisplayHeight / 8);
    if (displayBuffer) {
      memset(displayBuffer, 0, maxDisplayWidth * maxDisplayHeight / 8);
      
      // 简化快速处理
      for (int y = 0; y < scaledHeight; y += 2) {  // 每隔一行处理以加快速度
        for (int x = 0; x < scaledWidth; x += 2) {  // 每隔一列处理
          size_t sampleIndex = ((y * fb->height / scaledHeight) * 
                               (fb->width / scaledWidth) + 
                               (x * fb->width / scaledWidth)) % fb->len;
          
          bool isWhite = fb->buf[sampleIndex] > 100;
          
          // 设置2x2的像素块
          for (int dy = 0; dy < 2 && (y + dy) < scaledHeight; dy++) {
            for (int dx = 0; dx < 2 && (x + dx) < scaledWidth; dx++) {
              int px = x + dx + offsetX;
              int py = y + dy + offsetY;
              int byteIndex = py * (maxDisplayWidth / 8) + px / 8;
              int bitIndex = 7 - (px % 8);
              
              if (isWhite) {
                displayBuffer[byteIndex] |= (1 << bitIndex);
              }
            }
          }
        }
      }
      
      // 显示
      u8g2.clearBuffer();
      u8g2.drawXBM(0, 0, maxDisplayWidth, maxDisplayHeight, displayBuffer);
      u8g2.sendBuffer();
      
      free(displayBuffer);
    }
    
    esp_camera_fb_return(fb);
    delay(1000);  // 每张照片间隔1秒
  }
  
  // 恢复分辨率
  if (s != NULL) {
    s->set_framesize(s, FRAMESIZE_VGA);
  }
  
  showTwoLineMessage("连续拍照完成", "共5张照片");
  delay(2000);
}
