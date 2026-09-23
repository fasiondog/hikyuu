#!/bin/bash
set -e
cd "$(dirname "$0")"

# 双语构建脚本
#   ./make.sh      构建中英两棵树 -> build/html/{en,zh}
#   ./make.sh en   仅构建英文树   -> build/html/en
#   ./make.sh zh   仅构建中文树   -> build/html/zh

if [ -n "$1" ]; then
    sphinx-build -b html "$1" "build/html/$1"
    exit 0
fi

sphinx-build -b html en build/html/en
sphinx-build -b html zh build/html/zh
