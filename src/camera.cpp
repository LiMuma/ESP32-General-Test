#include <Arduino.h>
#include "esp_camera.h"

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
void showUTF8Status(const char* title, const char* message);

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
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
    config.jpeg_quality = 12;  // 在PSRAM环境下稍微提高质量
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_QVGA; // 320x240 更小的分辨率
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.jpeg_quality = 20; // 进一步降低质量以适应内存限制
  }

  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    showUTF8Status("Error:", "摄像头初始化失败");
    return;
  }
  
  // 获取传感器设置并优化
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    // 设置图像参数以获得更好的压缩比
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    
    Serial.println("Camera sensor configured for optimal compression");
  }
  
  Serial.println("Camera initialized successfully with optimized settings");
  showUTF8Status("Camera:", "初始化成功");
}

void captureImage() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    showUTF8Status("Error:", "拍照失败");
    return;
  }
  
  // Print image info
  Serial.printf("Image captured: %zu bytes\n", fb->len);
  showUTF8Status("Camera:", "拍照成功");

  // Convert to base64
  String base64Image = base64Encode(fb->buf, fb->len);
  Serial.println("data:image/jpeg;base64," + base64Image);
  
  esp_camera_fb_return(fb);
}

// 拍照并分析图像
String captureAndAnalyze(const String& question) {
  showUTF8Status("Camera:", "开始拍照...");
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    showUTF8Status("Error:", "拍照失败");
    return "拍照失败";
  }
  
  Serial.printf("Image captured: %zu bytes\n", fb->len);
  showUTF8Status("AI:", "图像分析中...");

  // Convert to base64
  String base64Image = base64Encode(fb->buf, fb->len);
  
  // 释放摄像头缓冲区
  esp_camera_fb_return(fb);
  
  // 调用AI分析
  String result = resolveImage(base64Image, question);
  
  return result;
}
