#include "ST7789Driver.h"
#include <WiFi.h>

namespace Display
{
  // ==================== ST7789Driver 类实现 ====================
  
  ST7789Driver::ST7789Driver() 
    : tft(TFT_CS, TFT_DC, TFT_RST), initialized(false)
  {
  }
  
  ST7789Driver::~ST7789Driver()
  {
    // 清理资源
  }
  
  bool ST7789Driver::begin()
  {
    if (initialized) {
      return true;
    }
    
    Serial.println("Initializing ST7789 display...");
    
    // 初始化引脚
    initializePins();

    // 开始初始化显示屏
    tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // 设置默认配置
    setupDisplay();
    
    initialized = true;
    Serial.println("ST7789 display initialized successfully");
    
    return true;
  }
  
  void ST7789Driver::initializePins()
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
  
  void ST7789Driver::setupDisplay()
  {
    // 设置默认方向
    tft.setRotation(1); // 横屏模式
    
    // 清屏
    tft.fillScreen(ST77XX_BLACK);
    
    // 设置默认文本颜色和大小
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    
    // 显示初始化完成信息
    showStartupScreen();
  }
  
  void ST7789Driver::setRotation(uint8_t rotation)
  {
    tft.setRotation(rotation);
  }
  
  void ST7789Driver::setBrightness(uint8_t brightness)
  {
    // ST7789 不直接支持亮度控制
    // 可以通过PWM控制背光引脚实现
    // 这里暂时留空，可根据硬件配置实现
  }
  
  void ST7789Driver::clearScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ST7789Driver::fillScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ST7789Driver::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    tft.drawPixel(x, y, color);
  }
  
  void ST7789Driver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
    tft.drawLine(x0, y0, x1, y1, color);
  }
  
  void ST7789Driver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.drawRect(x, y, w, h, color);
  }
  
  void ST7789Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.fillRect(x, y, w, h, color);
  }
  
  void ST7789Driver::displayText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
  {
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.print(text);
  }
  
  void ST7789Driver::displayCenteredText(const char* text, int16_t y, uint16_t color, uint8_t size)
  {
    tft.setTextSize(size);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int16_t x = (getWidth() - w) / 2;
    displayText(text, x, y, color, size);
  }
  
  void ST7789Driver::displayMultilineText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
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
  
  void ST7789Driver::showStartupScreen()
  {
    clearScreen(ST77XX_BLACK);
    
    // 显示标题
    displayCenteredText("Little Gallery ESP32", 50, ST77XX_CYAN, 2);
    displayCenteredText("ST7789 Display", 80, ST77XX_WHITE, 1);
    
    // 显示版本信息
    displayCenteredText("Initializing...", 120, ST77XX_YELLOW, 1);
    
    // 绘制装饰边框
    drawRect(10, 10, getWidth()-20, getHeight()-20, ST77XX_BLUE);
    drawRect(12, 12, getWidth()-24, getHeight()-24, ST77XX_BLUE);
  }
  
  void ST7789Driver::showWiFiConnecting()
  {
    clearScreen(ST77XX_BLACK);
    displayCenteredText("Connecting to WiFi...", 100, ST77XX_YELLOW, 2);
    
    // 显示连接动画点
    for (int i = 0; i < 3; i++) {
      displayText(".", 140 + i * 10, 130, ST77XX_WHITE, 2);
      delay(500);
    }
  }
  
  void ST7789Driver::showWiFiConnected(const String& ipAddress)
  {
    clearScreen(ST77XX_BLACK);
    displayCenteredText("WiFi Connected!", 60, ST77XX_GREEN, 2);

    displayCenteredText("IP Address:", 90, ST77XX_WHITE, 1);
    displayCenteredText(ipAddress.c_str(), 105, ST77XX_CYAN, 1);

    displayCenteredText("mDNS Address:", 125, ST77XX_WHITE, 1);
    String mdnsAddr = String(MDNS_HOSTNAME) + ".local";
    displayCenteredText(mdnsAddr.c_str(), 140, ST77XX_MAGENTA, 1);

    displayCenteredText("Ready to display images!", 170, ST77XX_YELLOW, 1);
  }
  
  void ST7789Driver::showSystemInfo(const String& info)
  {
    clearScreen(ST77XX_BLACK);
    displayCenteredText("System Info", 20, ST77XX_CYAN, 2);
    displayMultilineText(info.c_str(), 10, 60, ST77XX_WHITE, 1);
  }
  
  void ST7789Driver::showErrorMessage(const String& error)
  {
    clearScreen(ST77XX_RED);
    displayCenteredText("ERROR", 50, ST77XX_WHITE, 3);
    displayCenteredText(error.c_str(), 100, ST77XX_WHITE, 1);
  }
  
  void ST7789Driver::showImageInfo(const char* filename, int index, int total)
  {
    // 在屏幕底部显示图片信息
    fillRect(0, getHeight()-30, getWidth(), 30, ST77XX_BLACK);
    
    String info = String(index + 1) + "/" + String(total) + " " + String(filename);
    displayText(info.c_str(), 5, getHeight()-25, ST77XX_WHITE, 1);
  }
  
  void ST7789Driver::showNoImageMessage()
  {
    clearScreen(ST77XX_BLACK);
    displayCenteredText("No Images Found", 100, ST77XX_YELLOW, 2);
    displayCenteredText("Please upload some images", 130, ST77XX_WHITE, 1);
    displayCenteredText("via web interface", 150, ST77XX_WHITE, 1);
  }
  
  void ST7789Driver::showLoadingMessage()
  {
    clearScreen(ST77XX_BLACK);
    displayCenteredText("Loading Image...", 100, ST77XX_CYAN, 2);
  }
  
  void ST7789Driver::drawFileName(const char* filename)
  {
    // 在屏幕顶部显示文件名
    fillRect(0, 0, getWidth(), 20, ST77XX_BLACK);
    displayText(filename, 5, 5, ST77XX_WHITE, 1);
  }
  
  void ST7789Driver::testColorDisplay()
  {
    clearScreen(ST77XX_BLACK);
    
    // 显示颜色测试条
    int barHeight = getHeight() / 8;
    fillRect(0, 0 * barHeight, getWidth(), barHeight, ST77XX_RED);
    fillRect(0, 1 * barHeight, getWidth(), barHeight, ST77XX_GREEN);
    fillRect(0, 2 * barHeight, getWidth(), barHeight, ST77XX_BLUE);
    fillRect(0, 3 * barHeight, getWidth(), barHeight, ST77XX_YELLOW);
    fillRect(0, 4 * barHeight, getWidth(), barHeight, ST77XX_CYAN);
    fillRect(0, 5 * barHeight, getWidth(), barHeight, ST77XX_MAGENTA);
    fillRect(0, 6 * barHeight, getWidth(), barHeight, ST77XX_WHITE);
    fillRect(0, 7 * barHeight, getWidth(), barHeight, ST77XX_BLACK);
    
    displayCenteredText("Color Test", getHeight()/2 - 10, ST77XX_BLACK, 2);
    
    delay(3000);
    clearScreen(ST77XX_BLACK);
  }
}
