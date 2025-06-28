# 快速开始指南

## 1. 硬件连接

将ILI9341显示屏按以下方式连接到ESP32-C3：

```
ILI9341    ESP32-C3
VCC    ->  3.3V
GND    ->  GND
CS     ->  GPIO 7
DC     ->  GPIO 6
RST    ->  GPIO 10
SCK    ->  GPIO 4 (SPI CLK)
MOSI   ->  GPIO 5 (SPI MOSI)
```

## 2. 配置WiFi

编辑 `include/Config.h` 文件，修改WiFi设置：

```cpp
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

## 3. 编译和上传

1. 打开PlatformIO
2. 编译项目：`pio run`
3. 上传到设备：`pio run --target upload`
4. 打开串口监视器：`pio device monitor`

## 4. 使用Web界面

1. 设备启动后，屏幕会显示IP地址
2. 在浏览器中打开该IP地址
3. 上传图片文件（JPG、JPEG、BMP）
4. 使用控制按钮切换图片

## 5. 支持的图片格式

- **JPEG/JPG**: 推荐格式，压缩率高
- **BMP**: 24位BMP格式

## 6. 图片建议

- 分辨率：建议320x240或更小
- 文件大小：建议小于2MB
- 格式：JPEG格式获得最佳性能

## 7. 故障排除

### 显示屏不亮
- 检查电源连接
- 确认引脚连接正确
- 检查SPI连接

### WiFi连接失败
- 确认WiFi名称和密码正确
- 检查WiFi信号强度
- 查看串口输出的错误信息

### 图片不显示
- 确认图片格式支持
- 检查文件是否上传成功
- 查看串口输出的调试信息

## 8. 高级配置

可以在 `include/Config.h` 中修改更多设置：

- `WEB_SERVER_PORT`: Web服务器端口
- `IMAGE_UPDATE_INTERVAL`: 图片更新检查间隔
- `MAX_IMAGES`: 最大图片数量
- `MAX_FILE_SIZE`: 最大文件大小

## 9. 开发调试

使用串口监视器查看详细的调试信息：
- WiFi连接状态
- 文件上传进度
- 图片解码状态
- 错误信息

## 10. 扩展功能

项目支持进一步扩展：
- 添加更多图片格式支持
- 实现图片幻灯片功能
- 添加图片缩放功能
- 支持从网络下载图片
