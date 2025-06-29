# 📤 批量上传功能

## ✨ 功能概述

实现了完整的批量上传功能，允许用户一次选择多个图片文件并同时上传，提供详细的进度显示、错误处理和存储空间管理。

## 🎯 核心功能

### 1. **智能上传模式**
- **单文件上传** - 简化界面，快速上传
- **批量上传** - 详细进度，批量处理
- **自动切换** - 根据文件数量自动选择模式

### 2. **详细进度显示**
- **总体进度条** - 显示整体上传进度
- **文件列表** - 每个文件的详细状态
- **实时更新** - 动态更新上传状态
- **取消功能** - 随时取消批量上传

### 3. **存储空间管理**
- **空间检查** - 上传前检查可用空间
- **智能提醒** - 根据空间使用率给出建议
- **容量估算** - 估算可上传的图片数量

### 4. **错误处理和恢复**
- **格式验证** - 自动过滤不支持的文件
- **失败重试** - 单个文件失败不影响其他文件
- **详细报告** - 显示成功和失败的详细信息

## 🔧 技术实现

### 前端架构

#### 1. 智能文件处理
```javascript
async processFiles(files) {
    if (files.length === 0) {
        this.showStatus("没有选择文件", "warning");
        return;
    }

    // 智能模式切换
    if (files.length > 1) {
        await this.processBatchUpload(files);  // 批量模式
    } else {
        await this.processSingleFile(files[0]); // 单文件模式
    }
}
```

#### 2. 批量上传流程
```javascript
async processBatchUpload(files) {
    // 1. 存储空间检查
    const storageCheck = await this.checkStorageSpace(files);
    if (!storageCheck.canUpload) {
        this.showStatus(storageCheck.message, "error");
        return;
    }

    // 2. 显示批量上传界面
    this.showBatchUploadProgress(true);
    this.batchUploadCancelled = false;

    // 3. 逐个处理文件
    for (let i = 0; i < files.length; i++) {
        if (this.batchUploadCancelled) break;
        
        const file = files[i];
        const fileId = this.addFileToProgressList(file.name, "uploading");
        
        try {
            await this.uploadFileWithName(file, safeFileName);
            this.updateFileProgress(fileId, "success", "上传成功");
            successCount++;
        } catch (error) {
            this.updateFileProgress(fileId, "error", error.message);
            failedCount++;
        }
    }

    // 4. 显示上传总结
    this.showBatchUploadSummary(successCount, failedCount, results);
}
```

#### 3. 存储空间检查
```javascript
async checkStorageSpace(files) {
    const response = await fetch("/api/upload-status");
    const status = await response.json();

    // 计算文件总大小
    let totalSize = 0;
    for (const file of files) {
        totalSize += file.size;
    }

    // 检查空间是否足够
    if (totalSize > status.available_storage) {
        return {
            canUpload: false,
            message: "存储空间不足！"
        };
    }

    // 检查使用率警告
    if (status.storage_used_percent > 75) {
        return {
            canUpload: true,
            warning: true,
            message: "存储空间较少，建议适量上传"
        };
    }

    return { canUpload: true };
}
```

### 后端支持

#### 1. 上传状态API
```cpp
void WebServerController::handleUploadStatusAPI(AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    // 存储信息
    doc["free_heap"] = ESP.getFreeHeap();
    doc["used_storage"] = LittleFS.usedBytes();
    doc["total_storage"] = LittleFS.totalBytes();
    doc["available_storage"] = LittleFS.totalBytes() - LittleFS.usedBytes();
    
    // 使用率计算
    float storageUsedPercent = (float)LittleFS.usedBytes() / LittleFS.totalBytes() * 100;
    doc["storage_used_percent"] = storageUsedPercent;
    
    // 容量估算
    size_t availableBytes = LittleFS.totalBytes() - LittleFS.usedBytes();
    int estimatedImageCount = availableBytes / (100 * 1024); // 100KB per image
    doc["estimated_uploadable_images"] = estimatedImageCount;
    
    // 上传建议
    if (storageUsedPercent > 90) {
        doc["upload_recommendation"] = "storage_almost_full";
        doc["message"] = "存储空间不足，建议删除一些图片后再上传";
    } else if (storageUsedPercent > 75) {
        doc["upload_recommendation"] = "storage_warning";
        doc["message"] = "存储空间较少，建议适量上传";
    } else {
        doc["upload_recommendation"] = "storage_ok";
        doc["message"] = "存储空间充足，可以批量上传";
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
```

## 🎨 用户界面

### 批量上传进度界面
```html
<div id="batchUploadProgress" class="batch-progress">
    <!-- 进度头部 -->
    <div class="progress-header">
        <h4>📤 批量上传进度</h4>
        <button onclick="cancelBatchUpload()" class="cancel-btn">❌ 取消</button>
    </div>
    
    <!-- 总体进度 -->
    <div class="overall-progress">
        <div class="progress-info">
            <span id="uploadProgressText">准备上传...</span>
            <span id="uploadProgressCount">0/0</span>
        </div>
        <div class="progress-bar">
            <div id="overallProgressBar" class="progress-fill"></div>
        </div>
    </div>
    
    <!-- 文件进度列表 -->
    <div id="fileProgressList" class="file-progress-list">
        <!-- 动态生成的文件进度项 -->
    </div>
</div>
```

### 文件进度项
```html
<div class="file-progress-item">
    <div class="file-info">
        <span class="file-name">example.jpg</span>
        <span class="file-status success">上传成功</span>
    </div>
    <div class="file-progress-bar">
        <div class="file-progress-fill"></div>
    </div>
</div>
```

## 📊 功能特性

### 1. 智能模式切换
- **单文件**: 简化界面，快速上传
- **多文件**: 详细进度，批量处理
- **自动检测**: 根据选择的文件数量自动切换

### 2. 进度可视化
- **总体进度条**: 显示整体完成百分比
- **文件计数**: 显示"当前/总数"
- **状态文本**: 实时更新当前操作
- **文件列表**: 每个文件的详细状态

### 3. 存储管理
- **预检查**: 上传前检查可用空间
- **智能提醒**: 根据使用率给出建议
- **容量估算**: 估算可上传图片数量
- **实时监控**: 动态更新存储状态

### 4. 错误处理
- **格式过滤**: 自动跳过不支持的文件
- **独立处理**: 单个文件失败不影响其他
- **详细报告**: 显示失败原因和统计
- **用户友好**: 清晰的错误信息

## 🚀 使用指南

### 基本使用
1. **选择文件** - 点击上传区域或拖拽文件
2. **自动检测** - 系统自动选择上传模式
3. **进度监控** - 观察上传进度和状态
4. **完成确认** - 查看上传结果总结

### 批量上传流程
1. **多选文件** - 选择多个图片文件
2. **空间检查** - 系统自动检查存储空间
3. **开始上传** - 显示批量上传界面
4. **进度监控** - 实时查看每个文件状态
5. **取消选项** - 可随时取消剩余上传
6. **结果总结** - 查看成功和失败统计

### 最佳实践
- **文件准备**: 提前整理好要上传的图片
- **空间管理**: 定期清理不需要的图片
- **网络稳定**: 确保WiFi连接稳定
- **分批上传**: 大量文件建议分批上传

## 📈 性能优化

### 前端优化
- **异步处理**: 使用async/await避免阻塞
- **内存管理**: 及时清理文件引用
- **进度缓存**: 缓存进度状态减少DOM操作
- **错误恢复**: 智能重试机制

### 后端优化
- **并发控制**: 合理控制同时上传数量
- **内存监控**: 实时监控ESP32内存使用
- **存储优化**: 高效的文件系统操作
- **错误处理**: 完善的异常处理机制

## 🎉 预期效果

### 用户体验提升
- ✅ **操作简化** - 一次选择多个文件
- ✅ **进度可见** - 清晰的上传进度显示
- ✅ **错误友好** - 详细的错误信息和处理
- ✅ **空间管理** - 智能的存储空间提醒

### 功能完善性
- ✅ **批量处理** - 支持多文件同时上传
- ✅ **进度控制** - 可取消和监控上传过程
- ✅ **存储管理** - 智能的空间检查和建议
- ✅ **错误恢复** - 单个失败不影响整体

### 系统稳定性
- ✅ **内存优化** - 合理的内存使用
- ✅ **错误隔离** - 单个文件错误不影响其他
- ✅ **资源管理** - 及时释放系统资源
- ✅ **状态同步** - 前后端状态保持一致

现在用户可以轻松地批量上传多个图片文件，系统会提供详细的进度反馈和智能的存储管理！
