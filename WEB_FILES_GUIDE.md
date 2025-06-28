# Web文件管理指南

## 📁 文件结构

项目现在使用LittleFS来存储前端文件，所有Web相关文件都放在`data/`文件夹中：

```
LittleGallery-ESP32/
├── data/
│   ├── index.html     # 主页面
│   ├── style.css      # 样式文件
│   └── app.js         # JavaScript应用
├── scripts/
│   └── build_web.py   # Web文件构建脚本
└── ...
```

## 🚀 部署步骤

### 1. 编译并上传固件
```bash
pio run --target upload
```

### 2. 上传文件系统
```bash
pio run --target uploadfs
```

### 3. 访问Web界面
- 打开串口监视器查看IP地址
- 在浏览器中访问显示的IP地址

## ✨ 前端功能特性

### 🎨 现代化界面
- 响应式设计，支持移动设备
- 渐变背景和现代化UI组件
- 平滑动画和过渡效果
- 直观的图标和视觉反馈

### 📤 文件上传
- 支持拖拽上传
- 多文件同时上传
- 上传进度显示
- 文件格式验证

### 🎮 图片控制
- 上一张/下一张切换
- 直接选择图片
- 自动幻灯片播放
- 实时状态更新

### 📊 系统监控
- WiFi连接状态
- 存储空间使用情况
- 系统运行时间
- 图片统计信息

### 🗂️ 图片管理
- 图片列表显示
- 批量选择和删除
- 当前图片高亮显示
- 实时列表更新

## 🔧 自定义修改

### 修改样式
编辑 `data/style.css` 文件来自定义界面样式：

```css
:root {
    --primary-color: #007bff;    /* 主色调 */
    --success-color: #28a745;    /* 成功色 */
    --danger-color: #dc3545;     /* 危险色 */
    /* ... 更多变量 */
}
```

### 修改功能
编辑 `data/app.js` 文件来添加或修改功能：

```javascript
class LittleGallery {
    constructor() {
        // 自定义配置
        this.slideshowInterval = 3000; // 幻灯片间隔
        // ...
    }
}
```

### 修改页面结构
编辑 `data/index.html` 文件来修改页面布局和内容。

## 📱 API接口

前端通过以下API与ESP32通信：

### 图片管理
- `GET /api/images` - 获取图片列表
- `POST /api/next` - 切换到下一张
- `POST /api/previous` - 切换到上一张
- `POST /api/setimage` - 设置当前图片
- `POST /api/delete` - 删除图片

### 系统状态
- `GET /api/status` - 获取系统状态

### 文件操作
- `POST /upload` - 上传文件

## 🛠️ 开发调试

### 本地开发
1. 修改 `data/` 目录中的文件
2. 重新上传文件系统：`pio run --target uploadfs`
3. 刷新浏览器查看更改

### 调试技巧
- 使用浏览器开发者工具查看网络请求
- 检查控制台错误信息
- 使用串口监视器查看ESP32日志

## 📦 文件压缩（可选）

为了节省存储空间，可以启用文件压缩：

1. 在 `platformio.ini` 中添加：
```ini
custom_compress_web = true
```

2. 重新构建项目

## 🔄 更新Web文件

当需要更新Web界面时：

1. 修改 `data/` 目录中的文件
2. 运行 `pio run --target uploadfs` 上传新文件
3. 重启ESP32或刷新浏览器

## 🚨 注意事项

### 文件大小限制
- 确保所有Web文件总大小不超过LittleFS分区大小
- 建议压缩大型文件或使用CDN

### 浏览器兼容性
- 使用现代JavaScript特性，建议使用较新的浏览器
- 支持Chrome、Firefox、Safari、Edge等主流浏览器

### 网络安全
- 在生产环境中考虑添加身份验证
- 使用HTTPS（需要额外配置）

## 🎯 最佳实践

1. **模块化开发** - 将功能分离到不同的JavaScript模块
2. **错误处理** - 添加适当的错误处理和用户反馈
3. **性能优化** - 压缩文件，减少HTTP请求
4. **用户体验** - 提供加载指示器和状态反馈
5. **响应式设计** - 确保在不同设备上都能正常使用

## 📚 扩展功能建议

- 添加图片预览功能
- 支持更多图片格式
- 添加图片编辑功能
- 实现用户认证
- 添加图片分类和标签
- 支持图片批量下载
