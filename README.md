# Little Gallery ESP32

一个基于ESP32-C3和ILI9341显示屏的图片库系统，支持通过Web界面上传和切换图片。

## 功能特性

- 📱 Web界面上传图片（支持JPG、JPEG、BMP格式）
- 🖼️ 自动图片显示和切换
- 🌐 WiFi连接和Web服务器
- 💾 LittleFS文件系统存储
- 🎮 Web界面控制图片切换
- 📊 实时图片列表管理

## 硬件要求

- ESP32-C3开发板
- ILI9341 320x240 TFT显示屏
- SPI连接

### 引脚连接

| ILI9341 | ESP32-C3 |
|---------|----------|
| VCC     | 3.3V     |
| GND     | GND      |
| CS      | GPIO 7   |
| DC      | GPIO 6   |
| RST     | GPIO 10  |
| SCK     | GPIO 4   |
| MOSI    | GPIO 5   |

## 软件配置

### 1. WiFi配置

在 `include/secrets.h` 文件中修改WiFi配置：

```cpp
const char* ssid = "YourWiFiSSID";        // 修改为您的WiFi名称
const char* password = "YourWiFiPassword"; // 修改为您的WiFi密码
```

### 2. 编译和上传

1. 编译并上传固件：
```bash
pio run --target upload
```

2. 上传Web文件到LittleFS：
```bash
pio run --target uploadfs
```

3. 打开串口监视器查看IP地址

### 3. 使用Web界面

1. 在浏览器中访问显示的IP地址
2. 享受现代化的Web界面：
   - 拖拽上传图片文件
   - 实时图片切换控制
   - 批量图片管理
   - 系统状态监控

## 支持的图片格式

- **JPEG/JPG**: 使用TJpg_Decoder库解码，支持各种JPEG格式
- **BMP**: 支持24位BMP格式，自动转换为RGB565显示

## Web界面功能

- **上传图片**: 拖拽或选择文件上传
- **图片控制**: 上一张/下一张按钮
- **图片列表**: 显示所有已上传的图片
- **选择图片**: 点击列表中的图片直接跳转
- **删除图片**: 删除不需要的图片
- **实时更新**: 自动刷新图片列表和状态

## 文件结构

```
LittleGallery-ESP32/
├── include/               # 头文件目录
│   ├── Config.h           # 系统配置文件
│   ├── ILI9341.h          # 显示屏管理头文件
│   ├── WebServer.h        # Web服务器管理头文件
│   └── ImageDisplay.h     # 图片显示管理头文件
├── src/                   # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── ILI9341Display.cpp # 显示屏管理实现
│   ├── WebServerManager.cpp # Web服务器管理实现
│   └── ImageDisplay.cpp   # 图片显示管理实现
├── data/                  # Web前端文件目录（上传到LittleFS）
│   ├── index.html         # 主页面
│   ├── style.css          # 样式文件
│   └── app.js             # JavaScript应用
├── scripts/               # 构建脚本目录
│   └── build_web.py       # Web文件构建脚本
├── platformio.ini         # PlatformIO配置
├── README.md             # 详细说明文档
├── QUICK_START.md        # 快速开始指南
└── WEB_FILES_GUIDE.md    # Web文件管理指南
```

## 模块化设计

项目采用模块化设计，每个模块负责特定功能：

### 1. 配置模块 (Config.h)
- 集中管理所有系统配置
- WiFi设置、硬件引脚、系统参数等
- 便于修改和维护

### 2. 显示屏模块 (ILI9341Display)
- `Display::ILI9341Manager` 类管理显示屏
- 提供文本显示、状态显示、错误显示等功能
- 支持多种显示模式和布局

### 3. 图片显示模块 (ImageDisplay)
- `ImageDisplay::ImageDisplayManager` 类管理图片显示
- 支持JPEG和BMP格式
- 自动居中、缩放等功能

### 4. Web服务器模块 (WebServerManager)
- `WebServerManager::WebServerController` 类管理Web服务
- 文件上传、图片管理、API接口
- 静态文件服务（从LittleFS提供Web文件）

### 5. Web前端模块 (data目录)
- **现代化界面** - 响应式设计，支持移动设备
- **拖拽上传** - 支持多文件拖拽上传
- **实时控制** - 图片切换、幻灯片播放
- **批量管理** - 图片选择、批量删除
- **系统监控** - WiFi状态、存储空间、运行时间

## 库依赖

项目自动管理以下库依赖：

- ESP32Async/ESPAsyncWebServer
- ESP32Async/AsyncTCP
- ArduinoJson
- adafruit/Adafruit GFX Library
- adafruit/Adafruit ILI9341
- bodmer/TJpg_Decoder

## 使用说明

1. **首次启动**: 设备会显示WiFi连接状态和IP地址
2. **上传图片**: 通过Web界面上传图片到设备
3. **自动显示**: 图片会自动在屏幕上显示
4. **切换控制**: 使用Web界面或等待自动切换
5. **管理图片**: 通过Web界面管理图片列表

## 故障排除

### 显示问题
- 检查SPI连接是否正确
- 确认引脚配置是否匹配

### WiFi连接问题
- 检查WiFi名称和密码是否正确
- 确认WiFi信号强度

### 图片显示问题
- 确认图片格式是否支持
- 检查文件大小是否合适
- 查看串口输出的错误信息

## 技术特点

- **异步Web服务器**: 高效处理多个并发请求
- **LittleFS文件系统**: 可靠的文件存储
- **硬件加速**: 使用SPI硬件加速显示
- **内存优化**: 逐行处理大图片，避免内存溢出
- **实时更新**: 自动检测图片变化并更新显示

## 扩展功能

可以考虑添加的功能：
- 图片幻灯片自动播放
- 图片缩放和旋转
- 更多图片格式支持
- 图片元数据显示
- 远程图片下载

## 许可证

本项目采用MIT许可证。

## 演示视频

- [B站演示视频（Little Gallery ESP32）](https://www.bilibili.com/video/BV1BzgZzMEYn/)
