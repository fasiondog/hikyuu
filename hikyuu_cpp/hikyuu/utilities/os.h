/*
 * os.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-12-19
 *      Author: fasiondog
 *
 *  用途：跨系统函数封装
 *  注意：如无特殊说明，输入和输出的字符串均为 UTF8 编码
 */

#pragma once

#include <string>
#include "Null.h"
#include "arithmetic.h"
#include <cstdint>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * 判断文件或目录是否存在
 * @param filename 文件名或目录名
 */
bool HKU_API existFile(const std::string& filename) noexcept;

/**
 * 创建目录
 * @param pathname 路径名
 * @return 如果目录已存在或者创建成功返回 true，否则返回 false
 */
bool HKU_API createDir(const std::string& pathname) noexcept;

/**
 * 删除文件
 * @param filename 文件名
 * @return 删除失败或文件不存在时返回false
 */
bool HKU_API removeFile(const std::string& filename) noexcept;

/**
 * 删除目录及其包含的文件和子目录
 * @param path 待删除目录
 */
bool HKU_API removeDir(const std::string& path) noexcept;

/**
 * 拷贝文件
 * @param src 原文件
 * @param dst 目标文件
 * @param flush 是否立即落盘
 */
bool HKU_API copyFile(const std::string& src, const std::string& dst, bool flush = false) noexcept;

/**
 * 文件、目录改名或移动
 * @param oldname 旧名
 * @param newname 新名
 * @param overlay 新名文件已存在时，强制覆盖
 * @return true 成功
 * @return false 失败 旧名文件不存在，或文件被占用等其他原因导致的失败
 */
bool HKU_API renameFile(const std::string& oldname, const std::string& newname,
                        bool overlay = false) noexcept;

/**
 * 获取用户路径
 */
std::string HKU_API getUserDir();

/**
 * 获取程序当前所在路径
 */
std::string HKU_API getCurrentDir();

/**
 * 输出终端是否支持彩色控制字符
 */
bool HKU_API isColorTerminal() noexcept;

/**
 * @brief 获取硬盘剩余存储空间大小
 * @note windows下如果指定路径无效时，返回的是当前磁盘剩余空间。
 *       某些 64 位linux下 size_t 位 32位无符号整数，这里返回值需使用 uint64_t
 * @param path 指定路径名
 * @return uint64_t 获取失败时，返回 Null<uint64_t>()
 */
uint64_t HKU_API getDiskFreeSpace(const char* path);

/** 获取当前系统名称 */
std::string HKU_API getPlatform();

/** 获取当前CPU架构名称 */
std::string HKU_API getCpuArch();

}  // namespace hku