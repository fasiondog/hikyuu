/*
 * os.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-12-19
 *      Author: fasiondog
 */

#include "osdef.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <array>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#endif

#if HKU_OS_LINUX || HKU_OS_ANDROID
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#endif

#if HKU_OS_OSX || HKU_OS_IOS
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <cstring>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "arithmetic.h"
#include "Log.h"
#include "os.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

namespace hku {

bool HKU_UTILS_API existFile(const std::string &filename) noexcept {
#ifdef _WIN32
    auto attribs = GetFileAttributesA(HKU_PATH(filename).c_str());
    return attribs != INVALID_FILE_ATTRIBUTES;
#else  // common linux/unix all have the stat system call
    struct stat buffer;
    return (::stat(filename.c_str(), &buffer) == 0);
#endif
}

bool HKU_UTILS_API createDir(const std::string &pathname) noexcept {
    std::string npath = HKU_PATH(pathname);

    // 目录已存在，直接返回成功
    if (access(npath.c_str(), 0) == 0) {
        return true;
    }

#ifdef _WIN32
    if (mkdir(npath.c_str()) == 0) {
        return true;
    }
#else
    // 默认权限：rwxrwx--x
    if (mkdir(npath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH) == 0) {
        return true;
    }
#endif

    // mkdir失败的情况下，重新判断目录是否存在，保护并发情况下创建同一目录的情况
    return access(npath.c_str(), 0) == 0;
}

bool HKU_UTILS_API isColorTerminal() noexcept {
#if defined(_WIN32) || defined(__linux__) || defined(__ANDROID__)
    return true;
#elif TARGET_OS_IPHONE
    return false;
#else
    static constexpr std::array<const char *, 14> Terms = {
      {"ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux", "msys", "putty",
       "rxvt", "screen", "vt100", "xterm"}};

    const char *env_p = std::getenv("TERM");
    if (env_p == nullptr) {
        return false;
    }

    static const bool result =
      std::any_of(std::begin(Terms), std::end(Terms),
                  [&](const char *term) { return std::strstr(env_p, term) != nullptr; });
    return result;
#endif
}

// 删除文件
bool HKU_UTILS_API removeFile(const std::string &filename) noexcept {
    return std::remove(HKU_PATH(filename).c_str()) == 0;
}

#ifdef _WIN32
// 删除目录及其包含的文件和子目录
bool HKU_UTILS_API removeDir(const std::string &path) noexcept {
    std::string strPath = HKU_PATH(path);
    struct _finddata_t fb;  // 查找相同属性文件的存储结构体
    // 制作用于正则化路径
    if (strPath.at(strPath.length() - 1) != '\\' && strPath.at(strPath.length() - 1) != '/') {
        strPath.append("\\");
    }
    std::string findPath = strPath + "*";
    intptr_t handle;  // 用long类型会报错
    handle = _findfirst(findPath.c_str(), &fb);
    // 找到第一个匹配的文件
    if (handle != -1L) {
        std::string pathTemp;
        do  // 循环找到的文件
        {
            // 系统有个系统文件，名为“..”和“.”,对它不做处理
            if (strcmp(fb.name, "..") != 0 &&
                strcmp(fb.name, ".") != 0)  // 对系统隐藏文件的处理标记
            {
                // 制作完整路径
                pathTemp.clear();
                pathTemp = strPath + std::string(fb.name);
                // 属性值为16，则说明是文件夹，迭代
                if (fb.attrib == _A_SUBDIR)  //_A_SUBDIR=16
                {
                    removeDir(GBToUTF8(pathTemp));
                }
                // 非文件夹的文件，直接删除。对文件属性值的情况没做详细调查，可能还有其他情况。
                else {
                    remove(pathTemp.c_str());
                }
            }
        } while (0 == _findnext(handle, &fb));  // 判断放前面会失去第一个搜索的结果
        // 关闭文件夹，只有关闭了才能删除。找这个函数找了很久，标准c中用的是closedir
        // 经验介绍：一般产生Handle的函数执行后，都要进行关闭的动作。
        _findclose(handle);
    }
    // 移除文件夹
    return _rmdir(strPath.c_str()) == 0;
}

#else   // #ifdef _WIN32
// 删除目录及其包含的文件和子目录
bool HKU_UTILS_API removeDir(const std::string &path) noexcept {
    std::string strPath(path);
    if (strPath.at(strPath.length() - 1) != '\\' && strPath.at(strPath.length() - 1) != '/') {
        strPath.append("/");
    }
    DIR *d = opendir(strPath.c_str());  // 打开这个目录
    if (d != NULL) {
        struct dirent *dt = NULL;

        // 逐个读取目录中的文件到dt
        while (NULL != (dt = readdir(d))) {
            // 系统有个系统文件，名为“..”和“.”,对它不做处理
            if (std::strcmp(dt->d_name, "..") != 0 && std::strcmp(dt->d_name, ".") != 0) {
                struct stat st;        // 文件的信息
                std::string fileName;  // 文件夹中的文件名
                fileName = strPath + std::string(dt->d_name);
                stat(fileName.c_str(), &st);
                if (S_ISDIR(st.st_mode)) {
                    removeDir(fileName);
                } else {
                    remove(fileName.c_str());
                }
            }
        }
        closedir(d);
    }
    return rmdir(strPath.c_str()) == 0;
}
#endif  // #ifdef _WIN32

bool HKU_UTILS_API copyFile(const std::string &src, const std::string &dst, bool flush) noexcept {
    bool success = false;
    try {
        std::ifstream srcio(HKU_PATH(src), std::ios::binary);
        std::ofstream dstio(HKU_PATH(dst), std::ios::binary);
        dstio << srcio.rdbuf();
        if (flush) {
            dstio.flush();
        }
        success = true;
    } catch (...) {
        success = false;
    }
    return success;
}

bool HKU_UTILS_API renameFile(const std::string &oldname, const std::string &newname,
                              bool overlay) noexcept {
    // 先判定文件是否存在，保证 std::rename 的行为和系统无关
    if (overlay) {
        HKU_ERROR_IF_RETURN(existFile(newname) && !removeFile(newname), false,
                            "Error renaming file! The new file is occupied");
    } else {
        HKU_ERROR_IF_RETURN(existFile(newname), false,
                            "Error renaming file! The new file is occupied");
    }
    int result = std::rename(HKU_PATH(oldname).c_str(), HKU_PATH(newname).c_str());
    HKU_ERROR_IF_RETURN(result != 0, false, "Error renaming file! errno: {}, errmsg: {}", errno,
                        strerror(errno));
    return true;
}

/*
 * 获取用户路径
 */
static std::string _getUserDir() {
    char *home = getenv("HOME");
    if (home) {
        return std::string(home);
    }

    home = getenv("USERPROFILE");
    if (home) {
        return std::string(home);
    }

    char *hdrive = getenv("HOMEDRIVE"), *hpath = getenv("HOMEPATH");
    if (hdrive && hpath) {
        return std::string(hdrive) + std::string(hpath);
    }

    printf("Can't get user's path");
    return std::string();
}

std::string HKU_UTILS_API getUserDir() {
#ifdef _WIN32
    return GBToUTF8(_getUserDir());
#else
    return _getUserDir();
#endif
}

std::string HKU_UTILS_API getCurrentDir() {
    std::string ret;
    char *buffer = NULL;
#if HKU_OS_WINSOWS
    buffer = _getcwd(buffer, 0);
#else
    buffer = getcwd(buffer, 0);
#endif

    if (buffer) {
#if HKU_OS_WINDOWS
        ret = GBToUTF8(std::string(buffer));
#else
        ret = std::string(buffer);
#endif
        free(buffer);
    }
    return ret;
}

std::string HKU_UTILS_API getDllSelfDir() {
    std::string libraryPath;

#if HKU_OS_WINDOWS
    char buffer[MAX_PATH];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL) {
        if (GetModuleFileNameA(hModule, buffer, MAX_PATH) > 0) {
            libraryPath = buffer;
        }
    }
#else
    void *handle = dlopen(NULL, RTLD_LAZY);
    Dl_info info;

    if (dladdr((void *)getDllSelfDir, &info) && info.dli_fname) {
        libraryPath = info.dli_fname;
    }
    dlclose(handle);
#endif

    // 提取目录部分（不包含文件名）
    if (!libraryPath.empty()) {
#if HKU_OS_WINDOWS
        // Windows路径处理
        size_t pos = libraryPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            return libraryPath.substr(0, pos);
        }
#else
        // Unix路径处理
        char *dir = dirname(const_cast<char *>(libraryPath.c_str()));
        if (dir) {
            return dir;
        }
#endif
    }

    return libraryPath;
}

uint64_t HKU_UTILS_API getDiskFreeSpace(const char *path) {
#if HKU_OS_WINDOWS
    uint64_t freespace = Null<uint64_t>();
    std::string npath = HKU_PATH(path);
    LPCSTR lpcwstrDriver = npath.c_str();
    ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
    if (GetDiskFreeSpaceExA(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes,
                            &liTotalFreeBytes)) {
        freespace = uint64_t(liTotalFreeBytes.QuadPart);
    }
    return freespace;

#else
    uint64_t freespace = Null<uint64_t>();
    struct statfs disk_statfs;
    if (statfs(path, &disk_statfs) >= 0) {
        freespace = ((uint64_t)disk_statfs.f_bsize * (uint64_t)disk_statfs.f_bfree);
    }
    return freespace;
#endif
}

std::string HKU_UTILS_API getPlatform() {
    std::string ret;
#if HKU_OS_WINDOWS
    ret = "windows";
#elif HKU_OS_LINUX
    ret = "linux";
#elif HKU_OS_ANDROID
    ret = "android";
#elif HKU_OS_OSX
    ret = "macosx";
#elif TARGET_OS_SIMULATOR
    ret = "iphoneos_simulator";
#elif HKU_OS_IOS
    ret = "iphoneos";
#else
    ret = "unknown";
#endif
    return ret;
}

std::string HKU_UTILS_API getCpuArch() {
    std::string ret;
#if HKU_ARCH_ARM
    ret = "arm";
#elif HKU_ARCH_ARM64
    ret = "arm64";
#elif HKU_ARCH_X86
    ret = "x86";
#elif HKU_ARCH_X64
    ret = "x64";
#else
    ret = "unknown";
#endif
    return ret;
}

#if HKU_OS_WINDOWS
std::string HKU_UTILS_API getSystemLanguage() {
    LCID lcid = GetUserDefaultUILanguage();
    char lang[256];

    std::string ret;
    if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, lang, sizeof(lang)) == 0) {
        return ret;
    }

    ret = std::string(lang);
    to_lower(ret);
    if (ret == "zh") {
        ret = "zh_cn";
    }
    return ret;
}

#elif HKU_OS_LINUX
std::string HKU_UTILS_API getSystemLanguage() {
    std::string ret;
    const char *langEnv = std::getenv("LANG");
    HKU_IF_RETURN(langEnv == nullptr, ret);

    std::string lang(langEnv);
    auto ss = split(lang, '.');
    ret = std::string(ss[0]);
    to_lower(ret);
    return ret;
}

#elif HKU_OS_OSX
std::string HKU_UTILS_API getSystemLanguage() {
    CFLocaleRef currentLocale = CFLocaleCopyCurrent();

    // 显式类型转换
    CFStringRef languageCode = (CFStringRef)CFLocaleGetValue(currentLocale, kCFLocaleLanguageCode);

    if (languageCode == nullptr || !(CFStringGetTypeID() == CFGetTypeID(languageCode))) {
        CFRelease(currentLocale);
        return "";
    }

    CFIndex length = CFStringGetLength(languageCode);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    std::unique_ptr<char[]> buffer(new char[maxSize]);

    if (CFStringGetCString(languageCode, buffer.get(), maxSize, kCFStringEncodingUTF8)) {
        std::string result(buffer.get());
        CFRelease(currentLocale);
        to_lower(result);
        if (result == "zh") {
            result = "zh_cn";
        }
        return result;
    }

    CFRelease(currentLocale);
    return "";
}
#endif

uint64_t HKU_UTILS_API getMemoryMaxSize() {
#if HKU_OS_WINDOWS
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<uint64_t>(memInfo.ullTotalPhys);
    }
    return 0;

#elif HKU_OS_LINUX || HKU_OS_ANDROID
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) == 0) {
        return static_cast<uint64_t>(memInfo.totalram) * memInfo.mem_unit;
    }
    return 0;

#elif HKU_OS_OSX || HKU_OS_IOS
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t physicalMemory;
    size_t length = sizeof(physicalMemory);

    if (sysctl(mib, 2, &physicalMemory, &length, NULL, 0) == 0) {
        return physicalMemory;
    }
    return 0;

#else
    return 0;
#endif
}

uint64_t HKU_UTILS_API getMemoryIdleSize() {
#if HKU_OS_WINDOWS
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<uint64_t>(memInfo.ullAvailPhys);
    }
    return 0;

#elif HKU_OS_LINUX || HKU_OS_ANDROID
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) == 0) {
        return static_cast<uint64_t>(memInfo.freeram) * memInfo.mem_unit;
    }
    return 0;

#elif HKU_OS_OSX || HKU_OS_IOS
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    mach_port_t hostPort = mach_host_self();

    if (host_statistics(hostPort, HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS) {
        // 计算空闲内存：空闲页 + 非活跃页
        uint64_t pageSize = static_cast<uint64_t>(vm_page_size);
        uint64_t freeMemory =
          static_cast<uint64_t>(vmstat.free_count + vmstat.inactive_count) * pageSize;
        return freeMemory;
    }
    return 0;

#else
    return 0;
#endif
}

}  // namespace hku

#ifdef _MSC_VER
#pragma warning(pop)
#endif