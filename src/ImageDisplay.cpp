#include "ImageDisplay.h"
#include "ILI9341.h"

namespace ImageDisplay
{
  // 全局图片显示管理器实例
  ImageDisplayManager imageDisplayManager;

  // ==================== ImageDisplayManager 类实现 ====================

  bool ImageDisplayManager::begin()
  {
    if (initialized) {
      return true;
    }

    Serial.println("Initializing Image Display Manager...");

    // 初始化JPEG解码器
    if (!initJPEGDecoder()) {
      Serial.println("Failed to initialize JPEG decoder");
      return false;
    }

    // 设置默认参数
    centerImage = true;
    scaleToFit = false;
    cacheEnabled = false;

    initialized = true;
    Serial.println("Image Display Manager initialized successfully");

    return true;
  }

  bool ImageDisplayManager::initJPEGDecoder()
  {
    // 设置TJpg_Decoder
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(jpegOutputCallback);

    return true;
  }

  void ImageDisplayManager::setDisplayMode(bool centerImg, bool scaleImg)
  {
    centerImage = centerImg;
    scaleToFit = scaleImg;
  }
  
  ImageFormat ImageDisplayManager::detectImageFormat(const char* filename)
  {
    if (!filename) return ImageFormat::UNKNOWN;

    String name = String(filename);
    name.toLowerCase();

    if (name.endsWith(".jpg") || name.endsWith(".jpeg")) {
      return ImageFormat::JPEG;
    } else if (name.endsWith(".bmp")) {
      return ImageFormat::BMP;
    }

    return ImageFormat::UNKNOWN;
  }

  bool ImageDisplayManager::isImageFile(const char* filename)
  {
    return detectImageFormat(filename) != ImageFormat::UNKNOWN;
  }

  bool ImageDisplayManager::displayImage(const char* filename)
  {
    if (!initialized) {
      Serial.println("ImageDisplayManager not initialized");
      return false;
    }

    if (!filename || strlen(filename) == 0) {
      showImageError("Invalid filename");
      return false;
    }

    Serial.printf("Displaying image: %s\n", filename);

    // 显示加载指示器
    showLoadingIndicator();

    // 检测图片格式并显示
    ImageFormat format = detectImageFormat(filename);
    bool success = false;

    switch (format) {
      case ImageFormat::JPEG:
        success = displayJPEG(filename);
        break;
      case ImageFormat::BMP:
        success = displayBMP(filename);
        break;
      default:
        showImageError("Unsupported format");
        return false;
    }

    // 隐藏加载指示器
    hideLoadingIndicator();

    if (!success) {
      showImageError("Failed to display image");
    }

    return success;
  }

  bool ImageDisplayManager::jpegOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
  {
    // 确保坐标在屏幕范围内
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return 0;

    // 裁剪到屏幕边界
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;

    // 绘制到屏幕
    Display::getTFT().drawRGBBitmap(x, y, bitmap, w, h);

    return 1;
  }
  
  bool ImageDisplayManager::displayJPEG(const char* filename)
  {
    Serial.printf("Displaying JPEG: %s\n", filename);
    
    // 确保文件名以斜杠开头
    String fullPath = filename;
    if (!fullPath.startsWith("/")) {
      fullPath = "/" + fullPath;
    }
    
    // 检查文件是否存在
    if (!LittleFS.exists(fullPath)) {
      Serial.printf("File not found: %s\n", fullPath.c_str());
      return false;
    }
    
    // 清屏
    Display::displayManager.clearScreen();

    // 解码并显示JPEG
    uint16_t w = 0, h = 0;
    TJpgDec.getFsJpgSize(&w, &h, fullPath, LittleFS);

    Serial.printf("JPEG size: %dx%d\n", w, h);

    // 计算显示位置
    int16_t x, y;
    calculateImagePosition(w, h, x, y);

    // 绘制JPEG
    uint16_t result = TJpgDec.drawFsJpg(x, y, fullPath, LittleFS);
    
    if (result == JDR_OK) {
      Serial.println("JPEG displayed successfully");
      return true;
    } else {
      Serial.printf("JPEG decode error: %d\n", result);
      return false;
    }
  }

  bool ImageDisplayManager::displayBMP(const char *filename)
  {
    Serial.printf("Displaying BMP: %s\n", filename);
    
    // 确保文件名以斜杠开头
    String fullPath = filename;
    if (!fullPath.startsWith("/")) {
      fullPath = "/" + fullPath;
    }
    
    // 打开文件
    File bmpFile = LittleFS.open(fullPath, "r");
    if (!bmpFile) {
      Serial.printf("Failed to open BMP file: %s\n", fullPath.c_str());
      return false;
    }
    
    // 读取BMP头
    BMPHeader header;
    if (bmpFile.read((uint8_t*)&header, sizeof(header)) != sizeof(header)) {
      Serial.println("Failed to read BMP header");
      bmpFile.close();
      return false;
    }
    
    // 检查BMP签名
    if (header.signature != 0x4D42) { // "BM"
      Serial.println("Invalid BMP signature");
      bmpFile.close();
      return false;
    }
    
    // 只支持24位BMP
    if (header.bitsPerPixel != 24) {
      Serial.printf("Unsupported BMP format: %d bits per pixel\n", header.bitsPerPixel);
      bmpFile.close();
      return false;
    }
    
    Serial.printf("BMP size: %dx%d, %d bits per pixel\n", 
                  header.width, header.height, header.bitsPerPixel);
    
    // 清屏
    Display::getTFT().fillScreen(ILI9341_BLACK);

    // 计算居中位置
    int16_t startX = (SCREEN_WIDTH - header.width) / 2;
    int16_t startY = (SCREEN_HEIGHT - header.height) / 2;

    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    
    // 计算行字节数（4字节对齐）
    uint32_t rowSize = ((header.width * 3 + 3) / 4) * 4;
    
    // 分配行缓冲区
    uint8_t* rowBuffer = (uint8_t*)malloc(rowSize);
    if (!rowBuffer) {
      Serial.println("Failed to allocate row buffer");
      bmpFile.close();
      return false;
    }
    
    // 跳到图像数据
    bmpFile.seek(header.dataOffset);
    
    // BMP图像是从底部开始存储的，所以从最后一行开始读取
    for (int32_t y = header.height - 1; y >= 0; y--) {
      // 读取一行数据
      if (bmpFile.read(rowBuffer, rowSize) != rowSize) {
        Serial.printf("Failed to read row %d\n", y);
        break;
      }
      
      // 转换并显示像素
      for (uint32_t x = 0; x < header.width && x < SCREEN_WIDTH; x++)
      {
        if (startX + x >= SCREEN_WIDTH || startY + y >= SCREEN_HEIGHT)
        {
          continue;
        }

        // BMP格式是BGR，需要转换为RGB
        uint8_t b = rowBuffer[x * 3];
        uint8_t g = rowBuffer[x * 3 + 1];
        uint8_t r = rowBuffer[x * 3 + 2];
        
        // 转换为16位RGB565格式
        uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

        Display::getTFT().drawPixel(startX + x, startY + y, color);
      }
    }
    
    free(rowBuffer);
    bmpFile.close();
    
    Serial.println("BMP displayed successfully");
    return true;
  }

  // ==================== 辅助函数实现 ====================

  void ImageDisplayManager::calculateImagePosition(uint16_t imgWidth, uint16_t imgHeight,
                                                  int16_t& x, int16_t& y)
  {
    if (centerImage) {
      x = (SCREEN_WIDTH - imgWidth) / 2;
      y = (SCREEN_HEIGHT - imgHeight) / 2;

      // 确保不超出屏幕边界
      if (x < 0) x = 0;
      if (y < 0) y = 0;
    } else {
      x = 0;
      y = 0;
    }
  }

  void ImageDisplayManager::showLoadingIndicator()
  {
    Display::displayManager.showLoadingMessage();
  }

  void ImageDisplayManager::hideLoadingIndicator()
  {
    // 加载指示器会被图片覆盖，无需特别处理
  }

  void ImageDisplayManager::showImageError(const String& error)
  {
    Display::displayManager.showErrorMessage(error);
  }

  void ImageDisplayManager::clearImageArea()
  {
    Display::displayManager.clearScreen();
  }

  uint16_t ImageDisplayManager::rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b)
  {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  // ==================== 便捷函数实现 ====================

  void setup()
  {
    imageDisplayManager.begin();
  }

  bool displayImage(const char* filename)
  {
    return imageDisplayManager.displayImage(filename);
  }

  bool displayJPEG(const char* filename)
  {
    return imageDisplayManager.displayJPEG(filename);
  }

  void showNoImageMessage()
  {
    Display::displayManager.showNoImageMessage();
  }

  void showImageInfo(const char* filename, int index, int total)
  {
    Display::displayManager.showImageInfo(filename, index, total);
  }

  void showLoadingMessage()
  {
    Display::displayManager.showLoadingMessage();
  }

  void showErrorMessage(const String& error)
  {
    Display::displayManager.showErrorMessage(error);
  }

  ImageFormat getImageFormat(const char* filename)
  {
    return imageDisplayManager.detectImageFormat(filename);
  }

  bool isValidImageFile(const char* filename)
  {
    return imageDisplayManager.isImageFile(filename);
  }

  // TJpg_Decoder全局回调函数
  bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
  {
    return ImageDisplayManager::jpegOutputCallback(x, y, w, h, bitmap);
  }
}
