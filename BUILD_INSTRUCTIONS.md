# 构建说明

## 🚀 快速构建步骤

### 1. 配置WiFi
编辑 `include/Config.h` 文件：
```cpp
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

### 2. 清理并重新构建
```bash
# 清理之前的构建
pio run --target clean

# 重新安装依赖
pio lib install

# 编译项目
pio run

# 上传固件
pio run --target upload

# 上传文件系统（Web文件）
pio run --target uploadfs
```

### 3. 监控串口输出
```bash
pio device monitor
```

## 🔧 解决的编译问题

### 1. ArduinoJson 版本兼容性
- **问题**: `DynamicJsonDocument` 已弃用
- **解决**: 使用新的 `JsonDocument` API
- **更改**: 
  - `DynamicJsonDocument doc(size)` → `JsonDocument doc`
  - `doc.createNestedArray("key")` → `doc["key"].to<JsonArray>()`

### 2. ESPAsyncWebServer 兼容性
- **问题**: 库版本冲突
- **解决**: 使用GitHub直接源码
- **更改**: 使用官方仓库链接而不是PlatformIO Registry

### 3. 函数声明问题
- **问题**: `updateDisplayedImage()` 未声明
- **解决**: 添加函数前向声明

## 📦 库依赖

项目使用以下库：
- **ESPAsyncWebServer** - Web服务器
- **AsyncTCP** - 异步TCP支持
- **ArduinoJson v7** - JSON处理
- **Adafruit GFX** - 图形库
- **Adafruit ILI9341** - 显示屏驱动
- **TJpg_Decoder** - JPEG解码

## 🐛 常见问题

### 编译错误
1. **库依赖问题**
   ```bash
   pio lib install --force
   ```

2. **缓存问题**
   ```bash
   pio run --target clean
   rm -rf .pio
   ```

3. **平台问题**
   ```bash
   pio platform install espressif32 --force
   ```

### 上传问题
1. **端口权限**
   - Windows: 检查设备管理器
   - Linux/Mac: `sudo chmod 666 /dev/ttyUSB0`

2. **文件系统上传失败**
   - 确保 `data/` 目录存在
   - 检查文件大小不超过分区限制

### 运行时问题
1. **WiFi连接失败**
   - 检查SSID和密码
   - 确认2.4GHz网络

2. **Web界面无法访问**
   - 检查IP地址
   - 确认防火墙设置

## 📁 文件结构检查

确保以下文件存在：
```
LittleGallery-ESP32/
├── data/
│   ├── index.html     ✓
│   ├── style.css      ✓
│   └── app.js         ✓
├── include/
│   ├── Config.h       ✓
│   ├── ILI9341.h      ✓
│   ├── WebServer.h    ✓
│   └── ImageDisplay.h ✓
├── src/
│   ├── main.cpp       ✓
│   ├── ILI9341Display.cpp ✓
│   ├── WebServerManager.cpp ✓
│   └── ImageDisplay.cpp ✓
└── platformio.ini     ✓
```

## 🎯 验证步骤

1. **编译成功** - 无错误和警告
2. **上传成功** - 固件和文件系统都上传
3. **串口输出** - 显示WiFi连接和IP地址
4. **显示屏显示** - 显示启动画面和状态
5. **Web访问** - 浏览器能打开界面
6. **功能测试** - 上传图片和切换功能正常

## 🔄 更新流程

### 代码更新
1. 修改源代码
2. `pio run --target upload`

### Web文件更新
1. 修改 `data/` 目录文件
2. `pio run --target uploadfs`

### 完整更新
1. `pio run --target clean`
2. `pio run --target upload`
3. `pio run --target uploadfs`

## 📞 获取帮助

如果遇到问题：
1. 检查串口输出的错误信息
2. 确认硬件连接正确
3. 验证WiFi配置
4. 查看浏览器开发者工具
5. 参考项目文档
