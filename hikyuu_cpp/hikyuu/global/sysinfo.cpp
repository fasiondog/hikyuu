/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <hikyuu/GlobalInitializer.h>
#include <stdio.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <nlohmann/json.hpp>
#include "hikyuu/version.h"
#include "hikyuu/DataType.h"
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/http_client/HttpClient.h"
#include "sysinfo.h"

using json = nlohmann::json;

#define FEEDBACK_SERVER_ADDR "http://hikyuu.cpolar.cn"

namespace hku {

std::atomic<int> g_latest_version{0};
bool g_runningInPython{false};      // 是否是在 python 中运行
bool g_pythonInInteractive{false};  // python 是否运行在交互模式下
bool g_pythonInJupyter{false};      // python 是否运行在 Jupyter中

bool HKU_API runningInPython() {
    return g_runningInPython;
}

void HKU_API setRunningInPython(bool inpython) {
    g_runningInPython = inpython;
}

bool HKU_API pythonInInteractive() {
    return g_pythonInInteractive;
}

void HKU_API setPythonInInteractive(bool interactive) {
    g_pythonInInteractive = interactive;
}

bool HKU_API pythonInJupyter() {
    return g_pythonInJupyter;
}

void HKU_API setPythonInJupyter(bool injupyter) {
    g_pythonInJupyter = injupyter;
    if (createDir(fmt::format("{}/.hikyuu", getUserDir()))) {
        initLogger(injupyter, fmt::format("{}/.hikyuu/hikyuu.log", getUserDir()));
    } else {
        initLogger(injupyter);
    }
}

bool HKU_API CanUpgrade() {
    int current_version =
      HKU_VERSION_MAJOR * 1000000 + HKU_VERSION_MINOR * 1000 + HKU_VERSION_ALTER;
    return g_latest_version > current_version;
}

std::string HKU_API getLatestVersion() {
    int major = g_latest_version / 1000000;
    int minor = g_latest_version / 1000 - major * 1000;
    int alter = g_latest_version - (g_latest_version / 1000) * 1000;
    return fmt::format("{}.{}.{}", major, minor, alter);
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

// cppcheck-suppress constParameterReference
static bool readUUID(boost::uuids::uuid& out) {
    std::string filename = fmt::format("{}/.hikyuu/uid", getUserDir());
    FILE* fp = fopen(filename.c_str(), "rb");
    HKU_IF_RETURN(!fp, false);

    bool ret = true;
    if (16 != fread((void*)out.data, 1, 16, fp)) {
        ret = false;
    }

    fclose(fp);
    return ret;
}

static void saveUUID(const boost::uuids::uuid& uid) {
    std::string filename = fmt::format("{}/.hikyuu/uid", getUserDir());
    FILE* fp = fopen(filename.c_str(), "wb");
    HKU_IF_RETURN(!fp, void());

    fwrite(uid.data, 16, 1, fp);
    fclose(fp);
}

void sendFeedback() {
    std::thread t([] {
        try {
            boost::uuids::uuid uid;
            if (!readUUID(uid)) {
                uid = boost::uuids::random_generator()();
                saveUUID(uid);
            }

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
            g_latest_version = r["data"]["last_version"].get<int>();

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
