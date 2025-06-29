# 颜色显示调试工具

## 🎨 颜色问题诊断

### 常见颜色异常现象
1. **五彩斑斓** - 通常是字节序问题
2. **颜色偏移** - RGB通道错位
3. **颜色反转** - 颜色空间转换错误
4. **颜色暗淡** - 亮度或对比度问题

## 🔧 已实现的修复

### 1. TJpg_Decoder配置优化
```cpp
// 正确的ILI9341配置
TJpgDec.setJpgScale(1);           // 1:1缩放
TJpgDec.setSwapBytes(true);       // 字节交换 - 关键设置！
TJpgDec.setCallback(jpegOutputCallback);
```

**关键点**: `setSwapBytes(true)` 是解决颜色问题的关键！

### 2. 改进的输出回调函数
```cpp
bool jpegOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    // 边界检查
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return 0;
    if (x < 0 || y < 0) return 0;
    
    // 裁剪到屏幕范围
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    
    // 直接传递RGB565数据到ILI9341
    Display::getTFT().drawRGBBitmap(x, y, bitmap, w, h);
    
    return 1;
}
```

### 3. 颜色测试功能
系统启动时会自动进行颜色测试：
- 显示7种基本颜色条
- 验证RGB565格式正确性
- 输出颜色值到串口

## 🧪 测试方法

### 1. 基本颜色测试
启动ESP32后，串口会显示：
```
Color test RED: 0xF800
Color test GREEN: 0x07E0
Color test BLUE: 0x001F
Color test YELLOW: 0xFFE0
Color test MAGENTA: 0xF81F
Color test CYAN: 0x07FF
Color test WHITE: 0xFFFF
Color test completed
```

### 2. RGB565颜色值对照
| 颜色 | RGB888 | RGB565 | 说明 |
|------|--------|--------|------|
| 红色 | #FF0000 | 0xF800 | 纯红 |
| 绿色 | #00FF00 | 0x07E0 | 纯绿 |
| 蓝色 | #0000FF | 0x001F | 纯蓝 |
| 黄色 | #FFFF00 | 0xFFE0 | 红+绿 |
| 洋红 | #FF00FF | 0xF81F | 红+蓝 |
| 青色 | #00FFFF | 0x07FF | 绿+蓝 |
| 白色 | #FFFFFF | 0xFFFF | 全色 |

### 3. 手动颜色测试
在串口监视器中可以看到每个像素块的颜色值，用于验证JPEG解码是否正确。

## 🔍 问题诊断步骤

### 步骤1: 检查基本颜色显示
1. 重启ESP32
2. 观察启动时的颜色条
3. 确认7种颜色是否正确显示

**预期结果**: 应该看到清晰的红、绿、蓝、黄、洋红、青、白色条

### 步骤2: 检查串口输出
```
JPEG decoder initialized for ILI9341 RGB565
Screen size: 320x240
Testing color display...
Color test RED: 0xF800
...
Color test completed
```

### 步骤3: 测试JPEG显示
1. 上传一张小的测试图片
2. 观察颜色是否正确
3. 检查串口的JPEG解码日志

## 🎯 RGB565格式说明

### 格式结构
```
RGB565: RRRRR GGGGGG BBBBB
- R: 5位红色 (0-31)
- G: 6位绿色 (0-63) 
- B: 5位蓝色 (0-31)
```

### 颜色转换公式
```cpp
// RGB888 to RGB565
uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

// RGB565 to RGB888
uint8_t r = (rgb565 >> 11) << 3;
uint8_t g = ((rgb565 >> 5) & 0x3F) << 2;
uint8_t b = (rgb565 & 0x1F) << 3;
```

## 🔧 常见问题和解决方案

### 问题1: 颜色完全错乱
**原因**: 字节序问题
**解决**: 确保 `TJpgDec.setSwapBytes(true)`

### 问题2: 红蓝颜色互换
**原因**: RGB/BGR格式混淆
**解决**: 检查TJpg_Decoder输出格式

### 问题3: 颜色暗淡
**原因**: 颜色深度转换问题
**解决**: 确认RGB565转换正确

### 问题4: 部分区域颜色异常
**原因**: 内存越界或缓冲区问题
**解决**: 检查边界条件和内存分配

## 🛠️ 调试工具

### 启用详细调试
在编译时添加调试标志：
```cpp
#define DEBUG_JPEG_CALLBACK
```

这会在串口输出详细的像素信息：
```
JPEG callback: x=0, y=0, w=16, h=16
First pixel: 0x1234
```

### 手动颜色测试
```cpp
// 在代码中添加测试
void testSpecificColor() {
    uint16_t testColor = 0xF800; // 红色
    tft.fillRect(0, 0, 50, 50, testColor);
    Serial.printf("Test color: 0x%04X\n", testColor);
}
```

## 📊 性能优化

### 内存使用
- JPEG解码缓冲: ~50KB
- 显示缓冲: ~150KB (320x240x2)
- 总内存需求: ~200KB

### 优化建议
1. **使用适当的图片尺寸** (320x240或更小)
2. **压缩JPEG质量** (80-90%)
3. **避免过大的图片** (>500KB)

## 🎉 验证成功标准

### 颜色测试通过标准
1. ✅ 启动颜色条显示正确
2. ✅ 串口输出正确的RGB565值
3. ✅ JPEG图片颜色自然真实
4. ✅ 无五彩斑斓现象
5. ✅ 红绿蓝三原色清晰

### 预期效果
- **红色**: 鲜艳的红色，不偏橙或紫
- **绿色**: 纯正的绿色，不偏黄或蓝
- **蓝色**: 深蓝色，不偏紫或青
- **肤色**: 自然的肤色，不偏红或绿
- **天空**: 自然的蓝色，不偏紫

## 📞 如果问题仍然存在

### 收集信息
1. **串口完整日志** - 包括颜色测试输出
2. **问题图片** - 提供测试用的图片文件
3. **硬件信息** - ESP32型号和显示屏连接
4. **现象描述** - 具体的颜色异常表现

### 进一步调试
1. 尝试不同的测试图片
2. 检查硬件连接
3. 验证SPI通信
4. 测试不同的图片格式

记住：正确的颜色显示需要硬件、驱动和软件的完美配合！
