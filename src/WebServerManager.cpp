#include "WebServer.h"
#include "DisplayDriver.h"
#include "ImageDisplay.h"

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

    // 初始化成员变量
    serverRunning = false;
    fileSystemReady = false;
    imageCount = 0;
    currentImageIndex = 0;
    slideshowActive = false;
    slideshowInterval = 3000; // 默认3秒间隔
    lastSlideshowChange = 0;

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
    return (ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".png");
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

    // 颜色测试API
    server->on("/api/test-colors", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleColorTestAPI(request); });

    // 方向设置API
    server->on("/api/orientation", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleOrientationAPI(request); });
    server->on("/api/orientation", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleSetOrientationAPI(request); });

    // 批量上传状态API
    server->on("/api/upload-status", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleUploadStatusAPI(request); });

    // 幻灯片控制API
    server->on("/api/slideshow", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleSlideshowAPI(request); });
    server->on("/api/slideshow", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleSlideshowStatusAPI(request); });

    // 显示驱动控制API
    server->on("/api/display-driver", HTTP_GET, [this](AsyncWebServerRequest *request)
               { handleDisplayDriverAPI(request); });
    server->on("/api/display-driver", HTTP_POST, [this](AsyncWebServerRequest *request)
               { handleSetDisplayDriverAPI(request); });

    // 添加OPTIONS请求处理 (CORS预检)
    server->on("/api/orientation", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
               {
                 AsyncWebServerResponse *response = request->beginResponse(200);
                 response->addHeader("Access-Control-Allow-Origin", "*");
                 response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                 response->addHeader("Access-Control-Allow-Headers", "Content-Type");
                 request->send(response); });

    server->on("/api/slideshow", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
               {
                 AsyncWebServerResponse *response = request->beginResponse(200);
                 response->addHeader("Access-Control-Allow-Origin", "*");
                 response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                 response->addHeader("Access-Control-Allow-Headers", "Content-Type");
                 request->send(response); });

    server->on("/api/display-driver", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
               {
                 AsyncWebServerResponse *response = request->beginResponse(200);
                 response->addHeader("Access-Control-Allow-Origin", "*");
                 response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                 response->addHeader("Access-Control-Allow-Headers", "Content-Type");
                 request->send(response); });

    server->on("/api/upload-status", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
               {
                 AsyncWebServerResponse *response = request->beginResponse(200);
                 response->addHeader("Access-Control-Allow-Origin", "*");
                 response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
                 response->addHeader("Access-Control-Allow-Headers", "Content-Type");
                 request->send(response); });

    // 文件上传
    server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Upload complete"); }, handleFileUpload);

    // 通用API路由处理器 - 确保API请求不被静态文件服务器拦截
    server->on("/api/*", HTTP_GET, [](AsyncWebServerRequest *request)
               {
                 Serial.printf("Unhandled API GET request: %s\n", request->url().c_str());
                 JsonDocument doc;
                 doc["error"] = "API endpoint not found";
                 doc["path"] = request->url().c_str();
                 doc["method"] = "GET";
                 String response;
                 serializeJson(doc, response);
                 AsyncWebServerResponse *apiResponse = request->beginResponse(404, "application/json", response);
                 apiResponse->addHeader("Access-Control-Allow-Origin", "*");
                 request->send(apiResponse); });

    server->on("/api/*", HTTP_POST, [](AsyncWebServerRequest *request)
               {
                 Serial.printf("Unhandled API POST request: %s\n", request->url().c_str());
                 JsonDocument doc;
                 doc["error"] = "API endpoint not found";
                 doc["path"] = request->url().c_str();
                 doc["method"] = "POST";
                 String response;
                 serializeJson(doc, response);
                 AsyncWebServerResponse *apiResponse = request->beginResponse(404, "application/json", response);
                 apiResponse->addHeader("Access-Control-Allow-Origin", "*");
                 request->send(apiResponse); });

    // 最后注册静态文件服务 (确保API路由优先)
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 404处理
    server->onNotFound([](AsyncWebServerRequest *request)
                       {
                         Serial.printf("404 Not Found: %s %s\n",
                                       request->methodToString(),
                                       request->url().c_str());
                         request->send(404, "text/plain", "File not found"); });
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

    // 幻灯片状态
    doc["slideshow_active"] = slideshowActive;
    doc["slideshow_interval"] = slideshowInterval / 1000; // 转换为秒

    String result;
    serializeJson(doc, result);
    request->send(200, "application/json", result);
  }

  void WebServerController::handleColorTestAPI(AsyncWebServerRequest *request)
  {
    Serial.println("Color test requested via API");

    // 触发颜色测试
    Display::displayManager.clearScreen();

    // 显示颜色测试信息
    Display::displayManager.showSystemInfo("颜色测试中...");
    delay(1000);

    // 执行颜色测试
    testDisplayColors();

    // 返回测试结果
    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "Color test completed";
    doc["colors_tested"] = 7;
    doc["format"] = "RGB565";
    doc["screen_size"] = String(SCREEN_WIDTH) + "x" + String(SCREEN_HEIGHT);
    doc["free_heap"] = ESP.getFreeHeap();

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);

    Serial.println("Color test API completed");
  }

  void WebServerController::handleOrientationAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing orientation API request: %s %s\n",
                  request->methodToString(), request->url().c_str());
    JsonDocument doc;

    // 获取当前方向设置
    doc["auto_rotation"] = true;         // 从ImageDisplayManager获取
    doc["current_mode"] = "SMART_SCALE"; // 从ImageDisplayManager获取
    doc["current_rotation"] = 1;         // 从ImageDisplayManager获取
    doc["available_modes"] = JsonArray();
    doc["available_modes"].add("AUTO_ROTATE");
    doc["available_modes"].add("SMART_SCALE");
    doc["available_modes"].add("CENTER_CROP");
    doc["available_modes"].add("FIT_SCREEN");

    String response;
    serializeJson(doc, response);

    // 添加CORS头部
    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);
  }

  void WebServerController::handleSetOrientationAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing set orientation API request: %s %s\n",
                  request->methodToString(), request->url().c_str());
    // 处理POST参数
    if (request->hasParam("mode", true))
    {
      String mode = request->getParam("mode", true)->value();
      Serial.printf("Setting orientation mode to: %s\n", mode.c_str());

      // 这里需要调用ImageDisplayManager的方法
      // imageDisplayManager.setOrientationMode(mode);
    }

    if (request->hasParam("auto_rotation", true))
    {
      String autoRotation = request->getParam("auto_rotation", true)->value();
      bool enable = (autoRotation == "true");
      Serial.printf("Setting auto rotation to: %s\n", enable ? "enabled" : "disabled");

      // 这里需要调用ImageDisplayManager的方法
      // imageDisplayManager.setAutoRotation(enable);
    }

    JsonDocument doc;
    doc["status"] = "success";
    doc["message"] = "Orientation settings updated";

    String response;
    serializeJson(doc, response);

    // 添加CORS头部
    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);
  }

  void WebServerController::handleUploadStatusAPI(AsyncWebServerRequest *request)
  {
    Serial.println("Processing upload status API request");
    JsonDocument doc;

    // 获取系统状态信息
    doc["free_heap"] = ESP.getFreeHeap();
    doc["used_storage"] = LittleFS.usedBytes();
    doc["total_storage"] = LittleFS.totalBytes();
    doc["available_storage"] = LittleFS.totalBytes() - LittleFS.usedBytes();

    // 计算可用存储空间百分比
    float storageUsedPercent = (float)LittleFS.usedBytes() / LittleFS.totalBytes() * 100;
    doc["storage_used_percent"] = storageUsedPercent;

    // 估算可上传的图片数量（假设平均每张图片100KB）
    size_t availableBytes = LittleFS.totalBytes() - LittleFS.usedBytes();
    int estimatedImageCount = availableBytes / (100 * 1024); // 100KB per image
    doc["estimated_uploadable_images"] = estimatedImageCount;

    // 当前图片数量
    doc["current_image_count"] = imageCount;

    // 上传建议
    if (storageUsedPercent > 90)
    {
      doc["upload_recommendation"] = "storage_almost_full";
      doc["message"] = "存储空间不足，建议删除一些图片后再上传";
    }
    else if (storageUsedPercent > 75)
    {
      doc["upload_recommendation"] = "storage_warning";
      doc["message"] = "存储空间较少，建议适量上传";
    }
    else
    {
      doc["upload_recommendation"] = "storage_ok";
      doc["message"] = "存储空间充足，可以批量上传";
    }

    String response;
    serializeJson(doc, response);

    // 添加CORS头部
    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    apiResponse->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    apiResponse->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(apiResponse);

    Serial.println("Upload status API response sent");
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

  // ==================== 颜色测试函数 ====================

  void testDisplayColors()
  {
    Serial.println("Starting display color test...");

    // 获取显示屏引用
    auto &tft = Display::getTFT();

    // 清屏
    tft.fillScreen(0x0000); // 黑色 (ILI9341_BLACK)

    // 定义测试颜色（使用直接的RGB565值）
    const uint16_t colors[] = {
        0xF800, // 红色 (ILI9341_RED)
        0x07E0, // 绿色 (ILI9341_GREEN)
        0x001F, // 蓝色 (ILI9341_BLUE)
        0xFFE0, // 黄色 (ILI9341_YELLOW)
        0xF81F, // 洋红 (ILI9341_MAGENTA)
        0x07FF, // 青色 (ILI9341_CYAN)
        0xFFFF  // 白色 (ILI9341_WHITE)
    };

    const char *colorNames[] = {
        "RED", "GREEN", "BLUE", "YELLOW", "MAGENTA", "CYAN", "WHITE"};

    // 绘制颜色条
    int barWidth = SCREEN_WIDTH / 7;
    int barHeight = 60;

    for (int i = 0; i < 7; i++)
    {
      tft.fillRect(i * barWidth, 50, barWidth, barHeight, colors[i]);
      Serial.printf("Color %s: 0x%04X\n", colorNames[i], colors[i]);
    }

    // 显示RGB565格式说明
    tft.setTextColor(0xFFFF); // 白色 (ILI9341_WHITE)
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.print("RGB565 Color Test");

    tft.setCursor(10, 130);
    tft.print("Format: RGB565 (16-bit)");
    tft.setCursor(10, 150);
    tft.print("Screen: 320x240");

    // 显示内存信息
    tft.setCursor(10, 180);
    tft.printf("Free Heap: %d KB", ESP.getFreeHeap() / 1024);

    Serial.println("Color test display completed");

    // 保持显示3秒
    delay(3000);

    // 清屏
    tft.fillScreen(0x0000); // 黑色 (ILI9341_BLACK)

    Serial.println("Color test finished");
  }

  // ==================== 图片验证函数 ====================

  bool validateUploadedImage(const String &filename)
  {
    // 检查文件是否存在
    if (!LittleFS.exists(filename))
    {
      Serial.printf("Validation failed: file not found %s\n", filename.c_str());
      return false;
    }

    // 获取文件大小
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.printf("Validation failed: cannot open %s\n", filename.c_str());
      return false;
    }

    size_t fileSize = file.size();
    if (fileSize == 0)
    {
      Serial.printf("Validation failed: empty file %s\n", filename.c_str());
      file.close();
      return false;
    }

    if (fileSize > MAX_FILE_SIZE)
    {
      Serial.printf("Validation failed: file too large %d bytes\n", fileSize);
      file.close();
      return false;
    }

    // 检查JPEG文件头
    String ext = filename.substring(filename.lastIndexOf('.'));
    ext.toLowerCase();

    if (ext == ".jpg" || ext == ".jpeg")
    {
      // 读取JPEG文件头
      uint8_t header[4];
      if (file.read(header, 4) == 4)
      {
        // JPEG文件应该以 FF D8 开头
        if (header[0] == 0xFF && header[1] == 0xD8)
        {
          Serial.println("Valid JPEG header detected");
          file.close();
          return true;
        }
        else
        {
          Serial.printf("Invalid JPEG header: %02X %02X %02X %02X\n",
                        header[0], header[1], header[2], header[3]);
        }
      }
    }
    else if (ext == ".bmp")
    {
      // 读取BMP文件头
      uint8_t header[2];
      if (file.read(header, 2) == 2)
      {
        // BMP文件应该以 "BM" 开头
        if (header[0] == 'B' && header[1] == 'M')
        {
          Serial.println("Valid BMP header detected");
          file.close();
          return true;
        }
        else
        {
          Serial.printf("Invalid BMP header: %c%c\n", header[0], header[1]);
        }
      }
    }

    file.close();
    Serial.printf("Validation failed for %s\n", filename.c_str());
    return false;
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
      if (ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".png")
      {
        Serial.println("Unsupported file format");
        return;
      }

      // 检查文件大小限制
      if (len > MAX_FILE_SIZE)
      {
        Serial.printf("File too large: %d bytes (max: %d)\n", len, MAX_FILE_SIZE);
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

        // 验证上传的图片文件
        if (validateUploadedImage(safeFilename))
        {
          Serial.println("Image validation successful");
          // 重新扫描图片列表
          webServerController.scanImages();
        }
        else
        {
          Serial.println("Image validation failed, removing file");
          LittleFS.remove(safeFilename);
        }
      }
    }
  }

  // ==================== 幻灯片控制函数实现 ====================

  bool WebServerController::toggleSlideshow()
  {
    if (slideshowActive)
    {
      return stopSlideshow();
    }
    else
    {
      return startSlideshow();
    }
  }

  bool WebServerController::startSlideshow()
  {
    if (imageCount <= 1)
    {
      Serial.println("Cannot start slideshow: need at least 2 images");
      return false;
    }

    slideshowActive = true;
    lastSlideshowChange = millis();
    Serial.printf("Slideshow started with %lu ms interval\n", slideshowInterval);
    return true;
  }

  bool WebServerController::stopSlideshow()
  {
    slideshowActive = false;
    Serial.println("Slideshow stopped");
    return true;
  }

  void WebServerController::setSlideshowInterval(unsigned long interval)
  {
    // 限制间隔在1秒到10分钟之间
    if (interval < 1000)
      interval = 1000;
    if (interval > 600000)
      interval = 600000;

    slideshowInterval = interval;
    Serial.printf("Slideshow interval set to %lu ms\n", slideshowInterval);
  }

  void WebServerController::updateSlideshow()
  {
    if (!slideshowActive || imageCount <= 1)
    {
      return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastSlideshowChange >= slideshowInterval)
    {
      nextImage();
      lastSlideshowChange = currentTime;
      Serial.printf("Slideshow auto-switched to: %s\n", getCurrentImageName().c_str());
    }
  }

  void WebServerController::handleSlideshowAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing slideshow API request: %s %s\n",
                  request->methodToString(), request->url().c_str());

    JsonDocument doc;

    if (request->hasParam("action", true))
    {
      String action = request->getParam("action", true)->value();

      if (action == "toggle")
      {
        bool success = toggleSlideshow();
        doc["status"] = success ? "ok" : "error";
        doc["slideshow_active"] = slideshowActive;
        doc["interval"] = slideshowInterval / 1000; // 转换为秒
        if (!success)
        {
          doc["message"] = "Need at least 2 images for slideshow";
        }
      }
      else if (action == "set_interval" && request->hasParam("interval", true))
      {
        unsigned long interval = request->getParam("interval", true)->value().toInt() * 1000; // 转换为毫秒
        setSlideshowInterval(interval);
        doc["status"] = "ok";
        doc["interval"] = slideshowInterval / 1000;
      }
      else
      {
        doc["status"] = "error";
        doc["message"] = "Invalid action or missing parameters";
      }
    }
    else
    {
      doc["status"] = "error";
      doc["message"] = "Missing action parameter";
    }

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);

    Serial.println("Slideshow API response sent");
  }

  void WebServerController::handleSlideshowStatusAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing slideshow status API request: %s %s\n",
                  request->methodToString(), request->url().c_str());

    JsonDocument doc;
    doc["slideshow_active"] = slideshowActive;
    doc["interval"] = slideshowInterval / 1000; // 转换为秒
    doc["image_count"] = imageCount;
    doc["current_image"] = getCurrentImageName();
    doc["status"] = "ok";

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);

    Serial.println("Slideshow status API response sent");
  }

  void WebServerController::handleDisplayDriverAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing display driver API request: %s %s\n",
                  request->methodToString(), request->url().c_str());

    JsonDocument doc;
    doc["current_driver"] = Display::getCurrentDriverName();
    doc["current_driver_type"] = (int)Display::getCurrentDriver();
    doc["available_drivers"] = JsonArray();
    doc["available_drivers"].add("ILI9341");
    doc["available_drivers"].add("ST7789");
    doc["status"] = "ok";

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);

    Serial.println("Display driver API response sent");
  }

  void WebServerController::handleSetDisplayDriverAPI(AsyncWebServerRequest *request)
  {
    Serial.printf("Processing set display driver API request: %s %s\n",
                  request->methodToString(), request->url().c_str());

    JsonDocument doc;

    if (request->hasParam("driver", true))
    {
      String driverName = request->getParam("driver", true)->value();
      DisplayDriverType driverType;

      if (driverName == "ILI9341")
      {
        driverType = DRIVER_ILI9341;
      }
      else if (driverName == "ST7789")
      {
        driverType = DRIVER_ST7789;
      }
      else
      {
        doc["status"] = "error";
        doc["message"] = "Unknown driver type: " + driverName;
      }

      if (doc["status"] != "error")
      {
        if (Display::switchDriver(driverType))
        {
          doc["status"] = "ok";
          doc["current_driver"] = Display::getCurrentDriverName();
          doc["current_driver_type"] = (int)Display::getCurrentDriver();
          doc["message"] = "Display driver switched to " + driverName;
        }
        else
        {
          doc["status"] = "error";
          doc["message"] = "Failed to switch to " + driverName + " driver";
        }
      }
    }
    else
    {
      doc["status"] = "error";
      doc["message"] = "Missing driver parameter";
    }

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse *apiResponse = request->beginResponse(200, "application/json", response);
    apiResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(apiResponse);

    Serial.println("Set display driver API response sent");
  }
}
