/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <stdio.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "hikyuu/version.h"
#include "hikyuu/DataType.h"
#include "hikyuu/utilities/os.h"
#include "sysinfo.h"

using json = nlohmann::json;

namespace hku {

std::string getVersion() {
    return HKU_VERSION;
}

std::string HKU_API getVersionWithBuild() {
#if defined(_DEBUG) || defined(DEBUG)
    std::string mode("debug");
#else
    std::string mode("release");
#endif
#if HKU_ARCH_ARM
    return fmt::format("{}_{}_arm_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif HKU_ARCH_ARM64
    return fmt::format("{}_{}_aarch64_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif HKU_ARCH_X64
    return fmt::format("{}_{}_x64_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif HKU_ARCH_X86
    return fmt::format("{}_{}_i386_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#else
    return fmt::format("{}_{}_unknow_arch_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#endif
}

static bool readUUID(boost::uuids::uuid& out) {
    std::string filename = fmt::format("{}/.hikyuu/uid", getUserDir());
    FILE* fp = fopen(filename.c_str(), "rb");
    HKU_IF_RETURN(!fp, false);

    bool ret = true;
    if (16 != fread(out.data, 1, 16, fp)) {
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

            json req;
            req["uid"] = boost::uuids::to_string(uid);

            httplib::SSLClient cli("hikyuu.cpolar.cn");
            cli.set_compress(true);
            cli.set_connection_timeout(0, 3000000);  // 3 seconds
            cli.set_read_timeout(5, 0);              // 5 seconds
            cli.set_write_timeout(5, 0);             // 5 seconds
            cli.Post("/feedback", req.dump(), "application/json");
            HKU_INFO("success feedback");
        } catch (...) {
            // do nothing
        }
    });
    t.detach();
}

}  // namespace hku