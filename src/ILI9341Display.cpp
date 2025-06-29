#include "ILI9341.h"
#include <WiFi.h>

namespace Display
{
  // 全局显示管理器实例
  ILI9341Manager displayManager;
  
  // ==================== ILI9341Manager 类实现 ====================
  
  bool ILI9341Manager::begin()
  {
    if (initialized) {
      return true;
    }
    
    Serial.println("Initializing ILI9341 display...");
    
    // 初始化引脚
    initializePins();

    // TFT对象已在构造函数中初始化

    // 开始初始化显示屏
    tft.begin();
    
    // 设置默认配置
    setupDisplay();
    
    initialized = true;
    Serial.println("ILI9341 display initialized successfully");
    
    return true;
  }
  
  void ILI9341Manager::initializePins()
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
  
  void ILI9341Manager::setupDisplay()
  {
    // 设置横屏显示
    tft.setRotation(1);
    
    // 清屏
    tft.fillScreen(ILI9341_BLACK);
    
    // 设置默认文本参数
    tft.setTextWrap(true);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
  }
  
  void ILI9341Manager::setRotation(uint8_t rotation)
  {
    tft.setRotation(rotation);
  }
  
  void ILI9341Manager::setBrightness(uint8_t brightness)
  {
    // ILI9341 不直接支持亮度控制
    // 可以通过PWM控制背光引脚实现（如果硬件支持）
    // 这里提供接口，具体实现根据硬件而定
  }
  
  void ILI9341Manager::clearScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ILI9341Manager::fillScreen(uint16_t color)
  {
    tft.fillScreen(color);
  }
  
  void ILI9341Manager::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    tft.drawPixel(x, y, color);
  }
  
  void ILI9341Manager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
    tft.drawLine(x0, y0, x1, y1, color);
  }
  
  void ILI9341Manager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.drawRect(x, y, w, h, color);
  }
  
  void ILI9341Manager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    tft.fillRect(x, y, w, h, color);
  }
  
  void ILI9341Manager::displayText(const char* text, int16_t x, int16_t y, 
                                  uint16_t color, uint8_t size)
  {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
  }
  
  void ILI9341Manager::displayCenteredText(const char* text, int16_t y, 
                                          uint16_t color, uint8_t size)
  {
    tft.setTextColor(color);
    tft.setTextSize(size);
    
    // 计算文本宽度并居中
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int16_t x = (SCREEN_WIDTH - w) / 2;
    tft.setCursor(x, y);
    tft.print(text);
  }
  
  void ILI9341Manager::displayMultilineText(const char* text, int16_t x, int16_t y, 
                                           uint16_t color, uint8_t size)
  {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    
    // 简单的多行文本显示
    String str = String(text);
    int lineHeight = 8 * size; // 估算行高
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
  
  void ILI9341Manager::showStartupScreen()
  {
    clearScreen(ILI9341_BLACK);
    
    // 显示标题
    displayCenteredText("Little Gallery", 50, ILI9341_WHITE, 3);
    displayCenteredText("ESP32-C3", 80, ILI9341_CYAN, 2);
    displayCenteredText("Image Viewer", 110, ILI9341_GREEN, 1);
    
    // 显示版本信息
    displayCenteredText("v1.0.0", 140, ILI9341_YELLOW, 1);
    
    // 显示初始化状态
    displayCenteredText("Initializing...", 180, ILI9341_WHITE, 1);
  }
  
  void ILI9341Manager::showWiFiConnecting()
  {
    fillRect(0, 180, SCREEN_WIDTH, 20, ILI9341_BLACK);
    displayCenteredText("Connecting to WiFi...", 180, ILI9341_YELLOW, 1);
  }
  
  void ILI9341Manager::showWiFiConnected(const String& ipAddress)
  {
    fillRect(0, 160, SCREEN_WIDTH, 80, ILI9341_BLACK);
    
    displayCenteredText("WiFi Connected!", 160, ILI9341_GREEN, 1);
    
    String ipText = "IP: " + ipAddress;
    displayCenteredText(ipText.c_str(), 180, ILI9341_CYAN, 1);
    
    String webText = "Web: http://" + ipAddress;
    displayCenteredText(webText.c_str(), 200, ILI9341_YELLOW, 1);
  }
  
  void ILI9341Manager::showSystemInfo(const String& info)
  {
    fillRect(0, 220, SCREEN_WIDTH, 20, ILI9341_BLACK);
    displayText(info.c_str(), 5, 225, ILI9341_WHITE, 1);
  }
  
  void ILI9341Manager::showErrorMessage(const String& error)
  {
    fillRect(0, 100, SCREEN_WIDTH, 40, ILI9341_BLACK);
    displayCenteredText("ERROR:", 100, ILI9341_RED, 2);
    displayCenteredText(error.c_str(), 120, ILI9341_RED, 1);
  }
  
  void ILI9341Manager::showImageInfo(const char* filename, int index, int total)
  {
    // 在屏幕底部显示图片信息
    fillRect(0, 220, SCREEN_WIDTH, 20, ILI9341_BLACK);
    
    String info = String(filename) + " (" + String(index + 1) + "/" + String(total) + ")";
    displayText(info.c_str(), 5, 225, ILI9341_WHITE, 1);
  }
  
  void ILI9341Manager::showNoImageMessage()
  {
    clearScreen(ILI9341_BLACK);
    
    displayCenteredText("No Images Found", 80, ILI9341_WHITE, 2);
    displayCenteredText("Upload images via", 110, ILI9341_CYAN, 1);
    displayCenteredText("web interface", 130, ILI9341_CYAN, 1);
    
    // 显示Web地址提示
    if (WiFi.status() == WL_CONNECTED) {
      String webText = "http://" + WiFi.localIP().toString();
      displayCenteredText(webText.c_str(), 160, ILI9341_YELLOW, 1);
    }
  }
  
  void ILI9341Manager::showLoadingMessage()
  {
    fillRect(0, 100, SCREEN_WIDTH, 40, ILI9341_BLACK);
    displayCenteredText("Loading...", 120, ILI9341_YELLOW, 2);
  }
  
  // ==================== 便捷函数实现 ====================
  
  void setup()
  {
    displayManager.begin();
    displayManager.showStartupScreen();
  }
  
  void clearScreen()
  {
    displayManager.clearScreen();
  }
  
  void displayText(const char* text, int x, int y, uint16_t color)
  {
    displayManager.displayText(text, x, y, color);
  }
  
  Adafruit_ILI9341& getTFT()
  {
    return displayManager.getTFT();
  }
}
