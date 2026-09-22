/*
 * os.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-12-19
 *      Author: fasiondog
 *
 *  Purpose: the cross-system function encapsulation
 *  Note: unless otherwise specified, the input and output strings are UTF8 encoded
 */

#pragma once

#include <string>
#include "Null.h"
#include "arithmetic.h"
#include <cstdint>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * Judge whether a file or a directory exists
 * @param filename file name or directory name
 */
bool HKU_UTILS_API existFile(const std::string &filename) noexcept;

/**
 * Create a directory
 * @param pathname path name
 * @return true is returned if the directory already exists or is created successfully, otherwise
 *         false
 */
bool HKU_UTILS_API createDir(const std::string &pathname) noexcept;

/**
 * Delete a file
 * @param filename file name
 * @return false is returned when the deletion fails or the file does not exist
 */
bool HKU_UTILS_API removeFile(const std::string &filename) noexcept;

/**
 * Delete a directory and the files and subdirectories it contains
 * @param path the directory to be deleted
 */
bool HKU_UTILS_API removeDir(const std::string &path) noexcept;

/**
 * Copy a file
 * @param src the source file
 * @param dst the target file
 * @param flush whether to write to the disk immediately
 */
bool HKU_UTILS_API copyFile(const std::string &src, const std::string &dst,
                            bool flush = false) noexcept;

/**
 * Rename or move a file or a directory
 * @param oldname the old name
 * @param newname the new name
 * @param overlay forcefully overwrite when the file with the new name already exists
 * @return true success
 * @return false failure, caused by the non-existing file with the old name, the file being occupied
 *         or other reasons
 */
bool HKU_UTILS_API renameFile(const std::string &oldname, const std::string &newname,
                              bool overlay = false) noexcept;

/**
 * Get the user path
 */
std::string HKU_UTILS_API getUserDir();

/**
 * Get the current path of the program
 */
std::string HKU_UTILS_API getCurrentDir();

/**
 * Get the directory of the dll itself
 * @note It is supported under linux/macos only. Under Windows the path of the exe is got!
 */
std::string HKU_UTILS_API getDllSelfDir();

/**
 * Whether the output terminal supports the color control characters
 */
bool HKU_UTILS_API isColorTerminal() noexcept;

/**
 * @brief Get the size of the remaining storage space of the disk
 * @note Under Windows the remaining space of the current disk is returned when the given path is
 * invalid.
 *       Under some 64-bit linux size_t is a 32-bit unsigned integer, so uint64_t must be used for
 * the return value here
 * @param path the given path name
 * @return uint64_t Null<uint64_t>() is returned when it fails
 */
uint64_t HKU_UTILS_API getDiskFreeSpace(const char *path);

/** Get the current system name */
std::string HKU_UTILS_API getPlatform();

/** Get the current CPU architecture name */
std::string HKU_UTILS_API getCpuArch();

/** Get the current system language name (returned in lowercase) */
std::string HKU_UTILS_API getSystemLanguage();

uint64_t HKU_UTILS_API getMemoryMaxSize();

uint64_t HKU_UTILS_API getMemoryIdleSize();

}  // namespace hku