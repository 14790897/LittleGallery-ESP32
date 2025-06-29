#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "secrets.h"

namespace WebServerManager
{
  // ==================== Web服务器管理类 ====================

  class WebServerController
  {
  public:
    // 构造函数
    WebServerController() : server(nullptr), serverRunning(false), fileSystemReady(false), imageCount(0), currentImageIndex(0) {}

    // 析构函数
    ~WebServerController()
    {
      if (server)
      {
        delete server;
        server = nullptr;
      }
    }

    // 初始化和设置
    bool begin();
    void stop();
    bool isRunning() const { return serverRunning; }

    // WiFi管理
    bool connectWiFi(const char* ssid, const char* password);
    bool isWiFiConnected() const { return WiFi.status() == WL_CONNECTED; }
    String getIPAddress() const { return WiFi.localIP().toString(); }

    // 文件系统管理
    bool initFileSystem();
    bool isFileSystemReady() const { return fileSystemReady; }

    // 图片管理
    void scanImages();
    int getImageCount() const { return imageCount; }
    String getCurrentImageName() const;
    int getCurrentImageIndex() const { return currentImageIndex; }

    // 图片控制
    bool nextImage();
    bool previousImage();
    bool setCurrentImage(int index);
    bool deleteImage(const String& filename);

    // API响应
    String getImageListJson() const;
    String getSystemStatusJson() const;

  private:
    AsyncWebServer *server;
    bool serverRunning;
    bool fileSystemReady;

    // 图片管理
    String imageList[MAX_IMAGES];
    int imageCount;
    int currentImageIndex;

    // 路由设置
    void setupRoutes();
    void setupAPIRoutes();
    void setupFileRoutes();

    // 请求处理器
    void handleImageListAPI(AsyncWebServerRequest *request);
    void handleNextImageAPI(AsyncWebServerRequest *request);
    void handlePreviousImageAPI(AsyncWebServerRequest *request);
    void handleSetImageAPI(AsyncWebServerRequest *request);
    void handleDeleteImageAPI(AsyncWebServerRequest *request);
    void handleSystemStatusAPI(AsyncWebServerRequest *request);

    // 文件上传处理
    static void handleFileUpload(AsyncWebServerRequest *request, String filename,
                               size_t index, uint8_t *data, size_t len, bool final);

    // 工具函数
    bool isValidImageFile(const String& filename) const;
    String sanitizeFilename(const String& filename) const;
    bool validateImageUpload(const String& filename, size_t fileSize) const;
    String generateSafeFilename(const String &originalName) const;
  };

  // 全局Web服务器控制器实例
  extern WebServerController webServerController;

  // ==================== 便捷函数接口 ====================

  // 初始化和控制
  bool setup();
  bool isRunning();
  String getIPAddress();

  // 图片管理
  void scanImages();
  int getImageCount();
  String getCurrentImageName();
  int getCurrentImageIndex();

  // 图片控制
  bool nextImage();
  bool previousImage();
  bool setCurrentImage(int index);

  // 状态查询
  String getImageListJson();
  bool isWiFiConnected();

  // 向后兼容的全局变量访问
  extern int currentImageIndex;
  extern String imageList[];
  extern int imageCount;
}

#endif // WEB_SERVER_MANAGER_H
