#include "DisplayDriver.h"
#include "ILI9341Driver.h"
#include "ST7789Driver.h"
#include <WiFi.h>

namespace Display
{
  // 全局显示管理器实例
  DisplayManager displayManager;
  
  // ==================== DisplayManager 类实现 ====================
  
  DisplayManager::DisplayManager() 
    : currentDriver(nullptr), currentDriverType(DRIVER_ILI9341), initialized(false)
  {
  }
  
  DisplayManager::~DisplayManager()
  {
    if (currentDriver) {
      destroyDriver(currentDriver);
    }
  }
  
  bool DisplayManager::setDriver(DisplayDriverType driverType)
  {
    if (currentDriverType == driverType && currentDriver != nullptr) {
      return true; // 已经是目标驱动
    }
    
    Serial.printf("Switching display driver to: %s\n", 
                  driverType == DRIVER_ILI9341 ? "ILI9341" : "ST7789");
    
    // 销毁当前驱动
    if (currentDriver) {
      destroyDriver(currentDriver);
      currentDriver = nullptr;
    }
    
    // 创建新驱动
    currentDriver = createDriver(driverType);
    if (!currentDriver) {
      Serial.println("Failed to create display driver");
      return false;
    }
    
    currentDriverType = driverType;
    initialized = false;
    
    return true;
  }
  
  const char* DisplayManager::getCurrentDriverName() const
  {
    if (currentDriver) {
      return currentDriver->getDriverName();
    }
    return "None";
  }
  
  bool DisplayManager::begin()
  {
    if (initialized && currentDriver) {
      return true;
    }
    
    if (!currentDriver) {
      // 使用默认驱动
      if (!setDriver(DRIVER_ILI9341)) {
        return false;
      }
    }
    
    Serial.printf("Initializing %s display driver...\n", getCurrentDriverName());
    
    if (!currentDriver->begin()) {
      Serial.println("Failed to initialize display driver");
      return false;
    }
    
    initialized = true;
    Serial.printf("%s display driver initialized successfully\n", getCurrentDriverName());
    
    return true;
  }
  
  bool DisplayManager::begin(DisplayDriverType driverType)
  {
    if (!setDriver(driverType)) {
      return false;
    }
    
    return begin();
  }
  
  DisplayDriverBase* DisplayManager::createDriver(DisplayDriverType driverType)
  {
    switch (driverType) {
      case DRIVER_ILI9341:
        return new ILI9341Driver();
      case DRIVER_ST7789:
        return new ST7789Driver();
      default:
        Serial.println("Unknown display driver type");
        return nullptr;
    }
  }
  
  void DisplayManager::destroyDriver(DisplayDriverBase* driver)
  {
    if (driver) {
      delete driver;
    }
  }
  
  // ==================== 代理方法实现 ====================
  
  void DisplayManager::setRotation(uint8_t rotation)
  {
    if (currentDriver) currentDriver->setRotation(rotation);
  }
  
  void DisplayManager::setBrightness(uint8_t brightness)
  {
    if (currentDriver) currentDriver->setBrightness(brightness);
  }
  
  void DisplayManager::clearScreen(uint16_t color)
  {
    if (currentDriver) currentDriver->clearScreen(color);
  }
  
  void DisplayManager::fillScreen(uint16_t color)
  {
    if (currentDriver) currentDriver->fillScreen(color);
  }
  
  void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color)
  {
    if (currentDriver) currentDriver->drawPixel(x, y, color);
  }
  
  void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
  {
    if (currentDriver) currentDriver->drawLine(x0, y0, x1, y1, color);
  }
  
  void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    if (currentDriver) currentDriver->drawRect(x, y, w, h, color);
  }
  
  void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  {
    if (currentDriver) currentDriver->fillRect(x, y, w, h, color);
  }
  
  void DisplayManager::displayText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
  {
    if (currentDriver) currentDriver->displayText(text, x, y, color, size);
  }
  
  void DisplayManager::displayCenteredText(const char* text, int16_t y, uint16_t color, uint8_t size)
  {
    if (currentDriver) currentDriver->displayCenteredText(text, y, color, size);
  }
  
  void DisplayManager::displayMultilineText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size)
  {
    if (currentDriver) currentDriver->displayMultilineText(text, x, y, color, size);
  }
  
  void DisplayManager::showStartupScreen()
  {
    if (currentDriver) currentDriver->showStartupScreen();
  }
  
  void DisplayManager::showWiFiConnecting()
  {
    if (currentDriver) currentDriver->showWiFiConnecting();
  }
  
  void DisplayManager::showWiFiConnected(const String& ipAddress)
  {
    if (currentDriver) currentDriver->showWiFiConnected(ipAddress);
  }
  
  void DisplayManager::showSystemInfo(const String& info)
  {
    if (currentDriver) currentDriver->showSystemInfo(info);
  }
  
  void DisplayManager::showErrorMessage(const String& error)
  {
    if (currentDriver) currentDriver->showErrorMessage(error);
  }
  
  void DisplayManager::showImageInfo(const char* filename, int index, int total)
  {
    if (currentDriver) currentDriver->showImageInfo(filename, index, total);
  }
  
  void DisplayManager::showNoImageMessage()
  {
    if (currentDriver) currentDriver->showNoImageMessage();
  }
  
  void DisplayManager::showLoadingMessage()
  {
    if (currentDriver) currentDriver->showLoadingMessage();
  }
  
  void DisplayManager::drawFileName(const char* filename)
  {
    if (currentDriver) currentDriver->drawFileName(filename);
  }
  
  Adafruit_GFX& DisplayManager::getGFX()
  {
    if (currentDriver) {
      return currentDriver->getGFX();
    }
    // 这里需要返回一个有效的引用，但这种情况不应该发生
    static Adafruit_GFX* dummy = nullptr;
    return *dummy; // 这会导致崩溃，但表明了错误使用
  }
  
  int16_t DisplayManager::getWidth() const
  {
    if (currentDriver) return currentDriver->getWidth();
    return SCREEN_WIDTH;
  }
  
  int16_t DisplayManager::getHeight() const
  {
    if (currentDriver) return currentDriver->getHeight();
    return SCREEN_HEIGHT;
  }
  
  // ==================== 便捷函数实现 ====================
  
  void setup()
  {
    displayManager.begin();
  }
  
  void setup(DisplayDriverType driverType)
  {
    displayManager.begin(driverType);
  }
  
  void clearScreen()
  {
    displayManager.clearScreen();
  }
  
  void displayText(const char* text, int x, int y, uint16_t color)
  {
    displayManager.displayText(text, x, y, color);
  }
  
  Adafruit_GFX& getGFX()
  {
    return displayManager.getGFX();
  }
  
  bool switchDriver(DisplayDriverType driverType)
  {
    if (displayManager.setDriver(driverType)) {
      return displayManager.begin();
    }
    return false;
  }
  
  DisplayDriverType getCurrentDriver()
  {
    return displayManager.getCurrentDriver();
  }
  
  const char* getCurrentDriverName()
  {
    return displayManager.getCurrentDriverName();
  }
}
