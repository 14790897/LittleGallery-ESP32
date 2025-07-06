#ifndef ST7789_DRIVER_H
#define ST7789_DRIVER_H

#include "DisplayDriver.h"
#include <Adafruit_ST7789.h>

namespace Display
{
  // ==================== ST7789驱动实现 ====================
  class ST7789Driver : public DisplayDriverBase
  {
  public:
    ST7789Driver();
    virtual ~ST7789Driver();
    
    // 实现基类的纯虚函数
    bool begin() override;
    void setRotation(uint8_t rotation) override;
    void setBrightness(uint8_t brightness) override;
    
    // 基础绘制功能
    void clearScreen(uint16_t color = ST77XX_BLACK) override;
    void fillScreen(uint16_t color) override;
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    // 文本显示功能
    void displayText(const char* text, int16_t x = 10, int16_t y = 10,
                    uint16_t color = ST77XX_WHITE, uint8_t size = 1) override;
    void displayCenteredText(const char* text, int16_t y,
                           uint16_t color = ST77XX_WHITE, uint8_t size = 1) override;
    void displayMultilineText(const char* text, int16_t x, int16_t y,
                            uint16_t color = ST77XX_WHITE, uint8_t size = 1) override;
    
    // 状态显示功能
    void showStartupScreen() override;
    void showWiFiConnecting() override;
    void showWiFiConnected(const String& ipAddress) override;
    void showSystemInfo(const String& info) override;
    void showErrorMessage(const String& error) override;
    
    // 图片信息显示
    void showImageInfo(const char* filename, int index, int total) override;
    void showNoImageMessage() override;
    void showLoadingMessage() override;
    void drawFileName(const char* filename) override;
    
    // 获取显示屏对象
    Adafruit_GFX& getGFX() override { return tft; }
    Adafruit_ST7789& getTFT() { return tft; }
    
    // 获取屏幕尺寸
    int16_t getWidth() const override { return SCREEN_WIDTH; }
    int16_t getHeight() const override { return SCREEN_HEIGHT; }
    
    // 获取驱动信息
    DisplayDriverType getDriverType() const override { return DRIVER_ST7789; }
    const char* getDriverName() const override { return "ST7789"; }
    
  private:
    Adafruit_ST7789 tft;
    bool initialized;
    
    // 私有辅助函数
    void initializePins();
    void setupDisplay();
    void testColorDisplay();
  };
}

#endif // ST7789_DRIVER_H
