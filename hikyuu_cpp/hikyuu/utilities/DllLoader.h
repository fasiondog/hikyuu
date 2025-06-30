/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_DLL_LOADER_H_
#define HKU_UTILS_DLL_LOADER_H_

#include <string>
#include <vector>
#include <cstdlib>
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace hku {

class DllLoader {
public:
    DllLoader() {
        initDefaultSearchPath();
    }

    explicit DllLoader(const std::vector<std::string>& path) {
        if (!path.empty()) {
            m_search_paths.resize(path.size());
            std::copy(path.begin(), path.end(), m_search_paths.begin());
        } else {
            HKU_WARN("DllLoader: empty search path! Use default search path!");
            initDefaultSearchPath();
        }
    }

    DllLoader(const DllLoader&) = delete;
    DllLoader& operator=(const DllLoader&) = delete;

    DllLoader(DllLoader&& rhs)
    : m_handle(rhs.m_handle), m_search_paths(std::move(rhs.m_search_paths)) {
        rhs.m_handle = nullptr;
    }

    DllLoader& operator=(DllLoader&& rhs) {
        if (this == &rhs) {
            m_handle = rhs.m_handle;
            m_search_paths = std::move(rhs.m_search_paths);
            rhs.m_handle = nullptr;
        }
        return *this;
    }

    virtual ~DllLoader() {
        unload();
    }

    bool load(const std::string& dllname) noexcept {
        std::string filename = search(dllname);
        HKU_WARN_IF_RETURN(filename.empty(), false, "Not found dll: {}!", dllname);

#if HKU_OS_WINDOWS
        m_handle = LoadLibrary(HKU_PATH(filename).c_str());
#else
        m_handle = dlopen(filename.c_str(), RTLD_LAZY);
#endif
        HKU_WARN_IF_RETURN(!m_handle, false, "load dll({}) failed!", filename);
        return true;
    }

    void unload() noexcept {
        if (m_handle) {
#if HKU_OS_WINDOWS
            FreeLibrary(m_handle);
#else
            dlclose(m_handle);
#endif
        }
    }

    std::string search(const std::string& dllname) const noexcept {
#if HKU_OS_WINDOWS
        std::string dll = fmt::format("{}.dll", dllname);
#elif HKU_OS_OSX
        std::string dll = fmt::format("lib{}.dylib", dllname);
#else
        std::string dll = fmt::format("lib{}.so", dllname);
#endif
        for (const auto& path : m_search_paths) {
            auto filename = fmt::format("{}/{}", path, dll);
            if (existFile(filename)) {
                return filename;
            }
        }

        return std::string();
    }

    void* getSymbol(const char* symbol) noexcept {
#if HKU_OS_WINDOWS
        void* func = GetProcAddress(m_handle, symbol);
#else
        void* func = dlsym(m_handle, symbol);
#endif
        return func;
    }

private:
    void initDefaultSearchPath() noexcept {
        m_search_paths.emplace_back(".");

        std::string userdir = getUserDir();
        if (!userdir.empty()) {
            m_search_paths.emplace_back(userdir + "/lib");
        }

#if HKU_OS_WINDOWS
        m_search_paths.emplace_back("C:/Windows/System32");
        m_search_paths.emplace_back("C:/Windows/SysWOW64");
        m_search_paths.emplace_back("C:/Windows");
        const char* path = getenv("PATH");
        if (path) {
            std::string pathstr(path);
            auto items = split(pathstr, ";");
            for (auto& item : items) {
                std::string nitem(item);
                trim(nitem);
                if (!item.empty()) {
                    m_search_paths.emplace_back(nitem);
                }
            }
        }

#elif HKU_OS_OSX
        const char* path = getenv("DYLD_IMAGE_SUFFIX");
        if (path) {
            std::string pathstr(path);
            auto items = split(pathstr, ";");
            for (auto& item : items) {
                std::string nitem(item);
                trim(nitem);
                if (!item.empty()) {
                    m_search_paths.emplace_back(nitem);
                }
            }
        }
        path = getenv("DYLD_FRAMEWORK_PATH");
        if (path) {
            std::string pathstr(path);
            auto items = split(pathstr, ";");
            for (auto& item : items) {
                std::string nitem(item);
                trim(nitem);
                if (!item.empty()) {
                    m_search_paths.emplace_back(nitem);
                }
            }
        }
        path = getenv("DYLD_LIBRARY_PATH");
        if (path) {
            std::string pathstr(path);
            auto items = split(pathstr, ";");
            for (auto& item : items) {
                std::string nitem(item);
                trim(nitem);
                if (!item.empty()) {
                    m_search_paths.emplace_back(nitem);
                }
            }
        }
        m_search_paths.emplace_back("/Library/Frameworks");
        m_search_paths.emplace_back("/Network/Library/Frameworks");
        m_search_paths.emplace_back("/System/Library/Frameworks");
        m_search_paths.emplace_back("/usr/local/lib");
        m_search_paths.emplace_back("/usr/lib");
        m_search_paths.emplace_back("/lib");

#else
        const char* path = getenv("LD_LIBRARY_PATH");
        if (path) {
            std::string pathstr(path);
            auto items = split(pathstr, ";");
            for (auto& item : items) {
                std::string nitem(item);
                trim(nitem);
                if (!item.empty()) {
                    m_search_paths.emplace_back(nitem);
                }
            }
        }
        m_search_paths.emplace_back("/usr/local/lib");
        m_search_paths.emplace_back("/usr/lib");
        m_search_paths.emplace_back("/lib");
        if (HKU_ARCH_X64) {
            m_search_paths.emplace_back("/usr/lib/x86_64-linux-gnu");
        } else if (HKU_ARCH_ARM64) {
            m_search_paths.emplace_back("/usr/lib/aarch64-linux-gnu");
        } else if (HKU_ARCH_X86) {
            m_search_paths.emplace_back("/usr/lib/i386-linux-gnu");
        } else if (HKU_ARCH_ARM) {
            m_search_paths.emplace_back("/usr/lib/arm-linux-gnueabihf");
        }
#endif
    }

private:
#if HKU_OS_WINDOWS
    HMODULE m_handle{nullptr};
#else
    void* m_handle{nullptr};
#endif

    std::vector<std::string> m_search_paths;
};

}  // namespace hku

#endif /* HKU_UTILS_DLL_LOADER_H_ */