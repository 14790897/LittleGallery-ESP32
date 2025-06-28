# 部署指南

## 🚀 完整部署流程

### 1. 环境准备

确保已安装：
- PlatformIO Core 或 PlatformIO IDE
- ESP32开发环境
- USB驱动程序

### 2. 硬件连接

按照以下方式连接ILI9341显示屏到ESP32-C3：

| ILI9341 | ESP32-C3 | 说明 |
|---------|----------|------|
| VCC     | 3.3V     | 电源正极 |
| GND     | GND      | 电源负极 |
| CS      | GPIO 7   | 片选信号 |
| DC      | GPIO 6   | 数据/命令选择 |
| RST     | GPIO 10  | 复位信号 |
| SCK     | GPIO 4   | SPI时钟 |
| MOSI    | GPIO 5   | SPI数据输出 |

### 3. 配置WiFi

编辑 `include/Config.h` 文件：

```cpp
// WiFi配置 - 请修改为您的WiFi信息
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

### 4. 编译和上传

#### 4.1 上传固件
```bash
cd LittleGallery-ESP32
pio run --target upload
```

#### 4.2 上传Web文件
```bash
pio run --target uploadfs
```

### 5. 验证部署

1. **检查串口输出**
   ```bash
   pio device monitor
   ```
   
2. **查看启动信息**
   - 显示屏应显示启动画面
   - 串口输出WiFi连接状态
   - 记录显示的IP地址

3. **测试Web界面**
   - 在浏览器中访问显示的IP地址
   - 验证页面正常加载
   - 测试文件上传功能

## 🔧 故障排除

### 编译错误

**问题**: 库依赖缺失
```
解决方案:
pio lib install
```

**问题**: 平台不支持
```
解决方案:
pio platform install espressif32
```

### 上传错误

**问题**: 端口权限问题
```bash
# Linux/Mac
sudo chmod 666 /dev/ttyUSB0

# Windows - 检查设备管理器中的COM端口
```

**问题**: 文件系统上传失败
```bash
# 确保LittleFS分区配置正确
# 检查 platformio.ini 中的 board_build.filesystem = littlefs
```

### 运行时问题

**问题**: 显示屏不亮
- 检查电源连接（3.3V，不是5V）
- 验证SPI引脚连接
- 检查杜邦线接触

**问题**: WiFi连接失败
- 确认WiFi名称和密码正确
- 检查WiFi信号强度
- 验证ESP32-C3支持的WiFi频段（2.4GHz）

**问题**: Web页面无法访问
- 确认IP地址正确
- 检查防火墙设置
- 验证设备在同一网络

**问题**: 图片上传失败
- 检查文件格式（支持JPG、JPEG、BMP）
- 确认文件大小不超过限制
- 验证LittleFS空间充足

## 📊 性能优化

### 存储优化
- 使用JPEG格式图片（压缩率更高）
- 控制图片分辨率（建议320x240或更小）
- 定期清理不需要的图片

### 网络优化
- 确保WiFi信号强度良好
- 避免网络拥塞时段上传大文件
- 考虑使用有线网络进行初始配置

### 显示优化
- 图片分辨率匹配屏幕尺寸
- 避免频繁切换图片
- 合理设置幻灯片间隔

## 🔒 安全建议

### 网络安全
- 更改默认WiFi配置
- 考虑使用WPA3加密
- 定期更新固件

### 访问控制
- 在生产环境中添加身份验证
- 限制文件上传大小
- 监控系统资源使用

## 📈 监控和维护

### 系统监控
- 定期检查串口输出
- 监控存储空间使用
- 观察WiFi连接稳定性

### 定期维护
- 清理临时文件
- 更新库依赖
- 备份重要配置

## 🆙 升级指南

### 固件升级
1. 备份当前配置
2. 更新代码
3. 重新编译上传
4. 验证功能正常

### Web文件升级
1. 修改 `data/` 目录文件
2. 运行 `pio run --target uploadfs`
3. 清除浏览器缓存
4. 测试新功能

## 📞 技术支持

### 常用命令
```bash
# 查看设备信息
pio device list

# 清理构建
pio run --target clean

# 查看库信息
pio lib list

# 更新平台
pio platform update
```

### 调试技巧
- 使用串口监视器查看详细日志
- 启用浏览器开发者工具
- 检查网络请求和响应
- 验证API接口返回数据

### 获取帮助
- 查看项目文档
- 检查PlatformIO官方文档
- 搜索相关技术论坛
- 提交Issue到项目仓库

## ✅ 部署检查清单

- [ ] 硬件连接正确
- [ ] WiFi配置已修改
- [ ] 固件编译成功
- [ ] 固件上传成功
- [ ] Web文件上传成功
- [ ] 显示屏正常显示
- [ ] WiFi连接成功
- [ ] Web界面可访问
- [ ] 图片上传功能正常
- [ ] 图片切换功能正常
- [ ] 系统状态显示正常

完成以上检查后，您的Little Gallery ESP32系统就可以正常使用了！
