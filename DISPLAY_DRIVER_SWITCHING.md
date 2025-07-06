# 🖥️ ESP32显示驱动切换功能

## 📋 功能概述

新增了显示驱动动态切换功能，支持在ILI9341和ST7789之间无缝切换，无需重新编译固件。

## 🔧 支持的显示驱动

### 1. ILI9341
- **分辨率**: 320×240
- **接口**: SPI
- **颜色**: 16位RGB565
- **适用场景**: 标准2.8寸TFT显示屏

### 2. ST7789
- **分辨率**: 240×240 (可配置)
- **接口**: SPI
- **颜色**: 16位RGB565
- **适用场景**: 圆形或方形TFT显示屏

## 🏗️ 架构设计

### 抽象基类 (DisplayDriverBase)
```cpp
class DisplayDriverBase {
public:
    virtual bool begin() = 0;
    virtual void setRotation(uint8_t rotation) = 0;
    virtual void clearScreen(uint16_t color = 0x0000) = 0;
    virtual void displayText(const char* text, ...) = 0;
    // ... 其他虚函数
};
```

### 显示管理器 (DisplayManager)
- 统一的驱动管理接口
- 动态驱动切换
- 透明的API代理

### 具体驱动实现
- **ILI9341Driver**: 继承DisplayDriverBase
- **ST7789Driver**: 继承DisplayDriverBase

## 🔌 API接口

### 1. 获取当前驱动状态
```http
GET /api/display-driver
```

**响应示例**:
```json
{
  "current_driver": "ILI9341",
  "current_driver_type": 0,
  "available_drivers": ["ILI9341", "ST7789"],
  "status": "ok"
}
```

### 2. 切换显示驱动
```http
POST /api/display-driver
Content-Type: application/x-www-form-urlencoded

driver=ST7789
```

**响应示例**:
```json
{
  "status": "ok",
  "current_driver": "ST7789",
  "current_driver_type": 1,
  "message": "Display driver switched to ST7789"
}
```

## 🎮 前端控制界面

### 系统状态显示
- 实时显示当前使用的驱动
- 在系统信息面板中显示

### 驱动切换控件
- 下拉选择器选择目标驱动
- 一键切换功能
- 切换状态提示

### 用户体验
- 切换过程中显示进度提示
- 切换完成后自动刷新图片列表
- 错误处理和用户反馈

## 🔧 硬件配置

### 引脚定义 (secrets.h)
```cpp
// SPI引脚配置
#define TFT_CS    7   // 片选引脚
#define TFT_DC    2   // 数据/命令引脚
#define TFT_RST   10  // 复位引脚

// 屏幕尺寸
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
```

### 注意事项
- 两种驱动使用相同的SPI引脚配置
- 只需要更改软件驱动，无需修改硬件连接
- 确保引脚定义与实际硬件匹配

## 🚀 使用方法

### 1. 编译和上传
```bash
# 编译项目
pio run

# 上传固件
pio run --target upload

# 上传文件系统
pio run --target uploadfs
```

### 2. 运行时切换
1. 打开Web界面
2. 在系统信息部分查看当前驱动
3. 在显示驱动部分选择目标驱动
4. 点击切换，等待完成

### 3. 验证切换
- 观察ESP32串口输出
- 检查显示屏是否正常工作
- 验证图片显示效果

## 📊 技术特点

### 1. 动态切换
- 运行时无缝切换
- 无需重启设备
- 保持网络连接

### 2. 统一接口
- 相同的API调用
- 透明的驱动切换
- 向后兼容

### 3. 错误处理
- 切换失败自动回滚
- 详细的错误信息
- 用户友好的提示

### 4. 性能优化
- 最小化内存使用
- 快速驱动初始化
- 高效的资源管理

## 🐛 故障排除

### 常见问题

#### 1. 切换后显示异常
- **原因**: 引脚配置不匹配
- **解决**: 检查secrets.h中的引脚定义

#### 2. 切换失败
- **原因**: 驱动初始化失败
- **解决**: 检查硬件连接和电源

#### 3. 图片显示错误
- **原因**: 分辨率不匹配
- **解决**: 重新上传适合的图片尺寸

### 调试方法
1. 查看串口输出日志
2. 使用Web界面的调试工具
3. 测试显示颜色功能

## 🔮 未来扩展

### 计划支持的驱动
- **ST7735**: 1.8寸TFT显示屏
- **SSD1306**: OLED显示屏
- **ILI9488**: 3.5寸TFT显示屏

### 功能增强
- 自动检测显示屏类型
- 驱动参数在线配置
- 多显示屏同时支持

现在您的ESP32图片相册支持动态切换显示驱动，可以轻松适配不同类型的显示屏！
