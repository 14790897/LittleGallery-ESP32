#include "ImageDisplay.h"
#include "DisplayDriver.h"

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

    // 方向自适应默认设置
    orientationMode = DisplayMode::SMART_SCALE;
    autoRotationEnabled = true;
    currentRotation = 1; // 默认横屏 (320x240)

    initialized = true;
    Serial.println("Image Display Manager initialized successfully");

    return true;
  }

  bool ImageDisplayManager::initJPEGDecoder()
  {
    // 设置TJpg_Decoder for ILI9341 (RGB565)
    TJpgDec.setJpgScale(1);     // 1:1缩放
    TJpgDec.setSwapBytes(false); // 字节序问题修复：TFT_eSPI/Adafruit_ILI9341库通常期望小端序（Little Endian）。
                                 // TJpg_Decoder默认输出小端序，因此不需要交换字节。
                                 // 如果设置为true，会导致颜色字节（如红色和蓝色）颠倒，出现“五彩斑斓”的现象。
    TJpgDec.setCallback(jpegOutputCallback);

    // 设置颜色格式为RGB565
    // TJpg_Decoder默认输出RGB565格式，这正是ILI9341需要的

    Serial.println("JPEG decoder initialized for ILI9341 RGB565");
    Serial.printf("Screen size: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
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
    // 如果图像的y坐标超出了屏幕底部，则停止解码
    if (y >= Display::displayManager.getHeight())
      return 0;

    // drawRGBBitmap函数将自动处理屏幕边界的图像块裁剪
    Display::displayManager.getGFX().drawRGBBitmap(x, y, bitmap, w, h);

    // 返回1以继续解码下一个块
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

    // 获取文件大小
    File file = LittleFS.open(fullPath, "r");
    if (!file)
    {
        Serial.printf("Failed to open file: %s\n", fullPath.c_str());
        return false;
    }
    size_t fileSize = file.size();
    file.close();

    Serial.printf("File size: %d bytes\n", fileSize);

    // 检查可用内存
    size_t freeHeap = ESP.getFreeHeap();
    Serial.printf("Free heap: %d bytes\n", freeHeap);

    // 清屏
    Display::displayManager.clearScreen();

    // 获取JPEG尺寸
    uint16_t w = 0, h = 0;
    uint16_t sizeResult = TJpgDec.getFsJpgSize(&w, &h, fullPath, LittleFS);

    if (sizeResult != JDR_OK)
    {
        Serial.printf("Failed to get JPEG size, error: %d\n", sizeResult);
        showImageError("JPEG格式错误");
        return false;
    }

    Serial.printf("JPEG size: %dx%d\n", w, h);

    // 应用方向自适应
    applyOptimalRotation(w, h);

    // --- 智能缩放与定位 ---
    // 1. 计算最佳缩放比例以适应屏幕
    uint8_t scale = 1;
    uint16_t finalWidth, finalHeight;
    calculateOptimalScale(w, h, scale, finalWidth, finalHeight);

    // 2. 将计算出的缩放比例应用到JPEG解码器
    TJpgDec.setJpgScale(scale);
    Serial.printf("Applied scale factor: %d\n", scale);

    // 3. 根据最终缩放后的尺寸计算居中位置
    int16_t x, y;
    calculateSmartPosition(finalWidth, finalHeight, x, y, orientationMode);
    Serial.printf("Displaying at (%d, %d) with final size %dx%d\n", x, y, finalWidth, finalHeight);

    // 4. 绘制JPEG
    uint16_t result = TJpgDec.drawFsJpg(x, y, fullPath, LittleFS);

    // 恢复缩放设置
    TJpgDec.setJpgScale(1);

    if (result == JDR_OK) {
        Serial.println("JPEG displayed successfully");
        // 在图片底部显示文件名
        Display::displayManager.drawFileName(filename);
        return true;
    } else {
        Serial.printf("JPEG decode error: %d\n", result);

        // 提供更详细的错误信息
        String errorMsg = "解码失败";
        switch (result)
        {
        case 1:
            errorMsg = "内存不足或格式错误";
            break;
        case 2:
            errorMsg = "文件格式不支持";
            break;
        case 3:
            errorMsg = "文件损坏";
            break;
        case 4:
            errorMsg = "参数错误";
            break;
        default:
            errorMsg = "未知错误 " + String(result);
            break;
        }

        showImageError(errorMsg);
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
    Display::displayManager.fillScreen(0x0000); // 黑色

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

            Display::displayManager.drawPixel(startX + x, startY + y, color);
        }
    }
    
    free(rowBuffer);
    bmpFile.close();
    
    Serial.println("BMP displayed successfully");
    // 在图片底部显示文件名
    Display::displayManager.drawFileName(filename);
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

  // ==================== 方向自适应功能实现 ====================

  void ImageDisplayManager::setOrientationMode(DisplayMode mode)
  {
    orientationMode = mode;
    Serial.printf("Orientation mode set to: %d\n", (int)mode);
  }

  void ImageDisplayManager::setAutoRotation(bool enable)
  {
    autoRotationEnabled = enable;
    Serial.printf("Auto rotation %s\n", enable ? "enabled" : "disabled");
  }

  ImageOrientation ImageDisplayManager::detectImageOrientation(uint16_t width, uint16_t height)
  {
    float aspectRatio = (float)width / (float)height;

    if (aspectRatio > 1.2f)
    {
      return ImageOrientation::LANDSCAPE;
    }
    else if (aspectRatio < 0.8f)
    {
      return ImageOrientation::PORTRAIT;
    }
    else
    {
      return ImageOrientation::SQUARE;
    }
  }

  bool ImageDisplayManager::shouldRotateScreen(ImageOrientation imgOrientation)
  {
    if (!autoRotationEnabled)
    {
      return false;
    }

    // 当前屏幕是横屏 (320x240)
    // 如果图片是竖屏，考虑旋转屏幕
    return (imgOrientation == ImageOrientation::PORTRAIT);
  }

  void ImageDisplayManager::applyOptimalRotation(uint16_t imgWidth, uint16_t imgHeight)
  {
    ImageOrientation orientation = detectImageOrientation(imgWidth, imgHeight);

    Serial.printf("Image orientation: %s (%dx%d)\n",
                  orientation == ImageOrientation::LANDSCAPE ? "LANDSCAPE" : orientation == ImageOrientation::PORTRAIT ? "PORTRAIT"
                                                                                                                       : "SQUARE",
                  imgWidth, imgHeight);

    if (shouldRotateScreen(orientation))
    {
      // 旋转到竖屏模式 (240x320)
      currentRotation = 0;
      Display::displayManager.setRotation(0);
      Serial.println("Screen rotated to portrait mode (240x320)");
    }
    else
    {
      // 保持横屏模式 (320x240)
      currentRotation = 1;
      Display::displayManager.setRotation(1);
      Serial.println("Screen kept in landscape mode (320x240)");
    }
  }

  void ImageDisplayManager::calculateOptimalScale(uint16_t imgWidth, uint16_t imgHeight,
                                                  uint8_t &scale, uint16_t &finalWidth, uint16_t &finalHeight)
{
    // 获取当前屏幕尺寸
    uint16_t screenW = (currentRotation == 0) ? 240 : 320;
    uint16_t screenH = (currentRotation == 0) ? 320 : 240;

    Serial.printf("Screen size: %dx%d, Image size: %dx%d\n", screenW, screenH, imgWidth, imgHeight);

    float img_aspect = (float)imgWidth / (float)imgHeight;
    float screen_aspect = (float)screenW / (float)screenH;

    // 默认模式 SMART_SCALE 和 FIT_SCREEN 都采用“适应”逻辑，确保整个图片可见
    if (orientationMode == DisplayMode::SMART_SCALE || orientationMode == DisplayMode::FIT_SCREEN)
    {
        // 适配屏幕：缩放以使整个图像可见（可能留有黑边）
        // 我们需要根据更长的边来缩放
        if (img_aspect > screen_aspect) {
            // 图片比屏幕“宽”，因此按宽度缩放
            scale = 1;
            while ((imgWidth / scale) > screenW && scale < 8) scale *= 2;
        } else {
            // 图片比屏幕“高”，因此按高度缩放
            scale = 1;
            while ((imgHeight / scale) > screenH && scale < 8) scale *= 2;
        }
    }
    else // CENTER_CROP (填充屏幕)
    {
        // 填充逻辑：缩放以填充整个屏幕（可能裁剪图片）
        float scale_f;
        if (img_aspect > screen_aspect) {
            // 图片更“宽”，按高度缩放以填充
            scale_f = (float)imgHeight / screenH;
        } else {
            // 图片更“高”，按宽度缩放以填充
            scale_f = (float)imgWidth / screenW;
        }

        if (scale_f >= 8.0) scale = 8;
        else if (scale_f >= 4.0) scale = 4;
        else if (scale_f >= 2.0) scale = 2;
        else scale = 1;
    }

    finalWidth = imgWidth / scale;
    finalHeight = imgHeight / scale;

    Serial.printf("Optimal scale: %d, Final size: %dx%d\n", scale, finalWidth, finalHeight);
}

  void ImageDisplayManager::calculateSmartPosition(uint16_t imgWidth, uint16_t imgHeight,
                                                   int16_t &x, int16_t &y, DisplayMode mode)
  {
    // 获取当前屏幕尺寸
    uint16_t screenW = (currentRotation == 0) ? 240 : 320;
    uint16_t screenH = (currentRotation == 0) ? 320 : 240;

    switch (mode)
    {
    case DisplayMode::SMART_SCALE:
    case DisplayMode::FIT_SCREEN:
      // 居中显示
      x = (screenW - imgWidth) / 2;
      y = (screenH - imgHeight) / 2;
      break;

    case DisplayMode::CENTER_CROP:
      // 居中裁剪
      x = (screenW - imgWidth) / 2;
      y = (screenH - imgHeight) / 2;
      break;

    case DisplayMode::AUTO_ROTATE:
      // 自动旋转模式下的居中
      x = (screenW - imgWidth) / 2;
      y = (screenH - imgHeight) / 2;
      break;
    }

    // 对于FIT_SCREEN模式，确保图像的左上角不会在屏幕外，以避免图像移出视图
    if (mode == DisplayMode::FIT_SCREEN) {
      if (x < 0) x = 0;
      if (y < 0) y = 0;
    }
    // 对于其他模式（如SMART_SCALE, CENTER_CROP），允许负坐标以实现裁剪

    Serial.printf("Smart position: (%d, %d) for %dx%d image on %dx%d screen\n",
                  x, y, imgWidth, imgHeight, screenW, screenH);
  }
}
