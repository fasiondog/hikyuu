#!/bin/bash

# Script to automatically compile zh_CN.po to hikyuu.mo
# The hikyuu.mo will be saved in the zh_CN subdirectory

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 定义输入和输出路径
PO_FILE="${SCRIPT_DIR}/zh_CN.po"
ZH_CN_DIR="${SCRIPT_DIR}/zh_CN"
MO_FILE="${ZH_CN_DIR}/hikyuu.mo"

echo "Updating translation from zh_CN.po to hikyuu.mo..."

# 检查po文件是否存在
if [ ! -f "$PO_FILE" ]; then
    echo "Error: $PO_FILE does not exist!"
    exit 1
fi

# 检查zh_CN目录是否存在，如果不存在则创建
if [ ! -d "$ZH_CN_DIR" ]; then
    echo "Creating directory $ZH_CN_DIR..."
    mkdir -p "$ZH_CN_DIR"
fi

# 使用msgfmt命令编译po文件为mo文件
if command -v msgfmt &> /dev/null; then
    echo "Compiling $PO_FILE to $MO_FILE..."
    msgfmt -o "$MO_FILE" "$PO_FILE"
    
    if [ $? -eq 0 ]; then
        echo "Successfully compiled $PO_FILE to $MO_FILE"
    else
        echo "Error: Failed to compile $PO_FILE"
        exit 1
    fi
else
    echo "Error: msgfmt command not found. Please install gettext package."
    echo "On Ubuntu/Debian: sudo apt-get install gettext"
    echo "On CentOS/RHEL: sudo yum install gettext"
    echo "On macOS: brew install gettext"
    exit 1
fi

echo "Translation update completed!"