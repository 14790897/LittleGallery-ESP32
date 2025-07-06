# 🚀 构建自动化 - GitHub Actions

## 📋 概述

本项目使用GitHub Actions自动构建ESP32固件，支持PlatformIO构建系统，使用您的自定义`custom.csv`分区表。

## 🔧 GitHub Actions配置

### 构建工作流

项目包含两个GitHub Actions工作流：

1. **PlatformIO Build** (`.github/workflows/platformio-build.yml`)
   - 使用PlatformIO构建系统
   - 支持多种ESP32环境
   - 自动生成构建产物

2. **Build** (`.github/workflows/esp-idf-build.yml`)
   - 简化的PlatformIO构建
   - 专注于核心功能

### 触发条件

构建会在以下情况自动触发：

- **Push到主分支**: `main`, `develop`
- **Pull Request**: 针对`main`分支
- **手动触发**: 通过GitHub界面的"Run workflow"

### 手动触发选项

在GitHub Actions页面可以手动触发构建，支持以下选项：

- **构建环境**: 
  - `airm2m_core_esp32c3` (默认)
  - `esp32dev`
  - `esp32-s2-saola-1`
  - `esp32-s3-devkitc-1`
  - `esp32-c3-devkitm-1`

## 📦 构建产物

每次成功构建会生成以下文件：

### 固件文件
- `firmware.bin` - 主应用程序二进制文件
- `littlefs.bin` - LittleFS文件系统镜像
- `bootloader.bin` - ESP32引导加载程序
- `partitions.bin` - 分区表（来自custom.csv）

### 调试文件
- `firmware.elf` - ELF调试文件
- `firmware.map` - 内存映射文件

### 文档
- `build_info.md` - 构建信息和刷写说明

## 🔄 自定义分区表

项目使用您的`custom.csv`分区表文件，构建过程会：

1. 验证`custom.csv`文件存在
2. 显示分区表内容
3. 使用该分区表进行构建

### 分区表格式示例
```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x140000,
app1,     app,  ota_1,   0x150000,0x140000,
littlefs, data, spiffs,  0x290000,0x160000,
```

## 📥 下载构建产物

### 从GitHub Actions

1. 进入项目的GitHub页面
2. 点击"Actions"标签
3. 选择最新的成功构建
4. 在"Artifacts"部分下载构建文件

### 构建产物命名

文件命名格式：`littlegallery-esp32-{环境}-{构建号}`

例如：`littlegallery-esp32-airm2m_core_esp32c3-123`

## ⚡ 刷写固件

### 使用esptool.py

```bash
# 刷写完整固件
esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 write_flash \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin \
  0x290000 littlefs.bin

# 仅刷写应用程序
esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 write_flash \
  0x10000 firmware.bin

# 仅刷写文件系统
esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 write_flash \
  0x290000 littlefs.bin
```

### 使用PlatformIO

```bash
# 刷写固件
pio run -e airm2m_core_esp32c3 -t upload

# 刷写文件系统
pio run -e airm2m_core_esp32c3 -t uploadfs
```

## 🏷️ 发布版本

### 自动发布

当推送Git标签时，GitHub Actions会自动创建发布版本：

```bash
# 创建标签
git tag v1.0.0
git push origin v1.0.0
```

### 发布内容

自动发布包含：
- 所有构建的二进制文件
- 构建信息文档
- 版本说明

## 🔧 本地构建

### 环境要求

- Python 3.11+
- PlatformIO Core

### 构建命令

```bash
# 安装PlatformIO
pip install platformio

# 构建固件
pio run -e airm2m_core_esp32c3

# 构建文件系统
pio run -e airm2m_core_esp32c3 -t buildfs

# 刷写固件
pio run -e airm2m_core_esp32c3 -t upload

# 刷写文件系统
pio run -e airm2m_core_esp32c3 -t uploadfs
```

## 📊 构建状态

### 状态徽章

可以在README中添加构建状态徽章：

```markdown
![Build Status](https://github.com/your-username/LittleGallery-ESP32/workflows/PlatformIO%20Build/badge.svg)
```

### 构建历史

- 所有构建历史在GitHub Actions页面可见
- 构建日志包含详细的编译信息
- 失败的构建会显示错误详情

## 🐛 故障排除

### 常见问题

1. **构建失败**
   - 检查`secrets.h`文件是否存在
   - 验证`custom.csv`分区表格式
   - 查看构建日志中的错误信息

2. **分区表错误**
   - 确保`custom.csv`文件格式正确
   - 检查分区大小和偏移量
   - 验证分区不重叠

3. **依赖问题**
   - 检查`platformio.ini`中的库依赖
   - 确保所有必需的库都已列出

### 调试方法

1. 查看GitHub Actions构建日志
2. 本地复现构建过程
3. 检查PlatformIO配置

## 🔮 扩展功能

### 计划中的改进

- 多目标并行构建
- 自动测试集成
- 代码质量检查
- 安全扫描

### 自定义构建

可以通过修改`.github/workflows/`中的YAML文件来自定义构建过程：

- 添加新的构建环境
- 修改构建步骤
- 添加额外的检查

现在您的项目具备了完整的自动化构建系统，使用PlatformIO和您的自定义分区表！
