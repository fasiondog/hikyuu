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
#include "node/NodeClient.h"
#include "sysinfo.h"

using json = nlohmann::json;

namespace hku {

std::string getVersion() {
    return HKU_VERSION;
}

std::string HKU_API getVersionWithBuild() {
#if defined(_DEBUG) || defined(DEBUG)
    return fmt::format("{}_{}_debug_{}_{}", HKU_VERSION, HKU_VERSION_BUILD, getPlatform(),
                       getCpuArch());
#else
    return fmt::format("{}_{}_release_{}_{}", HKU_VERSION, HKU_VERSION_BUILD, getPlatform(),
                       getCpuArch());
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

            NodeClient client("tcp://1.tcp.cpolar.cn:20981");
            client.dial();

            json req, res;
            req["cmd"] = 2;
            client.post(req, res);
            std::string host = res["host"].get<std::string>();
            uint64_t port = res["port"].get<uint64_t>();
            client.close();

            client.setServerAddr(fmt::format("tcp://{}:{}", host, port));
            client.dial();
            req["cmd"] = 1;
            req["uid"] = boost::uuids::to_string(uid);
            req["part"] = "hikyuu";
            req["version"] = HKU_VERSION;
            req["build"] = fmt::format("{}", HKU_VERSION_BUILD);
            req["platform"] = getPlatform();
            req["arch"] = getCpuArch();
            client.post(req, res);

        } catch (...) {
            // do nothing
        }
    });
    t.detach();
    // t.join();
}

}  // namespace hku