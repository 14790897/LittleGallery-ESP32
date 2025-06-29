#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <LittleFS.h>
#include <TJpg_Decoder.h>
#include "ILI9341.h"
#include "secrets.h"

namespace ImageDisplay
{
  // ==================== 图片格式支持 ====================

  enum class ImageFormat {
    UNKNOWN,
    JPEG,
    BMP
  };

  // BMP文件结构
  struct BMPHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPixelsPerMeter;
    uint32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
  } __attribute__((packed));

  // ==================== 方向自适应枚举 ====================

  // 图片方向枚举
  enum class ImageOrientation
  {
    LANDSCAPE, // 横屏 (宽 > 高)
    PORTRAIT,  // 竖屏 (高 > 宽)
    SQUARE     // 正方形 (宽 ≈ 高)
  };

  // 显示模式枚举
  enum class DisplayMode
  {
    AUTO_ROTATE, // 自动旋转屏幕
    SMART_SCALE, // 智能缩放
    CENTER_CROP, // 居中裁剪
    FIT_SCREEN   // 适配屏幕
  };

  // ==================== 图片显示管理类 ====================

  class ImageDisplayManager
  {
  public:
    // 初始化和设置
    bool begin();
    void setDisplayMode(bool centerImage = true, bool scaleToFit = false);
    void setOrientationMode(DisplayMode mode);
    void setAutoRotation(bool enable);

    // 图片格式检测
    ImageFormat detectImageFormat(const char* filename);
    bool isImageFile(const char* filename);

    // 图片显示功能
    bool displayImage(const char* filename);
    bool displayJPEG(const char* filename);
    bool displayBMP(const char* filename);

    // 图片信息获取
    bool getImageDimensions(const char* filename, uint16_t& width, uint16_t& height);
    size_t getImageFileSize(const char* filename);

    // 显示状态管理
    void showLoadingIndicator();
    void hideLoadingIndicator();
    void showImageError(const String& error);
    void clearImageArea();

    // 图片缓存管理（可选功能）
    void enableCache(bool enable) { cacheEnabled = enable; }
    void clearCache();

  private:
    bool initialized;
    bool centerImage;
    bool scaleToFit;
    bool cacheEnabled;

    // 方向自适应配置
    DisplayMode orientationMode;
    bool autoRotationEnabled;
    uint8_t currentRotation;

    // JPEG解码相关
    bool initJPEGDecoder();

  public:
    static bool jpegOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

  private:
    // BMP解码相关
    bool readBMPHeader(File& file, BMPHeader& header);
    bool validateBMPHeader(const BMPHeader& header);
    bool decodeBMP24(File& file, const BMPHeader& header);

    // 图片定位和缩放
    void calculateImagePosition(uint16_t imgWidth, uint16_t imgHeight,
                               int16_t& x, int16_t& y);
    void calculateScaledSize(uint16_t imgWidth, uint16_t imgHeight,
                            uint16_t& newWidth, uint16_t& newHeight);

    // 方向自适应功能
    ImageOrientation detectImageOrientation(uint16_t width, uint16_t height);
    bool shouldRotateScreen(ImageOrientation imgOrientation);
    void applyOptimalRotation(uint16_t imgWidth, uint16_t imgHeight);
    void calculateOptimalScale(uint16_t imgWidth, uint16_t imgHeight,
                               uint8_t &scale, uint16_t &finalWidth, uint16_t &finalHeight);
    void calculateSmartPosition(uint16_t imgWidth, uint16_t imgHeight,
                                int16_t &x, int16_t &y, DisplayMode mode);

    // 颜色转换
    uint16_t rgb888ToRgb565(uint8_t r, uint8_t g, uint8_t b);
    void bgr888ToRgb565(uint8_t b, uint8_t g, uint8_t r, uint16_t& color);
  };

  // 全局图片显示管理器实例
  extern ImageDisplayManager imageDisplayManager;

  // ==================== 便捷函数接口 ====================

  // 初始化
  void setup();

  // 图片显示
  bool displayImage(const char* filename);
  bool displayJPEG(const char* filename);
  bool displayBMP(const char* filename);

  // 状态显示
  void showNoImageMessage();
  void showImageInfo(const char* filename, int index, int total);
  void showLoadingMessage();
  void showErrorMessage(const String& error);

  // 工具函数
  ImageFormat getImageFormat(const char* filename);
  bool isValidImageFile(const char* filename);

  // TJpg_Decoder回调函数（全局函数）
  bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
}

#endif // IMAGE_DISPLAY_H
