# 文件名自动处理功能

## 📋 问题背景

LittleFS 文件系统对文件名长度有严格限制：
- **最大长度**: 31个字符（包含路径）
- **实际可用**: 约25个字符（考虑路径前缀）
- **常见问题**: 中文文件名、长英文名、特殊字符

## ✨ 解决方案

系统现在自动处理文件名，确保符合LittleFS要求：

### 🔧 前端处理
1. **自动重命名**: 选择文件后自动生成安全文件名
2. **用户提示**: 显示原文件名和新文件名
3. **透明处理**: 用户无需手动修改文件名

### 🛠️ 后端验证
1. **双重检查**: 后端再次验证文件名安全性
2. **错误处理**: 文件名过长时的备用方案
3. **调试信息**: 详细的日志输出

## 📝 文件名处理规则

### 1. 字符清理
```javascript
// 只保留安全字符
const safeChars = /[a-zA-Z0-9_-]/g;

// 替换特殊字符
filename.replace(/[^a-zA-Z0-9_-]/g, '_')
```

**处理示例**:
- `我的照片.jpg` → `image_123456.jpg`
- `My Photo (1).jpeg` → `My_Photo_1_123456.jpeg`
- `IMG-20231201-WA0001.jpg` → `IMG_20231201_WA0001_123456.jpg`

### 2. 长度控制
```javascript
// 最大总长度: 25字符
// 结构: 名称_时间戳.扩展名
// 示例: photo_123456.jpg (16字符)
```

**长度分配**:
- 扩展名: 4字符 (`.jpg`)
- 时间戳: 7字符 (`_123456`)
- 可用名称: 14字符

### 3. 唯一性保证
```javascript
// 添加毫秒时间戳
const timestamp = Date.now().toString().slice(-6);
const finalName = `${cleanName}_${timestamp}${extension}`;
```

## 🎯 处理流程

### 前端流程
1. **文件选择** → 用户选择图片文件
2. **名称检查** → 检查文件名长度和字符
3. **自动处理** → 生成安全的文件名
4. **用户通知** → 显示重命名信息
5. **开始上传** → 使用新文件名上传

### 后端流程
1. **接收文件** → 获取上传的文件
2. **名称验证** → 再次检查文件名安全性
3. **备用处理** → 如果仍有问题，使用备用方案
4. **文件保存** → 保存到LittleFS
5. **列表更新** → 更新图片列表

## 💡 用户体验

### 自动提示
```html
<span class="upload-hint">
  📝 长文件名将自动缩短以符合系统要求
</span>
```

### 状态反馈
```javascript
// 重命名提示
this.showStatus(
  `文件 "${originalName}" 已重命名为 "${safeFileName}" 并上传成功`, 
  "success"
);

// 正常上传
this.showStatus(`文件 ${originalName} 上传成功`, "success");
```

## 🔍 技术细节

### 前端实现
```javascript
generateSafeFileName(originalName) {
  // 1. 分离文件名和扩展名
  const lastDotIndex = originalName.lastIndexOf('.');
  const extension = lastDotIndex > -1 ? originalName.substring(lastDotIndex) : '';
  const nameWithoutExt = lastDotIndex > -1 ? originalName.substring(0, lastDotIndex) : originalName;
  
  // 2. 清理特殊字符
  let cleanName = nameWithoutExt
    .replace(/[^a-zA-Z0-9_-]/g, '_')
    .replace(/_+/g, '_')
    .replace(/^_|_$/g, '');
  
  // 3. 处理空名称
  if (!cleanName) {
    cleanName = 'image';
  }
  
  // 4. 控制长度
  const maxNameLength = 25 - extension.length - 8;
  if (cleanName.length > maxNameLength) {
    cleanName = cleanName.substring(0, maxNameLength);
  }
  
  // 5. 添加时间戳
  const timestamp = Date.now().toString().slice(-6);
  return `${cleanName}_${timestamp}${extension}`;
}
```

### 后端实现
```cpp
String WebServerController::generateSafeFilename(const String& originalName) const {
  // 1. 获取扩展名
  int lastDotIndex = originalName.lastIndexOf('.');
  String extension = (lastDotIndex > -1) ? originalName.substring(lastDotIndex) : "";
  String nameWithoutExt = (lastDotIndex > -1) ? originalName.substring(0, lastDotIndex) : originalName;
  
  // 2. 字符清理
  String cleanName = "";
  for (int i = 0; i < nameWithoutExt.length(); i++) {
    char c = nameWithoutExt.charAt(i);
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
        (c >= '0' && c <= '9') || c == '_' || c == '-') {
      cleanName += c;
    } else {
      cleanName += '_';
    }
  }
  
  // 3. 长度控制和时间戳
  unsigned long timestamp = millis() % 1000000;
  String finalName = cleanName + "_" + String(timestamp) + extension;
  
  return finalName;
}
```

## 📊 测试用例

### 常见文件名测试
| 原文件名 | 处理后文件名 | 说明 |
|---------|-------------|------|
| `照片.jpg` | `image_123456.jpg` | 中文转换 |
| `My Very Long Photo Name.jpeg` | `My_Very_Long_Ph_123456.jpeg` | 长名称截断 |
| `IMG@#$%^&*().bmp` | `IMG_123456.bmp` | 特殊字符清理 |
| `photo-2023-12-01.jpg` | `photo_2023_12_01_123456.jpg` | 连字符保留 |
| `.hidden.jpg` | `hidden_123456.jpg` | 隐藏文件处理 |

### 边界情况测试
- 空文件名 → `image_123456.jpg`
- 只有扩展名 → `image_123456.ext`
- 超长扩展名 → 使用备用方案
- 无扩展名 → 添加默认扩展名

## 🔧 配置选项

### 可调整参数
```javascript
// 前端配置
const CONFIG = {
  MAX_FILENAME_LENGTH: 25,    // 最大文件名长度
  TIMESTAMP_LENGTH: 6,        // 时间戳长度
  DEFAULT_NAME: 'image',      // 默认文件名
  ALLOWED_CHARS: /[a-zA-Z0-9_-]/g  // 允许的字符
};
```

```cpp
// 后端配置
const int MAX_FILENAME_LENGTH = 25;
const String DEFAULT_NAME = "image";
```

## 🚀 未来改进

### 可能的增强功能
1. **智能命名** - 基于图片内容生成名称
2. **用户自定义** - 允许用户设置命名规则
3. **批量重命名** - 批量上传时的智能命名
4. **名称冲突处理** - 更智能的重复名称处理
5. **元数据保留** - 在数据库中保存原始文件名

### 性能优化
1. **缓存机制** - 缓存已处理的文件名
2. **异步处理** - 后台处理文件名转换
3. **压缩存储** - 压缩长文件名映射表
