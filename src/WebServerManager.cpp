#include "WebServer.h"

namespace WebServerManager
{
  // 全局Web服务器控制器实例
  WebServerController webServerController;
  
  // 向后兼容的全局变量
  int currentImageIndex = 0;
  String imageList[MAX_IMAGES];
  int imageCount = 0;
  
  // ==================== WebServerController 类实现 ====================
  
  bool WebServerController::begin()
  {
    Serial.println("Initializing Web Server Controller...");
    
    // 初始化文件系统
    if (!initFileSystem()) {
      Serial.println("Failed to initialize file system");
      return false;
    }
    
    // 连接WiFi
    if (!connectWiFi(WIFI_SSID, WIFI_PASSWORD)) {
      Serial.println("Failed to connect to WiFi");
      return false;
    }
    
    // 扫描现有图片
    scanImages();
    
    // 初始化Web服务器
    server = new AsyncWebServer(WEB_SERVER_PORT);
    if (!server)
    {
      Serial.println("Failed to create web server");
      return false;
    }
    setupRoutes();
    
    // 启动服务器
    server->begin();
    serverRunning = true;
    
    Serial.println("Web server started successfully");
    Serial.printf("Server running on: http://%s\n", getIPAddress().c_str());
    
    return true;
  }

  void WebServerController::stop()
  {
    if (server && serverRunning)
    {
      serverRunning = false;
      Serial.println("Web server stopped");
    }
  }

  bool WebServerController::connectWiFi(const char* ssid, const char* password)
  {
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.printf("WiFi connected! IP address: %s\n", WiFi.localIP().toString().c_str());
      return true;
    } else {
      Serial.println();
      Serial.println("Failed to connect to WiFi");
      return false;
    }
  }
  
  bool WebServerController::initFileSystem()
  {
    if (!LittleFS.begin(true)) {
      Serial.println("LittleFS Mount Failed");
      fileSystemReady = false;
      return false;
    }
    
    Serial.println("LittleFS mounted successfully");
    fileSystemReady = true;
    return true;
  }
  
  void WebServerController::scanImages()
  {
    imageCount = 0;
    currentImageIndex = 0;
    
    if (!fileSystemReady) {
      Serial.println("File system not ready for scanning");
      return;
    }
    
    File root = LittleFS.open("/");
    if (!root) {
      Serial.println("Failed to open root directory");
      return;
    }
    
    File file = root.openNextFile();
    while (file && imageCount < MAX_IMAGES) {
      String fileName = file.name();
      
      if (isValidImageFile(fileName)) {
        imageList[imageCount] = fileName;
        imageCount++;
        Serial.printf("Found image: %s\n", fileName.c_str());
      }
      
      file = root.openNextFile();
    }
    
    Serial.printf("Total images found: %d\n", imageCount);
    
    // 更新全局变量（向后兼容）
    ::WebServerManager::imageCount = imageCount;
    ::WebServerManager::currentImageIndex = currentImageIndex;
    for (int i = 0; i < imageCount; i++) {
      ::WebServerManager::imageList[i] = imageList[i];
    }
  }
  
  bool WebServerController::isValidImageFile(const String& filename) const
  {
    String ext = filename.substring(filename.lastIndexOf('.'));
    ext.toLowerCase();
    return (ext == ".jpg" || ext == ".jpeg" || ext == ".bmp");
  }

  String WebServerController::generateSafeFilename(const String &originalName) const
  {
    // 获取文件扩展名
    int lastDotIndex = originalName.lastIndexOf('.');
    String extension = (lastDotIndex > -1) ? originalName.substring(lastDotIndex) : "";
    String nameWithoutExt = (lastDotIndex > -1) ? originalName.substring(0, lastDotIndex) : originalName;

    // 清理文件名：只保留字母、数字、下划线和连字符
    String cleanName = "";
    for (int i = 0; i < nameWithoutExt.length(); i++)
    {
      char c = nameWithoutExt.charAt(i);
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-')
      {
        cleanName += c;
      }
      else
      {
        cleanName += '_';
      }
    }

    // 移除连续的下划线
    while (cleanName.indexOf("__") >= 0)
    {
      cleanName.replace("__", "_");
    }

    // 移除开头和结尾的下划线
    while (cleanName.startsWith("_"))
    {
      cleanName = cleanName.substring(1);
    }
    while (cleanName.endsWith("_"))
    {
      cleanName = cleanName.substring(0, cleanName.length() - 1);
    }

    // 如果清理后的名称为空，使用默认名称
    if (cleanName.length() == 0)
    {
      cleanName = "image";
    }

    // 计算最大允许的名称长度（总长度限制25 - 扩展名长度 - 时间戳长度）
    int maxNameLength = 25 - extension.length() - 7; // 7 = '_' + 6位时间戳

    // 如果名称太长，截断
    if (cleanName.length() > maxNameLength)
    {
      cleanName = cleanName.substring(0, maxNameLength);
    }

    // 添加时间戳确保唯一性
    unsigned long timestamp = millis() % 1000000; // 取6位数字
    String finalName = cleanName + "_" + String(timestamp) + extension;

    // 最终检查长度
    if (finalName.length() > 25)
    {
      int availableLength = 25 - extension.length() - 7;
      if (availableLength > 0)
      {
        cleanName = cleanName.substring(0, availableLength);
        finalName = cleanName + "_" + String(timestamp) + extension;
      }
      else
      {
        finalName = "img_" + String(timestamp) + extension;
      }
    }

    return finalName;
  }

  String WebServerController::getCurrentImageName() const
  {
    if (imageCount > 0 && currentImageIndex >= 0 && currentImageIndex < imageCount) {
      return imageList[currentImageIndex];
    }
    return "No image";
  }
  
  bool WebServerController::nextImage()
  {
    if (imageCount > 0) {
      currentImageIndex = (currentImageIndex + 1) % imageCount;
      ::WebServerManager::currentImageIndex = currentImageIndex; // 更新全局变量
      Serial.printf("Switched to next image: %s (index: %d)\n", 
                   getCurrentImageName().c_str(), currentImageIndex);
      return true;
    }
    return false;
  }
  
  bool WebServerController::previousImage()
  {
    if (imageCount > 0) {
      currentImageIndex = (currentImageIndex - 1 + imageCount) % imageCount;
      ::WebServerManager::currentImageIndex = currentImageIndex; // 更新全局变量
      Serial.printf("Switched to previous image: %s (index: %d)\n", 
                   getCurrentImageName().c_str(), currentImageIndex);
      return true;
    }
    return false;
  }
  
  bool WebServerController::setCurrentImage(int index)
  {
    if (index >= 0 && index < imageCount) {
      currentImageIndex = index;
      ::WebServerManager::currentImageIndex = currentImageIndex; // 更新全局变量
      Serial.printf("Set current image: %s (index: %d)\n", 
                   getCurrentImageName().c_str(), currentImageIndex);
      return true;
    }
    return false;
  }
  
  String WebServerController::getImageListJson() const
  {
    JsonDocument doc;
    JsonArray images = doc["images"].to<JsonArray>();

    for (int i = 0; i < imageCount; i++) {
      images.add(imageList[i]);
    }
    
    doc["current"] = currentImageIndex;
    doc["total"] = imageCount;
    doc["status"] = "ok";
    
    String result;
    serializeJson(doc, result);
    return result;
  }

  String WebServerController::getSystemStatusJson() const
  {
    JsonDocument doc;

    doc["wifi"] = isWiFiConnected();
    doc["ip"] = getIPAddress();
    doc["uptime"] = millis() / 1000; // 运行时间（秒）

    // 获取存储信息
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    doc["storage"] = String(usedBytes / 1024) + "KB / " + String(totalBytes / 1024) + "KB";

    // 图片统计
    doc["imageCount"] = imageCount;
    doc["currentImage"] = getCurrentImageName();
    doc["currentIndex"] = currentImageIndex;

    String result;
    serializeJson(doc, result);
    return result;
  }

  void WebServerController::setupRoutes()
  {
    // 先注册API路由
    server->on("/api/images", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleImageListAPI(request); });

    server->on("/api/next", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleNextImageAPI(request); });

    server->on("/api/previous", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handlePreviousImageAPI(request); });

    server->on("/api/setimage", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleSetImageAPI(request); });

    server->on("/api/delete", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleDeleteImageAPI(request); });

    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleSystemStatusAPI(request); });

    // 文件上传
    server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Upload complete"); }, handleFileUpload);

    // 再注册静态文件服务
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 404处理
    server->onNotFound([](AsyncWebServerRequest *request)
                       { request->send(404, "text/plain", "File not found"); });
  }
  

  
  void WebServerController::handleImageListAPI(AsyncWebServerRequest *request)
  {
    request->send(200, "application/json", getImageListJson());
  }
  
  void WebServerController::handleNextImageAPI(AsyncWebServerRequest *request)
  {
    if (nextImage()) {
      request->send(200, "application/json", 
                   "{\"status\":\"ok\",\"current\":\"" + getCurrentImageName() + "\"}");
    } else {
      request->send(400, "application/json", 
                   "{\"status\":\"error\",\"message\":\"No images available\"}");
    }
  }
  
  void WebServerController::handlePreviousImageAPI(AsyncWebServerRequest *request)
  {
    if (previousImage()) {
      request->send(200, "application/json", 
                   "{\"status\":\"ok\",\"current\":\"" + getCurrentImageName() + "\"}");
    } else {
      request->send(400, "application/json", 
                   "{\"status\":\"error\",\"message\":\"No images available\"}");
    }
  }
  
  void WebServerController::handleSetImageAPI(AsyncWebServerRequest *request)
  {
    if (request->hasParam("index", true)) {
      int index = request->getParam("index", true)->value().toInt();
      if (setCurrentImage(index)) {
        request->send(200, "application/json",
                     "{\"status\":\"ok\",\"current\":\"" + getCurrentImageName() + "\"}");
      } else {
        request->send(400, "application/json",
                     "{\"status\":\"error\",\"message\":\"Invalid image index\"}");
      }
    } else {
      request->send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Missing index parameter\"}");
    }
  }

  void WebServerController::handleDeleteImageAPI(AsyncWebServerRequest *request)
  {
    if (request->hasParam("filename", true)) {
      String filename = request->getParam("filename", true)->value();
      if (deleteImage(filename)) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
      } else {
        request->send(500, "application/json",
                     "{\"status\":\"error\",\"message\":\"Failed to delete file\"}");
      }
    } else {
      request->send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Missing filename parameter\"}");
    }
  }

  void WebServerController::handleSystemStatusAPI(AsyncWebServerRequest *request)
  {
    JsonDocument doc;

    doc["wifi"] = isWiFiConnected();
    doc["ip"] = getIPAddress();
    doc["uptime"] = millis() / 1000; // 运行时间（秒）

    // 获取存储信息
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    doc["storage"] = String(usedBytes / 1024) + "KB / " + String(totalBytes / 1024) + "KB";

    // 图片统计
    doc["imageCount"] = imageCount;
    doc["currentImage"] = getCurrentImageName();
    doc["currentIndex"] = currentImageIndex;

    String result;
    serializeJson(doc, result);
    request->send(200, "application/json", result);
  }

  bool WebServerController::deleteImage(const String& filename)
  {
    String fullPath = filename;
    if (!fullPath.startsWith("/")) {
      fullPath = "/" + fullPath;
    }

    if (LittleFS.remove(fullPath)) {
      Serial.printf("Deleted image: %s\n", fullPath.c_str());
      scanImages(); // 重新扫描图片列表
      return true;
    } else {
      Serial.printf("Failed to delete image: %s\n", fullPath.c_str());
      return false;
    }
  }
  
  // ==================== 便捷函数实现 ====================
  
  bool setup()
  {
    return webServerController.begin();
  }
  
  bool isRunning()
  {
    return webServerController.isRunning();
  }
  
  String getIPAddress()
  {
    return webServerController.getIPAddress();
  }
  
  void scanImages()
  {
    webServerController.scanImages();
  }
  
  int getImageCount()
  {
    return webServerController.getImageCount();
  }
  
  String getCurrentImageName()
  {
    return webServerController.getCurrentImageName();
  }
  
  int getCurrentImageIndex()
  {
    return webServerController.getCurrentImageIndex();
  }
  
  bool nextImage()
  {
    return webServerController.nextImage();
  }
  
  bool previousImage()
  {
    return webServerController.previousImage();
  }
  
  bool setCurrentImage(int index)
  {
    return webServerController.setCurrentImage(index);
  }
  
  String getImageListJson()
  {
    return webServerController.getImageListJson();
  }
  
  bool isWiFiConnected()
  {
    return webServerController.isWiFiConnected();
  }

  // ==================== 静态函数实现 ====================

  void WebServerController::handleFileUpload(AsyncWebServerRequest *request, String filename,
                                            size_t index, uint8_t *data, size_t len, bool final)
  {
    static File uploadFile;
    static String safeFilename;

    if (!index) {
      // 开始上传
      Serial.printf("Upload start: %s\n", filename.c_str());

      // 生成安全的文件名
      safeFilename = webServerController.generateSafeFilename(filename);
      Serial.printf("Safe filename: %s\n", safeFilename.c_str());

      // 确保文件名以斜杠开头
      if (!safeFilename.startsWith("/"))
      {
        safeFilename = "/" + safeFilename;
      }

      // 检查文件扩展名
      String ext = safeFilename.substring(safeFilename.lastIndexOf('.'));
      ext.toLowerCase();
      if (ext != ".jpg" && ext != ".jpeg" && ext != ".bmp") {
        Serial.println("Unsupported file format");
        return;
      }

      uploadFile = LittleFS.open(safeFilename, "w");
      if (!uploadFile) {
        Serial.printf("Failed to open file for writing: %s\n", safeFilename.c_str());
        Serial.printf("Used: %d, Total: %d\n", LittleFS.usedBytes(), LittleFS.totalBytes());
        Serial.printf("Filename length: %d\n", safeFilename.length());
        return;
      }
    }

    if (uploadFile) {
      // 写入数据
      uploadFile.write(data, len);
    }

    if (final) {
      // 上传完成
      if (uploadFile) {
        uploadFile.close();
        Serial.printf("Upload complete: %s (%d bytes)\n", safeFilename.c_str(), index + len);

        // 重新扫描图片列表
        webServerController.scanImages();
      }
    }
  }


}
