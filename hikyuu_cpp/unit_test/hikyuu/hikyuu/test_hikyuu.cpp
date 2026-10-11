/*
 * test_hikyuu.cpp
 *
 *  Regression tests for hikyuu global init / config parsing
 */

#include "doctest/doctest.h"
#include <fstream>
#include <cstdio>
#include <hikyuu/hikyuu.h>
#include <hikyuu/utilities/Parameter.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_suite test_hikyuu_suite
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_hikyuu_lazy_preload_ini_type") {
    const std::string filename("test_hikyuu_lazy_preload.ini");

    auto write_ini = [&](const std::string& value) {
        std::ofstream ini(filename, std::ofstream::trunc);
        ini << "[hikyuu]\n";
        ini << "lazy_preload = " << value << "\n";
        ini << "[baseinfo]\n";
        ini << "[block]\n";
        ini << "[kdata]\n";
        ini << "[preload]\n";
        ini.close();
    };

    Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;

    /** @arg lazy_preload = True must be read back as bool true (not swallowed by any_cast) */
    write_ini("True");
    getConfigFromIni(filename, baseParam, blockParam, kdataParam, preloadParam, hkuParam);
    CHECK(hkuParam.tryGet<bool>("lazy_preload", false));

    /** @arg lazy_preload = False must be read back as bool false, not the true default */
    write_ini("False");
    getConfigFromIni(filename, baseParam, blockParam, kdataParam, preloadParam, hkuParam);
    CHECK_UNARY_FALSE(hkuParam.tryGet<bool>("lazy_preload", true));

    std::remove(filename.c_str());
}

/** @} */
