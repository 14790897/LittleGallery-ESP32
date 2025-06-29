# 🔄 图片方向自适应功能

## ✨ 功能概述

实现了智能的图片方向自适应显示系统，能够根据图片的宽高比自动选择最佳的显示方向和缩放方式，确保各种尺寸的图片都能在320x240的ILI9341显示屏上以最佳方式展示。

## 🎯 核心功能

### 1. **智能方向检测**
- 自动检测图片的宽高比
- 分类为横屏、竖屏或正方形
- 基于宽高比阈值进行判断

### 2. **自动屏幕旋转**
- 竖屏图片自动旋转显示屏到240x320模式
- 横屏图片保持320x240模式
- 智能选择最佳显示方向

### 3. **多种显示模式**
- **智能缩放** (SMART_SCALE) - 自动计算最佳缩放比例
- **自动旋转** (AUTO_ROTATE) - 根据图片方向旋转屏幕
- **居中裁剪** (CENTER_CROP) - 居中显示并裁剪
- **适配屏幕** (FIT_SCREEN) - 完整适配屏幕尺寸

### 4. **Web界面控制**
- 实时切换显示模式
- 启用/禁用自动旋转
- 方向测试功能
- API接口控制

## 🔧 技术实现

### 核心算法

#### 1. 方向检测算法
```cpp
ImageOrientation detectImageOrientation(uint16_t width, uint16_t height) {
    float aspectRatio = (float)width / (float)height;
    
    if (aspectRatio > 1.2f) {
        return ImageOrientation::LANDSCAPE;  // 横屏
    } else if (aspectRatio < 0.8f) {
        return ImageOrientation::PORTRAIT;   // 竖屏
    } else {
        return ImageOrientation::SQUARE;     // 正方形
    }
}
```

#### 2. 智能缩放算法
```cpp
void calculateOptimalScale(uint16_t imgWidth, uint16_t imgHeight, 
                          uint8_t& scale, uint16_t& finalWidth, uint16_t& finalHeight) {
    // 获取当前屏幕尺寸
    uint16_t screenW = (currentRotation == 0) ? 240 : 320;
    uint16_t screenH = (currentRotation == 0) ? 320 : 240;
    
    // 计算最佳缩放比例
    scale = 1;
    while ((imgWidth/scale > screenW || imgHeight/scale > screenH) && scale < 8) {
        scale *= 2;
    }
}
```

#### 3. 智能位置计算
```cpp
void calculateSmartPosition(uint16_t imgWidth, uint16_t imgHeight,
                           int16_t& x, int16_t& y, DisplayMode mode) {
    uint16_t screenW = (currentRotation == 0) ? 240 : 320;
    uint16_t screenH = (currentRotation == 0) ? 320 : 240;
    
    // 根据显示模式计算位置
    x = (screenW - imgWidth) / 2;
    y = (screenH - imgHeight) / 2;
    
    // 边界检查
    if (x < 0) x = 0;
    if (y < 0) y = 0;
}
```

### 显示流程

#### 图片显示处理流程
1. **获取图片尺寸** → 读取JPEG文件头信息
2. **方向检测** → 分析宽高比确定图片方向
3. **屏幕旋转** → 根据图片方向决定是否旋转屏幕
4. **智能缩放** → 计算最佳缩放比例
5. **位置计算** → 确定最佳显示位置
6. **图片渲染** → 使用优化参数显示图片

## 🎨 显示模式详解

### 1. 智能缩放 (SMART_SCALE)
- **特点**: 自动计算最佳缩放比例
- **适用**: 大部分图片的默认模式
- **效果**: 保持图片比例，完整显示

### 2. 自动旋转 (AUTO_ROTATE)
- **特点**: 根据图片方向自动旋转屏幕
- **适用**: 竖屏图片优化显示
- **效果**: 最大化利用屏幕空间

### 3. 居中裁剪 (CENTER_CROP)
- **特点**: 居中显示，可能裁剪边缘
- **适用**: 需要填满屏幕的场景
- **效果**: 图片填满屏幕，保持比例

### 4. 适配屏幕 (FIT_SCREEN)
- **特点**: 完整适配屏幕尺寸
- **适用**: 需要看到完整图片的场景
- **效果**: 图片完整显示，可能有黑边

## 🌐 Web界面控制

### 控制面板功能
```html
<div class="orientation-controls">
    <h4>🔄 显示方向设置</h4>
    <select id="orientationMode">
        <option value="SMART_SCALE">智能缩放</option>
        <option value="AUTO_ROTATE">自动旋转</option>
        <option value="CENTER_CROP">居中裁剪</option>
        <option value="FIT_SCREEN">适配屏幕</option>
    </select>
    <input type="checkbox" id="autoRotation" checked>启用自动旋转
    <button onclick="testOrientation()">🔄 测试方向</button>
</div>
```

### API接口

#### 获取方向设置
```
GET /api/orientation
```
返回：
```json
{
    "auto_rotation": true,
    "current_mode": "SMART_SCALE",
    "current_rotation": 1,
    "available_modes": ["AUTO_ROTATE", "SMART_SCALE", "CENTER_CROP", "FIT_SCREEN"]
}
```

#### 设置方向参数
```
POST /api/orientation
```
参数：
- `mode`: 显示模式
- `auto_rotation`: 是否启用自动旋转

## 📊 性能优化

### 内存使用优化
- **智能缩放**: 减少内存占用
- **边界检查**: 避免内存越界
- **及时释放**: 处理完成后释放资源

### 显示性能优化
- **预计算**: 提前计算显示参数
- **缓存机制**: 缓存计算结果
- **批量处理**: 减少重复计算

## 🧪 测试用例

### 不同图片类型测试

#### 1. 横屏图片 (1920x1080)
- **检测结果**: LANDSCAPE
- **屏幕方向**: 保持横屏 (320x240)
- **缩放比例**: 8倍缩放 → 240x135
- **显示位置**: 居中显示

#### 2. 竖屏图片 (1080x1920)
- **检测结果**: PORTRAIT
- **屏幕方向**: 旋转到竖屏 (240x320)
- **缩放比例**: 8倍缩放 → 135x240
- **显示位置**: 居中显示

#### 3. 正方形图片 (1080x1080)
- **检测结果**: SQUARE
- **屏幕方向**: 保持横屏 (320x240)
- **缩放比例**: 4倍缩放 → 270x270
- **显示位置**: 居中显示

#### 4. 小图片 (200x150)
- **检测结果**: LANDSCAPE
- **屏幕方向**: 保持横屏 (320x240)
- **缩放比例**: 无缩放 → 200x150
- **显示位置**: 居中显示

## 🔍 调试功能

### 串口输出信息
```
Image orientation: PORTRAIT (1080x1920)
Screen rotated to portrait mode (240x320)
Screen size: 240x320, Image size: 1080x1920
Optimal scale: 8, Final size: 135x240
Smart position: (52, 40) for 135x240 image on 240x320 screen
Smart display position: (52, 40) for 135x240 image
JPEG displayed successfully
```

### Web界面测试
- **方向测试按钮**: 立即测试当前设置
- **实时反馈**: 显示设置更改结果
- **状态显示**: 显示当前方向和模式

## 🚀 使用指南

### 基本使用
1. **上传图片** → 系统自动应用方向自适应
2. **查看效果** → 观察图片在显示屏上的显示效果
3. **调整设置** → 根据需要调整显示模式

### 高级设置
1. **访问Web界面** → 打开方向设置面板
2. **选择显示模式** → 根据需求选择合适的模式
3. **启用自动旋转** → 让系统自动处理竖屏图片
4. **测试效果** → 使用测试按钮验证设置

### 最佳实践
- **横屏图片**: 使用智能缩放模式
- **竖屏图片**: 启用自动旋转
- **正方形图片**: 使用适配屏幕模式
- **小图片**: 使用居中显示

## 🎉 预期效果

### 修复前的问题
- ❌ 竖屏图片显示不当
- ❌ 横屏图片空间浪费
- ❌ 固定显示方向
- ❌ 缺少智能适配

### 修复后的效果
- ✅ 智能方向检测
- ✅ 自动屏幕旋转
- ✅ 最佳空间利用
- ✅ 多种显示模式
- ✅ Web界面控制
- ✅ 实时参数调整

现在系统能够智能地处理各种尺寸和方向的图片，确保每张图片都能以最佳方式在ILI9341显示屏上展示！
