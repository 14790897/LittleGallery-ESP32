#ifndef ILI9341_DISPLAY_H
#define ILI9341_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "secrets.h"

namespace Display
{
  // ==================== 显示屏管理类 ====================
  class ILI9341Manager
  {
  public:
    // 构造函数
    ILI9341Manager() : tft(TFT_CS, TFT_DC, TFT_RST), initialized(false) {}

    // 初始化和设置
    bool begin();
    void setRotation(uint8_t rotation);
    void setBrightness(uint8_t brightness);

    // 基础绘制功能
    void clearScreen(uint16_t color = ILI9341_BLACK);
    void fillScreen(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // 文本显示功能
    void displayText(const char* text, int16_t x = 10, int16_t y = 10,
                    uint16_t color = ILI9341_WHITE, uint8_t size = 1);
    void displayCenteredText(const char* text, int16_t y,
                           uint16_t color = ILI9341_WHITE, uint8_t size = 1);
    void displayMultilineText(const char* text, int16_t x, int16_t y,
                            uint16_t color = ILI9341_WHITE, uint8_t size = 1);

    // 状态显示功能
    void showStartupScreen();
    void showWiFiConnecting();
    void showWiFiConnected(const String& ipAddress);
    void showSystemInfo(const String& info);
    void showErrorMessage(const String& error);

    // 图片信息显示
    void showImageInfo(const char* filename, int index, int total);
    void showNoImageMessage();
    void showLoadingMessage();
    void drawFileName(const char* filename);

    // 获取显示屏对象（用于高级操作）
    Adafruit_ILI9341& getTFT() { return tft; }

    // 获取屏幕尺寸
    int16_t getWidth() const { return SCREEN_WIDTH; }
    int16_t getHeight() const { return SCREEN_HEIGHT; }

  private:
    Adafruit_ILI9341 tft;
    bool initialized;

    // 私有辅助函数
    void initializePins();
    void setupDisplay();
    void testColorDisplay();
  };

  // 全局显示管理器实例
  extern ILI9341Manager displayManager;

  // 便捷函数（向后兼容）
  void setup();
  void clearScreen();
  void displayText(const char* text, int x = 10, int y = 10, uint16_t color = ILI9341_WHITE);
  Adafruit_ILI9341& getTFT();
}

#endif // ILI9341_DISPLAY_H
