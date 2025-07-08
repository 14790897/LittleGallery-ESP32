#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "secrets.h"
#include "DisplayDriver.h"
#include "WebServer.h"
#include "ImageDisplay.h"

// ==================== 全局变量 ====================
unsigned long lastImageUpdate = 0;
String lastDisplayedImage = "";
int lastImageIndex = -1;

// ==================== 函数声明 ====================
void updateDisplayedImage();
bool hasImageChanged();

// ==================== 主程序函数 ====================

void setup()
{
  // 初始化串口
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Starting Little Gallery ESP32...");

  // 初始化显示屏 (从platformio.ini配置中读取)
#ifndef DEFAULT_DISPLAY_DRIVER
#define DEFAULT_DISPLAY_DRIVER DRIVER_ILI9341 // 默认备选
#endif
  Display::setup(DEFAULT_DISPLAY_DRIVER);

  // 初始化图片显示
  ImageDisplay::setup();

  // 显示WiFi连接状态
  Display::displayManager.showWiFiConnecting();

  // 初始化Web服务器
  if (!WebServerManager::setup()) {
    Display::displayManager.showErrorMessage("Failed to start web server");
    return;
  }

  // 显示连接成功信息
  if (WebServerManager::isWiFiConnected()) {
    Display::displayManager.showWiFiConnected(WebServerManager::getIPAddress());
    delay(3000); // 显示连接信息3秒
  }

  // 显示第一张图片或无图片消息
  updateDisplayedImage();

  Serial.println("Setup complete. Ready to display images!");
}

// ==================== 辅助函数 ====================

void updateDisplayedImage()
{
  String currentImage = WebServerManager::getCurrentImageName();
  int currentIndex = WebServerManager::getCurrentImageIndex();
  int totalImages = WebServerManager::getImageCount();

  if (currentImage != "No image" && !currentImage.isEmpty()) {
    if (ImageDisplay::displayImage(currentImage.c_str())) {
      // ImageDisplay::showImageInfo(currentImage.c_str(), currentIndex, totalImages);
    } else {
      ImageDisplay::showErrorMessage("Failed to display: " + currentImage);
    }
  } else {
    ImageDisplay::showNoImageMessage();
  }

  lastDisplayedImage = currentImage;
  lastImageIndex = currentIndex;
}

bool hasImageChanged()
{
  String currentImage = WebServerManager::getCurrentImageName();
  int currentIndex = WebServerManager::getCurrentImageIndex();

  return (currentImage != lastDisplayedImage || currentIndex != lastImageIndex);
}

void loop()
{
  unsigned long now = millis();

  // 更新幻灯片（如果启用）
  WebServerManager::webServerController.updateSlideshow();

  // 定期检查是否需要更新显示的图片
  if (now - lastImageUpdate >= IMAGE_UPDATE_INTERVAL) {
    if (hasImageChanged()) {
      Serial.printf("Image changed: %s (index: %d)\n",
                   WebServerManager::getCurrentImageName().c_str(),
                   WebServerManager::getCurrentImageIndex());

      updateDisplayedImage();
    }

    lastImageUpdate = now;
  }

  // 让其他任务有机会运行
  delay(10);
}
