#include "ILI9341Driver.h"
#include <WiFi.h>

namespace Display
{
  // ==================== ILI9341Driver 类实现 ====================
  
  ILI9341Driver::ILI9341Driver() 
    : tft(TFT_CS, TFT_DC, TFT_RST), initialized(false)
  {
  }
  
  ILI9341Driver::~ILI9341Driver()
  {
    // 清理资源
  }
  
  bool ILI9341Driver::begin()
  {
    if (initialized) {
      return true;
    }
    
    Serial.println("Initializing ILI9341 display...");
    
    // 初始化引脚
    initializePins();

    // 开始初始化显示屏
    tft.begin();
    
    // 设置默认配置
    setupDisplay();
    
    initialized = true;
    Serial.println("ILI9341 display initialized successfully");
    
    return true;
  }
  
  void ILI9341Driver::initializePins()
  {
    // 配置SPI引脚（如果需要）
    // ESP32-C3 默认SPI引脚: SCK=4, MOSI=5, MISO=6
    // 这里主要是确保CS, DC, RST引脚正确配置
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_RST, HIGH);
  }
  
  void ILI9341Driver::setupDisplay()
  {
    // 设置默认方向
    tft.setRotation(1); // 横屏模式
    
    // 清屏
    tft.fillScreen(ILI9341_BLACK);
    
    // 设置默认文本颜色和大小
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    
    // 显示初始化完成信息
    showStartupScreen();
  }
  
  void ILI9341Driver::setRotation(uint8_t rotation)
  {
    tft.setRotation(rotation);
  }
  
  void ILI9341Driver::setBrightness(uint8_t brightness)
  {
    // ILI9341 不直接支持亮度控制
    // 可以通过PWM控制背光引脚实现
    // 这里暂时留空，可根据硬件配置实现
  }
  
  void ILI9341Driver::clearScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ILI9341Driver::fillScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ILI9341Driver::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    tft.drawPixel(x, y, color);
  }
  
  void ILI9341Driver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
    tft.drawLine(x0, y0, x1, y1, color);
  }
  
  void ILI9341Driver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.drawRect(x, y, w, h, color);
  }
  
  void ILI9341Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.fillRect(x, y, w, h, color);
  }
  
  void ILI9341Driver::displayText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
  {
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.print(text);
  }
  
  void ILI9341Driver::displayCenteredText(const char* text, int16_t y, uint16_t color, uint8_t size)
  {
    tft.setTextSize(size);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int16_t x = (getWidth() - w) / 2;
    displayText(text, x, y, color, size);
  }
  
  void ILI9341Driver::displayMultilineText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
  {
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    
    String str = String(text);
    int lineHeight = 8 * size;
    int currentY = y;
    
    int start = 0;
    int end = str.indexOf('\n');
    
    while (end != -1) {
      String line = str.substring(start, end);
      tft.setCursor(x, currentY);
      tft.print(line);
      currentY += lineHeight;
      
      start = end + 1;
      end = str.indexOf('\n', start);
    }
    
    // 打印最后一行
    if (start < str.length()) {
      String line = str.substring(start);
      tft.setCursor(x, currentY);
      tft.print(line);
    }
  }
  
  void ILI9341Driver::showStartupScreen()
  {
    clearScreen(ILI9341_BLACK);
    
    // 显示标题
    displayCenteredText("Little Gallery ESP32", 50, ILI9341_CYAN, 2);
    displayCenteredText("ILI9341 Display", 80, ILI9341_WHITE, 1);
    
    // 显示版本信息
    displayCenteredText("Initializing...", 120, ILI9341_YELLOW, 1);
    
    // 绘制装饰边框
    drawRect(10, 10, getWidth()-20, getHeight()-20, ILI9341_BLUE);
    drawRect(12, 12, getWidth()-24, getHeight()-24, ILI9341_BLUE);
  }
  
  void ILI9341Driver::showWiFiConnecting()
  {
    clearScreen(ILI9341_BLACK);
    displayCenteredText("Connecting to WiFi...", 100, ILI9341_YELLOW, 2);
    
    // 显示连接动画点
    for (int i = 0; i < 3; i++) {
      displayText(".", 140 + i * 10, 130, ILI9341_WHITE, 2);
      delay(500);
    }
  }
  
  void ILI9341Driver::showWiFiConnected(const String& ipAddress)
  {
    clearScreen(ILI9341_BLACK);
    displayCenteredText("WiFi Connected!", 80, ILI9341_GREEN, 2);
    displayCenteredText("IP Address:", 110, ILI9341_WHITE, 1);
    displayCenteredText(ipAddress.c_str(), 130, ILI9341_CYAN, 1);
    displayCenteredText("Ready to display images!", 160, ILI9341_YELLOW, 1);
  }
  
  void ILI9341Driver::showSystemInfo(const String& info)
  {
    clearScreen(ILI9341_BLACK);
    displayCenteredText("System Info", 20, ILI9341_CYAN, 2);
    displayMultilineText(info.c_str(), 10, 60, ILI9341_WHITE, 1);
  }
  
  void ILI9341Driver::showErrorMessage(const String& error)
  {
    clearScreen(ILI9341_RED);
    displayCenteredText("ERROR", 50, ILI9341_WHITE, 3);
    displayCenteredText(error.c_str(), 100, ILI9341_WHITE, 1);
  }
  
  void ILI9341Driver::showImageInfo(const char* filename, int index, int total)
  {
    // 在屏幕底部显示图片信息
    fillRect(0, getHeight()-30, getWidth(), 30, ILI9341_BLACK);
    
    String info = String(index + 1) + "/" + String(total) + " " + String(filename);
    displayText(info.c_str(), 5, getHeight()-25, ILI9341_WHITE, 1);
  }
  
  void ILI9341Driver::showNoImageMessage()
  {
    clearScreen(ILI9341_BLACK);
    displayCenteredText("No Images Found", 100, ILI9341_YELLOW, 2);
    displayCenteredText("Please upload some images", 130, ILI9341_WHITE, 1);
    displayCenteredText("via web interface", 150, ILI9341_WHITE, 1);
  }
  
  void ILI9341Driver::showLoadingMessage()
  {
    clearScreen(ILI9341_BLACK);
    displayCenteredText("Loading Image...", 100, ILI9341_CYAN, 2);
  }
  
  void ILI9341Driver::drawFileName(const char* filename)
  {
    // 在屏幕顶部显示文件名
    fillRect(0, 0, getWidth(), 20, ILI9341_BLACK);
    displayText(filename, 5, 5, ILI9341_WHITE, 1);
  }
  
  void ILI9341Driver::testColorDisplay()
  {
    clearScreen(ILI9341_BLACK);
    
    // 显示颜色测试条
    int barHeight = getHeight() / 8;
    fillRect(0, 0 * barHeight, getWidth(), barHeight, ILI9341_RED);
    fillRect(0, 1 * barHeight, getWidth(), barHeight, ILI9341_GREEN);
    fillRect(0, 2 * barHeight, getWidth(), barHeight, ILI9341_BLUE);
    fillRect(0, 3 * barHeight, getWidth(), barHeight, ILI9341_YELLOW);
    fillRect(0, 4 * barHeight, getWidth(), barHeight, ILI9341_CYAN);
    fillRect(0, 5 * barHeight, getWidth(), barHeight, ILI9341_MAGENTA);
    fillRect(0, 6 * barHeight, getWidth(), barHeight, ILI9341_WHITE);
    fillRect(0, 7 * barHeight, getWidth(), barHeight, ILI9341_BLACK);
    
    displayCenteredText("Color Test", getHeight()/2 - 10, ILI9341_BLACK, 2);
    
    delay(3000);
    clearScreen(ILI9341_BLACK);
  }
}
