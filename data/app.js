// Little Gallery ESP32 前端应用

class LittleGallery {
  constructor() {
    this.currentImages = [];
    this.selectedImages = new Set();
    this.slideshowActive = false;
    this.slideshowInterval = null;
    this.confirmCallback = null;

    // 图片预处理设置
    this.preprocessingSettings = {
      enabled: true,
      targetResolution: "auto",
      quality: 0.5, // 进一步降低质量以减小文件大小
      resizeMode: "contain",
      convertPngToJpg: true, // 自动将PNG转换为JPG
      autoRotate: true, // 自动旋转图片以适应屏幕方向
    };

    this.init();
  }

  init() {
    // 确保DOM完全加载后再设置事件监听器
    if (document.readyState === "loading") {
      document.addEventListener("DOMContentLoaded", () => {
        this.setupEventListeners();
      });
    } else {
      this.setupEventListeners();
    }

    this.refreshImageList();
    this.updateSystemStatus();

    // 加载预处理设置
    this.loadPreprocessingSettings();

    // 验证API端点
    this.verifyAPIEndpoints();

    // 定期刷新
    setInterval(() => this.refreshImageList(), 5000);
    setInterval(() => this.updateSystemStatus(), 30000);
  }

  setupEventListeners() {
    const fileInput = document.getElementById("fileInput");
    const uploadArea = document.getElementById("uploadArea");

    if (!fileInput || !uploadArea) {
      console.error("Required elements for upload not found!");
      return;
    }

    // Use a single, reliable 'change' event listener for file selection
    fileInput.addEventListener("change", (e) => {
      const files = e.target.files;
      if (files.length > 0) {
        this.processFiles(files);
        // Reset the input to allow selecting the same file again
        e.target.value = "";
      }
    });

    // Keep drag and drop functionality
    uploadArea.addEventListener("dragover", (e) => {
      e.preventDefault();
      uploadArea.classList.add("dragover");
    });

    uploadArea.addEventListener("dragleave", (e) => {
      uploadArea.classList.remove("dragover");
    });

    uploadArea.addEventListener("drop", (e) => {
      e.preventDefault();
      uploadArea.classList.remove("dragover");
      const files = e.dataTransfer.files;
      if (files.length > 0) {
        this.handleDroppedFiles(files);
      }
    });
  }

  async handleDroppedFiles(files) {
    // 使用统一的文件处理方法
    await this.processFiles(files);
  }

  async handleUpload(e) {
    console.log("handleUpload called");
    e.preventDefault();

    const fileInput = document.getElementById("fileInput");
    const files = fileInput.files;

    console.log("Files found:", files.length);

    if (files.length === 0) {
      this.showStatus("请选择文件", "error");
      return;
    }

    // 使用统一的文件处理方法
    console.log("Processing files...");
    await this.processFiles(files);

    // 清空文件输入框
    fileInput.value = "";
  }

  async processFiles(files) {
    console.log("processFiles called with", files.length, "files");
    const fileArray = Array.from(files);
    console.log(
      "File details:",
      fileArray.map((f) => f.name)
    );
    // 检查是否有文件被选择
    if (fileArray.length === 0) {
      this.showStatus("没有选择文件", "warning");
      return;
    }

    // 如果是批量上传（多个文件），显示批量上传界面
    if (fileArray.length > 1) {
      await this.processBatchUpload(fileArray);
    } else {
      // 单文件上传，使用简单模式
      await this.processSingleFile(fileArray[0]);
    }
  }

  async processSingleFile(file) {
    console.log("Processing single file:", file.name);
    this.showLoading(true);

    const originalName = file.name;

    if (!this.isValidImageFile(file)) {
      console.log("Invalid file format:", originalName);
      this.showStatus(`文件 ${originalName} 格式不支持`, "warning");
      this.showLoading(false);
      return;
    }

    try {
      // 图片预处理
      let processedFile = file;
      let isPngConverted = false;

      if (this.preprocessingSettings.enabled) {
        console.log("Preprocessing image:", originalName);
        this.showStatus("正在预处理图片...", "info");

        // 检查是否需要PNG转换
        isPngConverted =
          file.type === "image/png" &&
          this.preprocessingSettings.convertPngToJpg;
        if (isPngConverted) {
          this.showStatus("正在将PNG转换为JPG并压缩...", "info");
        }

        processedFile = await this.preprocessImage(file);
        console.log("Image preprocessed, new size:", processedFile.size);
      }

      // 生成安全的文件名（考虑PNG转换）
      const safeFileName = this.generateSafeFileName(
        originalName,
        isPngConverted
      );
      console.log("Safe filename:", safeFileName);

      console.log("Starting upload for:", safeFileName);
      await this.uploadFileWithName(processedFile, safeFileName);
      console.log("Upload successful for:", safeFileName);

      if (safeFileName !== originalName) {
        this.showStatus(
          `文件 "${originalName}" 已预处理并重命名为 "${safeFileName}" 上传成功`,
          "success"
        );
      } else {
        this.showStatus(`文件 ${originalName} 预处理并上传成功`, "success");
      }
    } catch (error) {
      console.error("Upload failed for:", originalName, error);
      this.showStatus(
        `文件 ${originalName} 处理失败: ${error.message}`,
        "error"
      );
    }

    this.showLoading(false);
    this.refreshImageList();
  }

  async processBatchUpload(files) {
    console.log("Starting batch upload for", files.length, "files");

    // 验证文件对象
    console.log("File objects validation:");
    for (let i = 0; i < files.length; i++) {
      const file = files[i];
      console.log(`File ${i + 1}:`, {
        name: file.name,
        type: file.type,
        size: file.size,
        lastModified: file.lastModified,
        isFile: file instanceof File,
        hasSize: "size" in file,
      });
    }

    // 先批量预处理所有文件，得到压缩后文件数组
    let processedFiles = [];
    let preprocessErrors = [];
    for (let i = 0; i < files.length; i++) {
      const file = files[i];
      let processedFile = file;
      let isPngConverted = false;
      if (this.preprocessingSettings.enabled) {
        isPngConverted =
          file.type === "image/png" &&
          this.preprocessingSettings.convertPngToJpg;
        try {
          processedFile = await this.preprocessImage(file);
        } catch (e) {
          preprocessErrors.push({ file: file.name, reason: e.message });
          processedFile = null;
        }
      }
      if (processedFile) {
        processedFiles.push({
          original: file,
          processed: processedFile,
          isPngConverted,
        });
      }
    }

    // 只用压缩后文件估算空间
    const processedFileList = processedFiles.map((f) => f.processed);
    try {
      console.log(
        "About to check storage space for processed files:",
        processedFileList.map((f) => ({ name: f.name, size: f.size }))
      );
      const storageCheck = await this.checkStorageSpace(processedFileList);
      console.log("Storage check result:", storageCheck);

      if (!storageCheck.canUpload) {
        console.log(
          "Storage check failed - cannot upload:",
          storageCheck.message
        );
        this.showStatus(storageCheck.message, "error");
        return;
      }
      if (storageCheck.warning) {
        console.log("Storage check warning:", storageCheck.message);
        this.showStatus(storageCheck.message, "warning");
      }
      console.log("Storage check passed, proceeding with upload...");
    } catch (error) {
      console.warn("Storage check failed:", error);
      // 继续上传，但给出警告
      this.showStatus("无法检查存储空间，请注意空间使用", "warning");
    }

    // 显示批量上传界面
    this.showBatchUploadProgress(true);
    this.batchUploadCancelled = false;

    // 初始化进度
    this.updateBatchProgress(0, processedFileList.length, "准备批量上传...");

    let successCount = 0;
    let failedCount = preprocessErrors.length;
    const results = [
      ...preprocessErrors.map((e) => ({
        file: e.file,
        status: "failed",
        reason: e.reason,
      })),
    ];

    console.log(
      `Starting batch upload loop for ${processedFileList.length} files`
    );

    for (let i = 0; i < processedFiles.length; i++) {
      // 检查是否被取消
      if (this.batchUploadCancelled) {
        console.log("Batch upload cancelled by user");
        break;
      }

      const file = processedFiles[i].original;
      const processedFile = processedFiles[i].processed;
      const originalName = file.name;
      const isPngConverted = processedFiles[i].isPngConverted;

      console.log(
        `Processing file ${i + 1}/${processedFileList.length}:`,
        originalName
      );
      console.log(`File details:`, {
        name: processedFile.name,
        type: processedFile.type,
        size: processedFile.size,
        lastModified: processedFile.lastModified,
      });

      // 更新总体进度
      this.updateBatchProgress(
        i,
        processedFileList.length,
        `正在上传: ${originalName}`
      );

      // 添加文件到进度列表
      const fileId = this.addFileToProgressList(originalName, "uploading");

      // 检查文件格式
      const isValid = this.isValidImageFile(file);
      if (!isValid) {
        console.log("Invalid file format:", originalName);
        this.updateFileProgress(fileId, "error", "格式不支持");
        failedCount++;
        results.push({
          file: originalName,
          status: "failed",
          reason: "格式不支持",
        });
        continue;
      }

      try {
        // 生成安全的文件名（考虑PNG转换）
        const safeFileName = this.generateSafeFileName(
          originalName,
          isPngConverted
        );
        console.log("Safe filename:", safeFileName);

        this.updateFileProgress(fileId, "uploading", "上传中...");
        console.log("Starting upload for:", safeFileName);
        await this.uploadFileWithName(processedFile, safeFileName);
        console.log("Upload successful for:", safeFileName);

        this.updateFileProgress(fileId, "success", "上传成功");
        successCount++;
        results.push({ file: originalName, safeFileName, status: "success" });
      } catch (error) {
        console.error("Upload failed for:", originalName, error);
        this.updateFileProgress(fileId, "error", error.message);
        failedCount++;
        results.push({
          file: originalName,
          status: "failed",
          reason: error.message,
        });
      }
    }

    // 更新最终进度
    const totalProcessed = successCount + failedCount;
    console.log(`Batch upload completed:`, {
      totalFiles: processedFileList.length,
      totalProcessed: totalProcessed,
      successCount: successCount,
      failedCount: failedCount,
      cancelled: this.batchUploadCancelled,
    });

    if (!this.batchUploadCancelled) {
      this.updateBatchProgress(
        processedFileList.length,
        processedFileList.length,
        `批量上传完成: ${successCount} 成功, ${failedCount} 失败`
      );

      // 显示总结
      this.showBatchUploadSummary(successCount, failedCount, results);
    } else {
      this.updateBatchProgress(0, processedFileList.length, "批量上传已取消");
    }

    // 刷新图片列表
    this.refreshImageList();

    // 3秒后隐藏批量上传界面
    setTimeout(() => {
      this.showBatchUploadProgress(false);
    }, 3000);
  }

  async uploadFile(file) {
    return this.uploadFileWithName(file, file.name);
  }

  async uploadFileWithName(file, fileName) {
    console.log("uploadFileWithName called:", fileName);
    return new Promise((resolve, reject) => {
      const formData = new FormData();

      // 创建一个新的File对象，使用自定义文件名
      const renamedFile = new File([file], fileName, { type: file.type });
      formData.append("file", renamedFile);
      console.log("FormData prepared, file size:", file.size);

      const xhr = new XMLHttpRequest();

      xhr.upload.addEventListener("progress", (e) => {
        if (e.lengthComputable) {
          const percentComplete = (e.loaded / e.total) * 100;
          console.log("Upload progress:", percentComplete.toFixed(1) + "%");
          this.updateProgress(percentComplete);
        }
      });

      xhr.addEventListener("load", () => {
        console.log("Upload completed, status:", xhr.status);
        console.log("Response:", xhr.responseText);
        if (xhr.status === 200) {
          resolve();
        } else {
          reject(new Error(`HTTP ${xhr.status}: ${xhr.responseText}`));
        }
      });

      xhr.addEventListener("error", () => {
        console.error("Upload error occurred");
        reject(new Error("网络错误"));
      });

      console.log("Starting upload to /upload");
      xhr.open("POST", "/upload");
      xhr.send(formData);
    });
  }

  isValidImageFile(file) {
    // 支持的输入格式：JPEG、BMP、PNG（PNG会自动转换为JPG）
    const validTypes = ["image/jpeg", "image/jpg", "image/bmp", "image/png"];
    const validExtensions = [".jpg", ".jpeg", ".bmp", ".png"];

    const isValidType = validTypes.includes(file.type);
    const isValidExtension = validExtensions.some((ext) =>
      file.name.toLowerCase().endsWith(ext)
    );

    console.log(`File validation for ${file.name}:`, {
      fileType: file.type,
      isValidType: isValidType,
      isValidExtension: isValidExtension,
      result: isValidType || isValidExtension,
      willConvertPng:
        file.type === "image/png" && this.preprocessingSettings.convertPngToJpg,
    });

    return isValidType || isValidExtension;
  }

  generateSafeFileName(originalName, isPngConverted = false) {
    // 获取文件扩展名
    const lastDotIndex = originalName.lastIndexOf(".");
    let extension =
      lastDotIndex > -1 ? originalName.substring(lastDotIndex) : "";
    const nameWithoutExt =
      lastDotIndex > -1
        ? originalName.substring(0, lastDotIndex)
        : originalName;

    // 如果PNG被转换为JPG，更改扩展名
    if (isPngConverted && extension.toLowerCase() === ".png") {
      extension = ".jpg";
    }

    // 清理文件名：只保留字母、数字、下划线和连字符
    let cleanName = nameWithoutExt
      .replace(/[^a-zA-Z0-9_-]/g, "_") // 替换特殊字符为下划线
      .replace(/_+/g, "_") // 合并多个下划线
      .replace(/^_|_$/g, ""); // 移除开头和结尾的下划线

    // 如果清理后的名称为空，使用默认名称
    if (!cleanName) {
      cleanName = "image";
    }

    // 计算最大允许的名称长度（总长度限制 - 扩展名长度 - 时间戳长度）
    const maxNameLength = 25 - extension.length - 8; // 8位时间戳

    // 如果名称太长，截断并添加时间戳
    if (cleanName.length > maxNameLength) {
      cleanName = cleanName.substring(0, maxNameLength);
    }

    // 添加时间戳确保唯一性
    const timestamp = Date.now().toString().slice(-6); // 取最后6位时间戳
    const finalName = `${cleanName}_${timestamp}${extension}`;

    // 确保最终文件名不超过25个字符
    if (finalName.length > 25) {
      const availableLength = 25 - extension.length - 7; // 7 = '_' + 6位时间戳
      const truncatedName = cleanName.substring(
        0,
        Math.max(1, availableLength)
      );
      return `${truncatedName}_${timestamp}${extension}`;
    }

    return finalName;
  }

  async refreshImageList() {
    try {
      const response = await fetch("/api/images");
      const data = await response.json();

      this.currentImages = data.images || [];
      this.updateImageList(data);
      this.updateCurrentImageInfo(data);
    } catch (error) {
      console.error("获取图片列表失败:", error);
      this.showStatus("获取图片列表失败", "error");
    }
  }

  updateImageList(data) {
    const container = document.getElementById("imageListContainer");
    const imageCount = document.getElementById("imageCount");

    imageCount.textContent = `共 ${data.total || 0} 张图片`;

    if (!data.images || data.images.length === 0) {
      container.innerHTML = `
                <div class="empty-state">
                    <p>📷 还没有图片</p>
                    <p>请上传一些图片开始使用</p>
                </div>
            `;
      return;
    }

    container.innerHTML = data.images
      .map(
        (image, index) => `
            <div class="image-item ${
              index === data.current ? "current" : ""
            }" data-index="${index}">
                <div class="image-info">
                    <input type="checkbox" class="image-checkbox" data-image="${image}">
                    <span class="image-name">${image}</span>
                    ${
                      index === data.current
                        ? '<span class="image-badge">当前</span>'
                        : ""
                    }
                </div>
                <div class="image-actions">
                    <button class="action-btn" onclick="gallery.setCurrentImage(${index})">选择</button>
                    <button class="action-btn delete" onclick="gallery.deleteImage('${image}')">删除</button>
                </div>
            </div>
        `
      )
      .join("");

    // 更新选中状态
    this.updateSelectedImages();
  }

  updateCurrentImageInfo(data) {
    const info = document.getElementById("currentImageInfo");

    if (data.total === 0) {
      info.innerHTML = "<p>📷 没有图片</p>";
    } else {
      const currentName =
        data.current >= 0 && data.current < data.images.length
          ? data.images[data.current]
          : "无";

      info.innerHTML = `
                <div class="current-info">
                    <strong>当前图片:</strong> ${currentName}<br>
                    <strong>位置:</strong> ${data.current + 1} / ${data.total}
                </div>
            `;
    }
  }

  async nextImage() {
    try {
      const response = await fetch("/api/next", { method: "POST" });
      const data = await response.json();

      if (data.status === "ok") {
        this.refreshImageList();
        this.showStatus("切换到下一张图片", "success");
      } else {
        this.showStatus(data.message || "切换失败", "error");
      }
    } catch (error) {
      this.showStatus("切换图片失败", "error");
    }
  }

  async previousImage() {
    try {
      const response = await fetch("/api/previous", { method: "POST" });
      const data = await response.json();

      if (data.status === "ok") {
        this.refreshImageList();
        this.showStatus("切换到上一张图片", "success");
      } else {
        this.showStatus(data.message || "切换失败", "error");
      }
    } catch (error) {
      this.showStatus("切换图片失败", "error");
    }
  }

  async setCurrentImage(index) {
    try {
      const response = await fetch("/api/setimage", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `index=${index}`,
      });

      const data = await response.json();

      if (data.status === "ok") {
        this.refreshImageList();
        this.showStatus(`切换到图片: ${data.current}`, "success");
      } else {
        this.showStatus(data.message || "切换失败", "error");
      }
    } catch (error) {
      this.showStatus("切换图片失败", "error");
    }
  }

  deleteImage(filename) {
    this.showConfirm(`确定要删除图片 "${filename}" 吗？此操作不可撤销。`, () =>
      this.performDeleteImage(filename)
    );
  }

  async performDeleteImage(filename) {
    try {
      const response = await fetch("/api/delete", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `filename=${encodeURIComponent(filename)}`,
      });

      const data = await response.json();

      if (data.status === "ok") {
        this.refreshImageList();
        this.showStatus(`图片 "${filename}" 已删除`, "success");
      } else {
        this.showStatus(data.message || "删除失败", "error");
      }
    } catch (error) {
      this.showStatus("删除图片失败", "error");
    }
  }

  selectAllImages() {
    const checkboxes = document.querySelectorAll(".image-checkbox");
    const allSelected = Array.from(checkboxes).every((cb) => cb.checked);

    checkboxes.forEach((cb) => {
      cb.checked = !allSelected;
      if (cb.checked) {
        this.selectedImages.add(cb.dataset.image);
      } else {
        this.selectedImages.delete(cb.dataset.image);
      }
    });

    this.updateSelectedImages();
  }

  deleteSelectedImages() {
    if (this.selectedImages.size === 0) {
      this.showStatus("请先选择要删除的图片", "warning");
      return;
    }

    const imageList = Array.from(this.selectedImages).join('", "');
    this.showConfirm(
      `确定要删除选中的 ${this.selectedImages.size} 张图片吗？\n"${imageList}"\n此操作不可撤销。`,
      () => this.performDeleteSelectedImages()
    );
  }

  async performDeleteSelectedImages() {
    const images = Array.from(this.selectedImages);
    let successCount = 0;

    this.showLoading(true);

    for (const image of images) {
      try {
        await this.performDeleteImage(image);
        successCount++;
      } catch (error) {
        console.error(`删除 ${image} 失败:`, error);
      }
    }

    this.selectedImages.clear();
    this.showLoading(false);
    this.refreshImageList();

    this.showStatus(
      `成功删除 ${successCount}/${images.length} 张图片`,
      "success"
    );
  }

  updateSelectedImages() {
    const checkboxes = document.querySelectorAll(".image-checkbox");
    checkboxes.forEach((cb) => {
      const item = cb.closest(".image-item");
      if (cb.checked) {
        item.classList.add("selected");
        this.selectedImages.add(cb.dataset.image);
      } else {
        item.classList.remove("selected");
        this.selectedImages.delete(cb.dataset.image);
      }
    });
  }

  toggleSlideshow() {
    const btn = document.getElementById("slideshowBtn");

    if (this.slideshowActive) {
      this.stopSlideshow();
      btn.textContent = "▶️ 幻灯片";
      btn.style.background = "#007bff";
    } else {
      this.startSlideshow();
      btn.textContent = "⏸️ 停止";
      btn.style.background = "#dc3545";
    }
  }

  startSlideshow() {
    if (this.currentImages.length === 0) {
      this.showStatus("没有图片可以播放", "warning");
      return;
    }

    this.slideshowActive = true;
    this.slideshowInterval = setInterval(() => {
      this.nextImage();
    }, 3000); // 3秒切换一次

    this.showStatus("幻灯片已开始", "success");
  }

  stopSlideshow() {
    this.slideshowActive = false;
    if (this.slideshowInterval) {
      clearInterval(this.slideshowInterval);
      this.slideshowInterval = null;
    }
    this.showStatus("幻灯片已停止", "success");
  }

  async updateSystemStatus() {
    try {
      const response = await fetch("/api/status");
      const data = await response.json();

      document.getElementById("wifiStatus").textContent = data.wifi
        ? "已连接"
        : "未连接";
      document.getElementById("ipAddress").textContent = data.ip || "未知";
      document.getElementById("storageInfo").textContent =
        data.storage || "计算中...";
      document.getElementById("uptime").textContent =
        data.uptime || "计算中...";
    } catch (error) {
      console.error("获取系统状态失败:", error);
    }
  }

  showStatus(message, type = "success") {
    const status = document.getElementById("uploadStatus");
    status.textContent = message;
    status.className = `status-message ${type}`;
    status.style.display = "block";

    setTimeout(() => {
      status.style.display = "none";
    }, 3000);
  }

  updateProgress(percent) {
    const progressBar = document.getElementById("uploadProgress");
    const progressFill = progressBar.querySelector(".progress-fill");

    progressBar.style.display = "block";
    progressFill.style.width = `${percent}%`;

    if (percent >= 100) {
      setTimeout(() => {
        progressBar.style.display = "none";
      }, 1000);
    }
  }

  showLoading(show) {
    const loading = document.getElementById("loadingIndicator");
    loading.style.display = show ? "flex" : "none";
  }

  showConfirm(message, callback) {
    const dialog = document.getElementById("confirmDialog");
    const messageEl = document.getElementById("confirmMessage");

    messageEl.textContent = message;
    dialog.style.display = "flex";
    this.confirmCallback = callback;
  }

  closeConfirmDialog() {
    const dialog = document.getElementById("confirmDialog");
    dialog.style.display = "none";
    this.confirmCallback = null;
  }

  confirmAction() {
    if (this.confirmCallback) {
      this.confirmCallback();
    }
    this.closeConfirmDialog();
  }

  // ==================== 批量上传相关方法 ====================

  showBatchUploadProgress(show) {
    const progressDiv = document.getElementById("batchUploadProgress");
    if (progressDiv) {
      progressDiv.style.display = show ? "block" : "none";
      if (show) {
        // 清空之前的进度列表
        const fileList = document.getElementById("fileProgressList");
        if (fileList) {
          fileList.innerHTML = "";
        }
      }
    }
  }

  updateBatchProgress(current, total, message) {
    const progressText = document.getElementById("uploadProgressText");
    const progressCount = document.getElementById("uploadProgressCount");
    const progressBar = document.getElementById("overallProgressBar");

    if (progressText) {
      progressText.textContent = message;
    }

    if (progressCount) {
      progressCount.textContent = `${current}/${total}`;
    }

    if (progressBar) {
      const percentage = total > 0 ? (current / total) * 100 : 0;
      progressBar.style.width = `${percentage}%`;
    }
  }

  addFileToProgressList(fileName, status) {
    const fileList = document.getElementById("fileProgressList");
    if (!fileList) return null;

    const fileId = `file_${Date.now()}_${Math.random()
      .toString(36)
      .substr(2, 9)}`;
    const fileItem = document.createElement("div");
    fileItem.className = "file-progress-item";
    fileItem.id = fileId;

    fileItem.innerHTML = `
      <div class="file-info">
        <span class="file-name">${fileName}</span>
        <span class="file-status ${status}">${this.getStatusText(status)}</span>
      </div>
      <div class="file-progress-bar">
        <div class="file-progress-fill"></div>
      </div>
    `;

    fileList.appendChild(fileItem);
    return fileId;
  }

  updateFileProgress(fileId, status, message) {
    const fileItem = document.getElementById(fileId);
    if (!fileItem) return;

    const statusEl = fileItem.querySelector(".file-status");
    const progressFill = fileItem.querySelector(".file-progress-fill");

    if (statusEl) {
      statusEl.className = `file-status ${status}`;
      statusEl.textContent = message || this.getStatusText(status);
    }

    if (progressFill) {
      if (status === "success") {
        progressFill.style.width = "100%";
        progressFill.style.backgroundColor = "#28a745";
      } else if (status === "error") {
        progressFill.style.width = "100%";
        progressFill.style.backgroundColor = "#dc3545";
      } else if (status === "uploading") {
        progressFill.style.width = "50%";
        progressFill.style.backgroundColor = "#007bff";
      }
    }
  }

  getStatusText(status) {
    switch (status) {
      case "uploading":
        return "上传中...";
      case "success":
        return "上传成功";
      case "error":
        return "上传失败";
      default:
        return "等待中";
    }
  }

  showBatchUploadSummary(successCount, failedCount, results) {
    const totalCount = successCount + failedCount;
    let message = `批量上传完成！\n`;
    message += `总计: ${totalCount} 个文件\n`;
    message += `成功: ${successCount} 个\n`;
    message += `失败: ${failedCount} 个`;

    if (failedCount > 0) {
      message += `\n\n失败的文件:\n`;
      results
        .filter((r) => r.status === "failed")
        .forEach((r) => {
          message += `• ${r.file}: ${r.reason}\n`;
        });
    }

    this.showStatus(
      `批量上传完成: ${successCount} 成功, ${failedCount} 失败`,
      failedCount > 0 ? "warning" : "success"
    );

    console.log("Batch upload summary:", message);
  }

  async checkStorageSpace(files) {
    try {
      console.log("Checking storage space via /api/upload-status...");
      const response = await fetch("/api/upload-status");

      console.log("Response status:", response.status, response.statusText);

      if (!response.ok) {
        throw new Error(
          `API request failed: ${response.status} ${response.statusText}`
        );
      }

      const responseText = await response.text();
      console.log("Raw response:", responseText);

      let status;
      try {
        status = JSON.parse(responseText);
      } catch (parseError) {
        console.error("JSON parse error:", parseError);
        console.error("Response text:", responseText);
        throw new Error(
          `Invalid JSON response: ${responseText.substring(0, 100)}`
        );
      }

      // 估算文件总大小
      let totalSize = 0;
      console.log("Calculating total file size...");
      for (let i = 0; i < files.length; i++) {
        const file = files[i];
        console.log(`File ${i + 1}: ${file.name}, size: ${file.size} bytes`);
        totalSize += file.size || 0; // 防止undefined
      }
      console.log(`Total calculated size: ${totalSize} bytes`);

      const availableSpace = status.available_storage;
      const storageUsedPercent = status.storage_used_percent;

      console.log("Storage check:", {
        totalFileSize: totalSize,
        availableSpace: availableSpace,
        storageUsedPercent: storageUsedPercent,
      });

      // 检查是否有足够空间
      if (totalSize > availableSpace) {
        return {
          canUpload: false,
          message: `存储空间不足！需要 ${(totalSize / 1024 / 1024).toFixed(
            1
          )}MB，可用 ${(availableSpace / 1024 / 1024).toFixed(1)}MB`,
        };
      }

      // 检查存储空间使用率
      if (storageUsedPercent > 90) {
        return {
          canUpload: false,
          message: "存储空间已满（>90%），请删除一些图片后再上传",
        };
      }

      if (storageUsedPercent > 75) {
        return {
          canUpload: true,
          warning: true,
          message: `存储空间较少（${storageUsedPercent.toFixed(
            1
          )}%），建议适量上传`,
        };
      }

      return {
        canUpload: true,
        message: `存储空间充足，可上传 ${files.length} 个文件`,
      };
    } catch (error) {
      console.error("Storage check failed:", error);
      throw error;
    }
  }

  // ==================== 图片预处理功能 ====================

  async preprocessImage(file) {
    return new Promise((resolve, reject) => {
      const img = new Image();
      const canvas = document.createElement("canvas");
      const ctx = canvas.getContext("2d");

      img.onload = () => {
        try {
          // 获取目标尺寸
          const targetDimensions = this.calculateTargetDimensions(
            img.width,
            img.height
          );

          console.log(
            `Original: ${img.width}x${img.height}, Target: ${targetDimensions.width}x${targetDimensions.height}`
          );

          // 设置canvas尺寸
          canvas.width = targetDimensions.width;
          canvas.height = targetDimensions.height;

          // 根据缩放模式绘制图片
          this.drawImageWithMode(ctx, img, targetDimensions);

          // 转换为Blob - 始终输出为JPEG格式以获得最佳压缩
          canvas.toBlob(
            (blob) => {
              if (blob) {
                // 创建新的File对象，始终使用JPEG格式
                let newFileName = file.name;

                // 如果原文件是PNG，更改文件名扩展名
                if (
                  file.type === "image/png" &&
                  this.preprocessingSettings.convertPngToJpg
                ) {
                  newFileName = file.name.replace(/\.png$/i, ".jpg");
                }

                const processedFile = new File([blob], newFileName, {
                  type: "image/jpeg",
                  lastModified: Date.now(),
                });

                const compressionRatio = (
                  ((file.size - processedFile.size) / file.size) *
                  100
                ).toFixed(1);
                console.log(
                  `Image processed: ${file.size} -> ${processedFile.size} bytes (${compressionRatio}% reduction)`
                );
                resolve(processedFile);
              } else {
                reject(new Error("图片处理失败"));
              }
            },
            "image/jpeg",
            this.preprocessingSettings.quality
          );
        } catch (error) {
          reject(new Error("图片处理过程中出错: " + error.message));
        }
      };

      img.onerror = () => {
        reject(new Error("无法加载图片"));
      };

      // 加载图片
      img.src = URL.createObjectURL(file);
    });
  }

  calculateTargetDimensions(originalWidth, originalHeight) {
    let targetWidth, targetHeight;
    let shouldRotate = false;

    // 根据设置确定目标分辨率
    if (this.preprocessingSettings.targetResolution === "auto") {
      // 自动选择最佳分辨率，保持ESP32显示屏的原始分辨率
      const aspectRatio = originalWidth / originalHeight;

      if (this.preprocessingSettings.autoRotate) {
        // 智能旋转模式：根据图片内容选择最佳方向
        if (aspectRatio > 1.0) {
          // 横屏图片：宽度大于高度
          targetWidth = 320;
          targetHeight = 240;
          shouldRotate = false;
        } else {
          // 竖屏图片：高度大于等于宽度
          // 检查是否需要旋转以更好地利用屏幕空间
          const horizontalFit = Math.min(320 / originalWidth, 240 / originalHeight);
          const verticalFit = Math.min(240 / originalWidth, 320 / originalHeight);

          if (horizontalFit > verticalFit) {
            // 旋转后能更好地利用屏幕空间
            targetWidth = 320;
            targetHeight = 240;
            shouldRotate = true;
          } else {
            // 保持竖屏方向
            targetWidth = 240;
            targetHeight = 320;
            shouldRotate = false;
          }
        }
      } else {
        // 传统模式：根据宽高比选择方向
        if (aspectRatio > 1.2) {
          // 明显的横屏图片
          targetWidth = 320;
          targetHeight = 240;
          shouldRotate = false;
        } else if (aspectRatio < 0.8) {
          // 明显的竖屏图片
          targetWidth = 240;
          targetHeight = 320;
          shouldRotate = false;
        } else {
          // 正方形或接近正方形的图片，默认选择横屏
          targetWidth = 320;
          targetHeight = 240;
          shouldRotate = false;
        }
      }
    } else {
      // 使用指定分辨率
      const [w, h] = this.preprocessingSettings.targetResolution
        .split("x")
        .map(Number);
      targetWidth = w;
      targetHeight = h;

      // 检查是否需要旋转以更好地适应目标分辨率
      if (this.preprocessingSettings.autoRotate) {
        const originalAspectRatio = originalWidth / originalHeight;
        const targetAspectRatio = targetWidth / targetHeight;

        // 如果原图和目标的宽高比差异很大，考虑旋转
        if (
          (originalAspectRatio > 1 && targetAspectRatio < 1) ||
          (originalAspectRatio < 1 && targetAspectRatio > 1)
        ) {
          shouldRotate = true;
        }
      }
    }

    return {
      width: targetWidth,
      height: targetHeight,
      shouldRotate: shouldRotate,
    };
  }

  drawImageWithMode(ctx, img, targetDimensions) {
    const {
      width: targetWidth,
      height: targetHeight,
      shouldRotate,
    } = targetDimensions;
    let originalWidth = img.width;
    let originalHeight = img.height;

    // 清空canvas
    ctx.fillStyle = "#000000";
    ctx.fillRect(0, 0, targetWidth, targetHeight);

    // 如果需要旋转，交换原始图片的宽高用于计算
    if (shouldRotate) {
      [originalWidth, originalHeight] = [originalHeight, originalWidth];
    }

    let drawX, drawY, drawWidth, drawHeight;

    switch (this.preprocessingSettings.resizeMode) {
      case "contain":
        // 完整显示，保持比例，可能有黑边
        const scaleContain = Math.min(
          targetWidth / originalWidth,
          targetHeight / originalHeight
        );
        drawWidth = originalWidth * scaleContain;
        drawHeight = originalHeight * scaleContain;
        drawX = (targetWidth - drawWidth) / 2;
        drawY = (targetHeight - drawHeight) / 2;
        break;

      case "cover":
        // 填满屏幕，保持比例，可能裁剪
        const scaleCover = Math.max(
          targetWidth / originalWidth,
          targetHeight / originalHeight
        );
        drawWidth = originalWidth * scaleCover;
        drawHeight = originalHeight * scaleCover;
        drawX = (targetWidth - drawWidth) / 2;
        drawY = (targetHeight - drawHeight) / 2;
        break;

      case "stretch":
        // 拉伸适配，可能变形
        drawX = 0;
        drawY = 0;
        drawWidth = targetWidth;
        drawHeight = targetHeight;
        break;

      default:
        // 默认使用contain模式
        const scaleDefault = Math.min(
          targetWidth / originalWidth,
          targetHeight / originalHeight
        );
        drawWidth = originalWidth * scaleDefault;
        drawHeight = originalHeight * scaleDefault;
        drawX = (targetWidth - drawWidth) / 2;
        drawY = (targetHeight - drawHeight) / 2;
    }

    // 绘制图片
    if (shouldRotate) {
      // 保存当前状态
      ctx.save();

      // 移动到画布中心
      ctx.translate(targetWidth / 2, targetHeight / 2);

      // 旋转90度（顺时针）
      ctx.rotate(Math.PI / 2);

      // 调整绘制位置（因为旋转后坐标系改变）
      const rotatedDrawX = -drawHeight / 2;
      const rotatedDrawY = -drawWidth / 2;

      // 绘制旋转后的图片（注意宽高交换）
      ctx.drawImage(img, rotatedDrawX, rotatedDrawY, drawHeight, drawWidth);

      // 恢复状态
      ctx.restore();
    } else {
      // 正常绘制
      ctx.drawImage(img, drawX, drawY, drawWidth, drawHeight);
    }
  }

  updatePreprocessingSettings() {
    const enabledCheckbox = document.getElementById("enablePreprocessing");
    const resolutionSelect = document.getElementById("targetResolution");
    const qualitySlider = document.getElementById("imageQuality");
    const qualityValue = document.getElementById("qualityValue");
    const resizeModeSelect = document.getElementById("resizeMode");
    const convertPngCheckbox = document.getElementById("convertPngToJpg");
    const autoRotateCheckbox = document.getElementById("autoRotate");

    if (enabledCheckbox) {
      this.preprocessingSettings.enabled = enabledCheckbox.checked;
    }

    if (resolutionSelect) {
      this.preprocessingSettings.targetResolution = resolutionSelect.value;
    }

    if (qualitySlider) {
      this.preprocessingSettings.quality = parseFloat(qualitySlider.value);
      if (qualityValue) {
        qualityValue.textContent =
          Math.round(this.preprocessingSettings.quality * 100) + "%";
      }
    }

    if (resizeModeSelect) {
      this.preprocessingSettings.resizeMode = resizeModeSelect.value;
    }

    if (convertPngCheckbox) {
      this.preprocessingSettings.convertPngToJpg = convertPngCheckbox.checked;
    }

    if (autoRotateCheckbox) {
      this.preprocessingSettings.autoRotate = autoRotateCheckbox.checked;
    }

    console.log("Preprocessing settings updated:", this.preprocessingSettings);

    // 更新状态指示器
    this.updatePreprocessingStatusIndicator();

    // 保存设置到localStorage
    localStorage.setItem(
      "preprocessingSettings",
      JSON.stringify(this.preprocessingSettings)
    );
  }

  loadPreprocessingSettings() {
    try {
      const saved = localStorage.getItem("preprocessingSettings");
      if (saved) {
        this.preprocessingSettings = {
          ...this.preprocessingSettings,
          ...JSON.parse(saved),
        };

        // 更新UI
        const enabledCheckbox = document.getElementById("enablePreprocessing");
        const resolutionSelect = document.getElementById("targetResolution");
        const qualitySlider = document.getElementById("imageQuality");
        const qualityValue = document.getElementById("qualityValue");
        const resizeModeSelect = document.getElementById("resizeMode");
        const convertPngCheckbox = document.getElementById("convertPngToJpg");
        const autoRotateCheckbox = document.getElementById("autoRotate");

        if (enabledCheckbox)
          enabledCheckbox.checked = this.preprocessingSettings.enabled;
        if (resolutionSelect)
          resolutionSelect.value = this.preprocessingSettings.targetResolution;
        if (qualitySlider)
          qualitySlider.value = this.preprocessingSettings.quality;
        if (qualityValue)
          qualityValue.textContent =
            Math.round(this.preprocessingSettings.quality * 100) + "%";
        if (resizeModeSelect)
          resizeModeSelect.value = this.preprocessingSettings.resizeMode;
        if (convertPngCheckbox)
          convertPngCheckbox.checked =
            this.preprocessingSettings.convertPngToJpg;
        if (autoRotateCheckbox)
          autoRotateCheckbox.checked = this.preprocessingSettings.autoRotate;

        // 更新状态指示器
        this.updatePreprocessingStatusIndicator();
      }
    } catch (error) {
      console.warn("Failed to load preprocessing settings:", error);
    }
  }

  updatePreprocessingStatusIndicator() {
    const statusIndicator = document.getElementById("preprocessingStatus");
    if (statusIndicator) {
      if (this.preprocessingSettings.enabled) {
        statusIndicator.textContent = "已启用";
        statusIndicator.className = "preprocessing-indicator enabled";
      } else {
        statusIndicator.textContent = "已禁用";
        statusIndicator.className = "preprocessing-indicator disabled";
      }
    }
  }

  async verifyAPIEndpoints() {
    console.log("Verifying API endpoints...");

    const endpoints = [
      { url: "/api/upload-status", name: "Upload Status" },
      { url: "/api/orientation", name: "Orientation Settings" },
      { url: "/api/images", name: "Image List" },
    ];

    for (const endpoint of endpoints) {
      try {
        console.log(`Testing ${endpoint.name}: ${endpoint.url}`);
        const response = await fetch(endpoint.url);

        if (response.ok) {
          console.log(`✅ ${endpoint.name} API working`);
        } else {
          console.warn(
            `⚠️ ${endpoint.name} API returned ${response.status}: ${response.statusText}`
          );
        }
      } catch (error) {
        console.error(`❌ ${endpoint.name} API failed:`, error.message);
      }
    }
  }
}

// 全局实例
const gallery = new LittleGallery();

// 全局函数（保持向后兼容）
function nextImage() {
  gallery.nextImage();
}

function previousImage() {
  gallery.previousImage();
}

function refreshImageList() {
  gallery.refreshImageList();
}

function toggleSlideshow() {
  gallery.toggleSlideshow();
}

function selectAllImages() {
  gallery.selectAllImages();
}

function deleteSelectedImages() {
  gallery.deleteSelectedImages();
}

function closeConfirmDialog() {
  gallery.closeConfirmDialog();
}

function confirmAction() {
  gallery.confirmAction();
}

// 颜色测试函数
async function testDisplayColors() {
  console.log("Testing display colors...");

  try {
    const response = await fetch("/api/test-colors");
    const result = await response.json();

    if (result.status === "success") {
      gallery.showStatus("颜色测试完成！请查看显示屏", "success");
      console.log("Color test result:", result);
    } else {
      gallery.showStatus("颜色测试失败", "error");
    }
  } catch (error) {
    console.error("Color test failed:", error);
    gallery.showStatus("颜色测试请求失败: " + error.message, "error");
  }
}

// 方向控制函数
async function updateOrientationMode() {
  const mode = document.getElementById("orientationMode").value;
  console.log("Updating orientation mode to:", mode);

  try {
    const formData = new FormData();
    formData.append("mode", mode);

    const response = await fetch("/api/orientation", {
      method: "POST",
      body: formData,
    });

    const result = await response.json();
    if (result.status === "success") {
      gallery.showStatus(`显示模式已设置为: ${mode}`, "success");
    } else {
      gallery.showStatus("设置显示模式失败", "error");
    }
  } catch (error) {
    console.error("Update orientation mode failed:", error);
    gallery.showStatus("设置显示模式请求失败: " + error.message, "error");
  }
}

async function updateAutoRotation() {
  const enabled = document.getElementById("autoRotation").checked;
  console.log("Updating auto rotation to:", enabled);

  try {
    const formData = new FormData();
    formData.append("auto_rotation", enabled.toString());

    const response = await fetch("/api/orientation", {
      method: "POST",
      body: formData,
    });

    const result = await response.json();
    if (result.status === "success") {
      gallery.showStatus(`自动旋转已${enabled ? "启用" : "禁用"}`, "success");
    } else {
      gallery.showStatus("设置自动旋转失败", "error");
    }
  } catch (error) {
    console.error("Update auto rotation failed:", error);
    gallery.showStatus("设置自动旋转请求失败: " + error.message, "error");
  }
}

async function testOrientation() {
  console.log("Testing orientation...");
  gallery.showStatus("正在测试方向设置...", "info");

  // 可以触发显示当前图片来测试方向
  if (gallery.currentImages.length > 0) {
    const currentImage = gallery.currentImages[0]; // 使用第一张图片
    try {
      const response = await fetch(
        `/api/set-image?filename=${encodeURIComponent(currentImage)}`
      );
      if (response.ok) {
        gallery.showStatus("方向测试完成！请查看显示屏", "success");
      } else {
        gallery.showStatus("方向测试失败", "error");
      }
    } catch (error) {
      gallery.showStatus("方向测试请求失败: " + error.message, "error");
    }
  } else {
    gallery.showStatus("没有图片可供测试", "warning");
  }
}

// 批量上传控制函数
function cancelBatchUpload() {
  console.log("Cancelling batch upload...");
  gallery.batchUploadCancelled = true;
  gallery.showStatus("正在取消批量上传...", "warning");
}

// 图片预处理控制函数
function updatePreprocessingSettings() {
  gallery.updatePreprocessingSettings();
}

async function testImageProcessing() {
  console.log("Testing image processing...");

  if (gallery.currentImages.length > 0) {
    gallery.showStatus("正在测试图片预处理功能...", "info");

    // 创建一个测试用的文件输入
    const input = document.createElement("input");
    input.type = "file";
    input.accept = ".jpg,.jpeg,.bmp";
    input.onchange = async (e) => {
      const file = e.target.files[0];
      if (file) {
        try {
          gallery.showStatus("正在预处理测试图片...", "info");
          const processedFile = await gallery.preprocessImage(file);
          gallery.showStatus(
            `预处理测试完成！原始: ${(file.size / 1024).toFixed(
              1
            )}KB → 处理后: ${(processedFile.size / 1024).toFixed(1)}KB`,
            "success"
          );
        } catch (error) {
          gallery.showStatus("预处理测试失败: " + error.message, "error");
        }
      }
    };
    input.click();
  } else {
    gallery.showStatus("请先选择一个测试图片", "warning");
  }
}
