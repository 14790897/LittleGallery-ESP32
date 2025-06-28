#!/usr/bin/env python3
"""
Little Gallery ESP32 Web文件构建脚本
用于在编译前处理前端文件
"""

import os
import shutil
import gzip
from pathlib import Path

Import("env")

def compress_file(source_path, target_path):
    """压缩文件以节省存储空间"""
    try:
        with open(source_path, 'rb') as f_in:
            with gzip.open(target_path, 'wb') as f_out:
                shutil.copyfileobj(f_in, f_out)
        print(f"Compressed: {source_path} -> {target_path}")
        return True
    except Exception as e:
        print(f"Failed to compress {source_path}: {e}")
        return False

def build_web_files():
    """构建Web文件"""
    project_dir = env.get("PROJECT_DIR")
    data_dir = os.path.join(project_dir, "data")
    
    if not os.path.exists(data_dir):
        print("Warning: data directory not found, creating...")
        os.makedirs(data_dir)
        return
    
    print("Building web files...")
    
    # 检查必要的文件
    required_files = ["index.html", "style.css", "app.js"]
    missing_files = []
    
    for file in required_files:
        file_path = os.path.join(data_dir, file)
        if not os.path.exists(file_path):
            missing_files.append(file)
    
    if missing_files:
        print(f"Warning: Missing web files: {', '.join(missing_files)}")
        print("Please ensure all web files are in the data/ directory")
    else:
        print("All web files found")
    
    # 可选：压缩文件以节省空间
    # 注意：ESP32AsyncWebServer 支持 gzip 文件
    compress_web_files = env.GetProjectOption("custom_compress_web", "false").lower() == "true"
    
    if compress_web_files:
        print("Compressing web files...")
        for file in required_files:
            source_path = os.path.join(data_dir, file)
            target_path = os.path.join(data_dir, file + ".gz")
            
            if os.path.exists(source_path):
                compress_file(source_path, target_path)
    
    print("Web files build complete")

# 在构建前执行
build_web_files()
