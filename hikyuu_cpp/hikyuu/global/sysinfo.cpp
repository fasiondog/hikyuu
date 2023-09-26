/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/Log.h"
#include "sysinfo.h"

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

static std::string generatorUUID() {
    boost::uuids::uuid uid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uid);
}

void sendProbeInfo() {}

}  // namespace hku