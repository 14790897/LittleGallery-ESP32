#include "DisplayDriver.h"
#include <WiFi.h>

namespace Display
{
  // 这个文件现在作为向后兼容层存在
  // 实际的显示功能已经移到 DisplayManager 和具体的驱动类中

  // ==================== 向后兼容函数 ====================

  bool begin()
  {
    return displayManager.begin();
  }

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

  void displayText(const char *text, int x, int y, uint16_t color)
  {
    displayManager.displayText(text, x, y, color);
  }

  Adafruit_GFX &getGFX()
  {
    return displayManager.getGFX();
  }
}
