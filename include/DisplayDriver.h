#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Adafruit_GFX.h>
#include <SPI.h>
#include "secrets.h"

// 显示驱动类型枚举
enum DisplayDriverType {
  DRIVER_ILI9341,
  DRIVER_ST7789
};

namespace Display
{
  // ==================== 抽象显示驱动基类 ====================
  class DisplayDriverBase
  {
  public:
    virtual ~DisplayDriverBase() = default;
    
    // 纯虚函数 - 必须在子类中实现
    virtual bool begin() = 0;
    virtual void setRotation(uint8_t rotation) = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    
    // 基础绘制功能
    virtual void clearScreen(uint16_t color = 0x0000) = 0;
    virtual void fillScreen(uint16_t color) = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    
    // 文本显示功能
    virtual void displayText(const char* text, int16_t x = 10, int16_t y = 10,
                           uint16_t color = 0xFFFF, uint8_t size = 1) = 0;
    virtual void displayCenteredText(const char* text, int16_t y,
                                   uint16_t color = 0xFFFF, uint8_t size = 1) = 0;
    virtual void displayMultilineText(const char* text, int16_t x, int16_t y,
                                    uint16_t color = 0xFFFF, uint8_t size = 1) = 0;
    
    // 状态显示功能
    virtual void showStartupScreen() = 0;
    virtual void showWiFiConnecting() = 0;
    virtual void showWiFiConnected(const String& ipAddress) = 0;
    virtual void showSystemInfo(const String& info) = 0;
    virtual void showErrorMessage(const String& error) = 0;
    
    // 图片信息显示
    virtual void showImageInfo(const char* filename, int index, int total) = 0;
    virtual void showNoImageMessage() = 0;
    virtual void showLoadingMessage() = 0;
    virtual void drawFileName(const char* filename) = 0;
    
    // 获取显示屏对象（用于高级操作）
    virtual Adafruit_GFX& getGFX() = 0;
    
    // 获取屏幕尺寸
    virtual int16_t getWidth() const = 0;
    virtual int16_t getHeight() const = 0;
    
    // 获取驱动类型
    virtual DisplayDriverType getDriverType() const = 0;
    virtual const char* getDriverName() const = 0;
  };

  // ==================== 显示管理器 ====================
  class DisplayManager
  {
  public:
    DisplayManager();
    ~DisplayManager();
    
    // 驱动切换
    bool setDriver(DisplayDriverType driverType);
    DisplayDriverType getCurrentDriver() const { return currentDriverType; }
    const char* getCurrentDriverName() const;
    
    // 初始化
    bool begin();
    bool begin(DisplayDriverType driverType);
    
    // 代理方法 - 转发到当前驱动
    void setRotation(uint8_t rotation);
    void setBrightness(uint8_t brightness);
    
    void clearScreen(uint16_t color = 0x0000);
    void fillScreen(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    void displayText(const char* text, int16_t x = 10, int16_t y = 10,
                    uint16_t color = 0xFFFF, uint8_t size = 1);
    void displayCenteredText(const char* text, int16_t y,
                           uint16_t color = 0xFFFF, uint8_t size = 1);
    void displayMultilineText(const char* text, int16_t x, int16_t y,
                            uint16_t color = 0xFFFF, uint8_t size = 1);
    
    void showStartupScreen();
    void showWiFiConnecting();
    void showWiFiConnected(const String& ipAddress);
    void showSystemInfo(const String& info);
    void showErrorMessage(const String& error);
    
    void showImageInfo(const char* filename, int index, int total);
    void showNoImageMessage();
    void showLoadingMessage();
    void drawFileName(const char* filename);
    
    Adafruit_GFX& getGFX();
    int16_t getWidth() const;
    int16_t getHeight() const;
    
  private:
    DisplayDriverBase* currentDriver;
    DisplayDriverType currentDriverType;
    bool initialized;
    
    // 创建驱动实例
    DisplayDriverBase* createDriver(DisplayDriverType driverType);
    void destroyDriver(DisplayDriverBase* driver);
  };

  // 全局显示管理器实例
  extern DisplayManager displayManager;

  // 便捷函数（向后兼容）
  void setup();
  void setup(DisplayDriverType driverType);
  void clearScreen();
  void displayText(const char* text, int x = 10, int y = 10, uint16_t color = 0xFFFF);
  Adafruit_GFX& getGFX();
  
  // 驱动切换函数
  bool switchDriver(DisplayDriverType driverType);
  DisplayDriverType getCurrentDriver();
  const char* getCurrentDriverName();
}

#endif // DISPLAY_DRIVER_H
