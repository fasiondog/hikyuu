/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <hikyuu/GlobalInitializer.h>
#include <stdio.h>
#include <shared_mutex>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <nlohmann/json.hpp>
#include "hikyuu/version.h"
#include "hikyuu/DataType.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/http_client/HttpClient.h"
#include "sysinfo.h"

using json = nlohmann::json;

#define FEEDBACK_SERVER_ADDR "http://hikyuu.cpolar.cn"

namespace hku {

struct InnerSysInfo {
    bool runningInPython{false};      // 是否是在 python 中运行
    bool pythonInInteractive{false};  // python 是否运行在交互模式下
    bool pythonInJupyter{false};      // python 是否运行在 Jupyter中

    LatestVersionInfo latest_version_info;
    std::shared_mutex latest_version_mutex;
};

static InnerSysInfo* g_sys_info;

void sysinfo_init() {
    g_sys_info = new InnerSysInfo;
    g_sys_info->latest_version_info.version =
      HKU_VERSION_MAJOR * 1000000 + HKU_VERSION_MINOR * 1000 + HKU_VERSION_ALTER;
}
void sysinfo_clean() {
    if (g_sys_info) {
        delete g_sys_info;
        g_sys_info = nullptr;
    }
}

bool HKU_API runningInPython() {
    return g_sys_info->runningInPython;
}

void HKU_API setRunningInPython(bool inpython) {
    g_sys_info->runningInPython = inpython;
}

bool HKU_API pythonInInteractive() {
    return g_sys_info->pythonInInteractive;
}

void HKU_API setPythonInInteractive(bool interactive) {
    g_sys_info->pythonInInteractive = interactive;
}

bool HKU_API pythonInJupyter() {
    return g_sys_info->pythonInJupyter;
}

void HKU_API setPythonInJupyter(bool injupyter) {
    g_sys_info->pythonInJupyter = injupyter;
    if (createDir(fmt::format("{}/.hikyuu", getUserDir()))) {
        initLogger(injupyter, fmt::format("{}/.hikyuu/hikyuu.log", getUserDir()));
    } else {
        initLogger(injupyter);
    }
}

bool HKU_API CanUpgrade() {
    int current_version =
      HKU_VERSION_MAJOR * 1000000 + HKU_VERSION_MINOR * 1000 + HKU_VERSION_ALTER;
    std::shared_lock<std::shared_mutex> lock(g_sys_info->latest_version_mutex);
    return g_sys_info->latest_version_info.version > current_version;
}

LatestVersionInfo HKU_API getLatestVersionInfo() {
    std::shared_lock<std::shared_mutex> lock(g_sys_info->latest_version_mutex);
    return g_sys_info->latest_version_info;
}

std::string getVersion() {
    return HKU_VERSION;
}

std::string getVersionWithBuild() {
    return fmt::format("{}_{}_{}_{}_{}", HKU_VERSION, HKU_VERSION_BUILD, HKU_VERSION_MODE,
                       getPlatform(), getCpuArch());
}

std::string getVersionWithGit() {
    return HKU_VERSION_GIT;
}

static boost::uuids::uuid readUUID() {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
    boost::uuids::uuid uid;
    std::string filename = fmt::format("{}/.hikyuu/uid", getUserDir());
    if (existFile(filename)) {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp) {
            fread((void*)uid.data, 1, 16, fp);
            fclose(fp);
        }
    } else {
        uid = boost::uuids::random_generator()();
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            fwrite(uid.data, 16, 1, fp);
            fclose(fp);
        }
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    return uid;
}

void sendFeedback() {
    boost::uuids::uuid uid = readUUID();
    std::thread t([uid] {
        try {
            HttpClient client(FEEDBACK_SERVER_ADDR, 2000);
            json req;
            req["uid"] = boost::uuids::to_string(uid);
            req["part"] = "hikyuu";
            req["version"] = HKU_VERSION;
            req["build"] = fmt::format("{}", HKU_VERSION_BUILD);
            req["platform"] = getPlatform();
            req["arch"] = getCpuArch();
            auto res = client.post("/hku/visit", req);
            json r = res.json();
            const json& data = r["data"];

            if (!g_sys_info || StockManager::instance().hasCancelLoad()) {
                return;
            }

            if (g_sys_info) {
                std::unique_lock<std::shared_mutex> lock(g_sys_info->latest_version_mutex);
                g_sys_info->latest_version_info.version = data["last_version"].get<int>();
                if (data.contains("remark")) {
                    g_sys_info->latest_version_info.remark = data["remark"].get<std::string>();
                    g_sys_info->latest_version_info.release_date =
                      Datetime(data["release_date"].get<std::string>());
                } else {
                    g_sys_info->latest_version_info.remark =
                      "release note: https://hikyuu.readthedocs.io/zh-cn/latest/release.html";
                    g_sys_info->latest_version_info.release_date = Datetime();
                }
            }

        } catch (...) {
            // do nothing
        }
    });
    t.detach();
    // t.join();
}

void sendPythonVersionFeedBack(int major, int minor, int micro) {
    std::thread t([=]() {
        try {
            HttpClient client(FEEDBACK_SERVER_ADDR, 2000);
            json req;
            req["major"] = major;
            req["minor"] = minor;
            req["micro"] = micro;
            client.post("/hku/pyver", req);
        } catch (...) {
            // do nothing
        }
    });
    t.detach();
}

}  // namespace hku
