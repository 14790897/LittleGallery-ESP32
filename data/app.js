// Little Gallery ESP32 前端应用

class LittleGallery {
    constructor() {
        this.currentImages = [];
        this.selectedImages = new Set();
        this.slideshowActive = false;
        this.slideshowInterval = null;
        this.confirmCallback = null;
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.refreshImageList();
        this.updateSystemStatus();
        
        // 定期刷新
        setInterval(() => this.refreshImageList(), 5000);
        setInterval(() => this.updateSystemStatus(), 30000);
    }
    
    setupEventListeners() {
        // 文件上传
        const uploadForm = document.getElementById('uploadForm');
        const fileInput = document.getElementById('fileInput');
        const uploadArea = document.getElementById('uploadArea');
        
        uploadForm.addEventListener('submit', (e) => this.handleUpload(e));
        
        // 拖拽上传
        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.classList.add('dragover');
        });
        
        uploadArea.addEventListener('dragleave', () => {
            uploadArea.classList.remove('dragover');
        });
        
        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.classList.remove('dragover');
            const files = e.dataTransfer.files;
            if (files.length > 0) {
                fileInput.files = files;
                this.handleUpload(e);
            }
        });
    }
    
    async handleUpload(e) {
        e.preventDefault();
        
        const fileInput = document.getElementById('fileInput');
        const files = fileInput.files;
        
        if (files.length === 0) {
            this.showStatus('请选择文件', 'error');
            return;
        }
        
        this.showLoading(true);
        
        for (let i = 0; i < files.length; i++) {
            const file = files[i];
            
            if (!this.isValidImageFile(file)) {
                this.showStatus(`文件 ${file.name} 格式不支持`, 'warning');
                continue;
            }
            
            try {
                await this.uploadFile(file);
                this.showStatus(`文件 ${file.name} 上传成功`, 'success');
            } catch (error) {
                this.showStatus(`文件 ${file.name} 上传失败: ${error.message}`, 'error');
            }
        }
        
        fileInput.value = '';
        this.showLoading(false);
        this.refreshImageList();
    }
    
    async uploadFile(file) {
        return new Promise((resolve, reject) => {
            const formData = new FormData();
            formData.append('file', file);
            
            const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', (e) => {
                if (e.lengthComputable) {
                    const percentComplete = (e.loaded / e.total) * 100;
                    this.updateProgress(percentComplete);
                }
            });
            
            xhr.addEventListener('load', () => {
                if (xhr.status === 200) {
                    resolve();
                } else {
                    reject(new Error(`HTTP ${xhr.status}`));
                }
            });
            
            xhr.addEventListener('error', () => {
                reject(new Error('网络错误'));
            });
            
            xhr.open('POST', '/upload');
            xhr.send(formData);
        });
    }
    
    isValidImageFile(file) {
        const validTypes = ['image/jpeg', 'image/jpg', 'image/bmp'];
        const validExtensions = ['.jpg', '.jpeg', '.bmp'];
        
        return validTypes.includes(file.type) || 
               validExtensions.some(ext => file.name.toLowerCase().endsWith(ext));
    }
    
    async refreshImageList() {
        try {
            const response = await fetch('/api/images');
            const data = await response.json();
            
            this.currentImages = data.images || [];
            this.updateImageList(data);
            this.updateCurrentImageInfo(data);
            
        } catch (error) {
            console.error('获取图片列表失败:', error);
            this.showStatus('获取图片列表失败', 'error');
        }
    }
    
    updateImageList(data) {
        const container = document.getElementById('imageListContainer');
        const imageCount = document.getElementById('imageCount');
        
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
        
        container.innerHTML = data.images.map((image, index) => `
            <div class="image-item ${index === data.current ? 'current' : ''}" data-index="${index}">
                <div class="image-info">
                    <input type="checkbox" class="image-checkbox" data-image="${image}">
                    <span class="image-name">${image}</span>
                    ${index === data.current ? '<span class="image-badge">当前</span>' : ''}
                </div>
                <div class="image-actions">
                    <button class="action-btn" onclick="gallery.setCurrentImage(${index})">选择</button>
                    <button class="action-btn delete" onclick="gallery.deleteImage('${image}')">删除</button>
                </div>
            </div>
        `).join('');
        
        // 更新选中状态
        this.updateSelectedImages();
    }
    
    updateCurrentImageInfo(data) {
        const info = document.getElementById('currentImageInfo');
        
        if (data.total === 0) {
            info.innerHTML = '<p>📷 没有图片</p>';
        } else {
            const currentName = data.current >= 0 && data.current < data.images.length 
                ? data.images[data.current] 
                : '无';
            
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
            const response = await fetch('/api/next', { method: 'POST' });
            const data = await response.json();
            
            if (data.status === 'ok') {
                this.refreshImageList();
                this.showStatus('切换到下一张图片', 'success');
            } else {
                this.showStatus(data.message || '切换失败', 'error');
            }
        } catch (error) {
            this.showStatus('切换图片失败', 'error');
        }
    }
    
    async previousImage() {
        try {
            const response = await fetch('/api/previous', { method: 'POST' });
            const data = await response.json();
            
            if (data.status === 'ok') {
                this.refreshImageList();
                this.showStatus('切换到上一张图片', 'success');
            } else {
                this.showStatus(data.message || '切换失败', 'error');
            }
        } catch (error) {
            this.showStatus('切换图片失败', 'error');
        }
    }
    
    async setCurrentImage(index) {
        try {
            const response = await fetch('/api/setimage', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `index=${index}`
            });
            
            const data = await response.json();
            
            if (data.status === 'ok') {
                this.refreshImageList();
                this.showStatus(`切换到图片: ${data.current}`, 'success');
            } else {
                this.showStatus(data.message || '切换失败', 'error');
            }
        } catch (error) {
            this.showStatus('切换图片失败', 'error');
        }
    }
    
    deleteImage(filename) {
        this.showConfirm(
            `确定要删除图片 "${filename}" 吗？此操作不可撤销。`,
            () => this.performDeleteImage(filename)
        );
    }
    
    async performDeleteImage(filename) {
        try {
            const response = await fetch('/api/delete', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `filename=${encodeURIComponent(filename)}`
            });
            
            const data = await response.json();
            
            if (data.status === 'ok') {
                this.refreshImageList();
                this.showStatus(`图片 "${filename}" 已删除`, 'success');
            } else {
                this.showStatus(data.message || '删除失败', 'error');
            }
        } catch (error) {
            this.showStatus('删除图片失败', 'error');
        }
    }
    
    selectAllImages() {
        const checkboxes = document.querySelectorAll('.image-checkbox');
        const allSelected = Array.from(checkboxes).every(cb => cb.checked);
        
        checkboxes.forEach(cb => {
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
            this.showStatus('请先选择要删除的图片', 'warning');
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
        
        this.showStatus(`成功删除 ${successCount}/${images.length} 张图片`, 'success');
    }
    
    updateSelectedImages() {
        const checkboxes = document.querySelectorAll('.image-checkbox');
        checkboxes.forEach(cb => {
            const item = cb.closest('.image-item');
            if (cb.checked) {
                item.classList.add('selected');
                this.selectedImages.add(cb.dataset.image);
            } else {
                item.classList.remove('selected');
                this.selectedImages.delete(cb.dataset.image);
            }
        });
    }
    
    toggleSlideshow() {
        const btn = document.getElementById('slideshowBtn');
        
        if (this.slideshowActive) {
            this.stopSlideshow();
            btn.textContent = '▶️ 幻灯片';
            btn.style.background = '#007bff';
        } else {
            this.startSlideshow();
            btn.textContent = '⏸️ 停止';
            btn.style.background = '#dc3545';
        }
    }
    
    startSlideshow() {
        if (this.currentImages.length === 0) {
            this.showStatus('没有图片可以播放', 'warning');
            return;
        }
        
        this.slideshowActive = true;
        this.slideshowInterval = setInterval(() => {
            this.nextImage();
        }, 3000); // 3秒切换一次
        
        this.showStatus('幻灯片已开始', 'success');
    }
    
    stopSlideshow() {
        this.slideshowActive = false;
        if (this.slideshowInterval) {
            clearInterval(this.slideshowInterval);
            this.slideshowInterval = null;
        }
        this.showStatus('幻灯片已停止', 'success');
    }
    
    async updateSystemStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            
            document.getElementById('wifiStatus').textContent = data.wifi ? '已连接' : '未连接';
            document.getElementById('ipAddress').textContent = data.ip || '未知';
            document.getElementById('storageInfo').textContent = data.storage || '计算中...';
            document.getElementById('uptime').textContent = data.uptime || '计算中...';
            
        } catch (error) {
            console.error('获取系统状态失败:', error);
        }
    }
    
    showStatus(message, type = 'success') {
        const status = document.getElementById('uploadStatus');
        status.textContent = message;
        status.className = `status-message ${type}`;
        status.style.display = 'block';
        
        setTimeout(() => {
            status.style.display = 'none';
        }, 3000);
    }
    
    updateProgress(percent) {
        const progressBar = document.getElementById('uploadProgress');
        const progressFill = progressBar.querySelector('.progress-fill');
        
        progressBar.style.display = 'block';
        progressFill.style.width = `${percent}%`;
        
        if (percent >= 100) {
            setTimeout(() => {
                progressBar.style.display = 'none';
            }, 1000);
        }
    }
    
    showLoading(show) {
        const loading = document.getElementById('loadingIndicator');
        loading.style.display = show ? 'flex' : 'none';
    }
    
    showConfirm(message, callback) {
        const dialog = document.getElementById('confirmDialog');
        const messageEl = document.getElementById('confirmMessage');
        
        messageEl.textContent = message;
        dialog.style.display = 'flex';
        this.confirmCallback = callback;
    }
    
    closeConfirmDialog() {
        const dialog = document.getElementById('confirmDialog');
        dialog.style.display = 'none';
        this.confirmCallback = null;
    }
    
    confirmAction() {
        if (this.confirmCallback) {
            this.confirmCallback();
        }
        this.closeConfirmDialog();
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
