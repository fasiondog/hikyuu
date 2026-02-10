
target("hikyuu")
    set_kind("$(kind)")

    if is_mode("coverage") then 
        add_cxflags("-fprofile-update=atomic")
    end

    if get_config("leak_check") then
        if is_plat("macosx") then
            set_policy("build.sanitizer.address", true)
        elseif is_plat("linux") then
            -- 需要 export LD_PRELOAD=libasan.so
            set_policy("build.sanitizer.address", true)
            set_policy("build.sanitizer.leak", true)
            -- set_policy("build.sanitizer.memory", true)
            -- set_policy("build.sanitizer.thread", true)
        end
    end

    add_packages("boost", "fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json", "xxhash", "eigen")
    if is_plat("windows", "linux", "cross", "macosx") then
        if get_config("sqlite") or get_config("hdf5") then
            add_packages("sqlite3")
        end
    end

    if has_config("mimalloc") then 
        add_packages("mimalloc")
    end

    if has_config("omp") then 
        add_packages("openmp")
        if is_plat("macosx") then
            add_packages("libomp")
        end
    end

    if has_config("http_client_zip") then
        add_packages("gzip-hpp")
    end

    if has_config("ta_lib") then
        add_packages("ta-lib")
    end

    add_options("mysql")
    add_includedirs("..")

    -- set version for release
    set_configdir("./")
    add_configfiles("$(projectdir)/config.h.in")
    add_configfiles("$(projectdir)/version.h.in")
    add_configfiles("$(projectdir)/config_utils.h.in", {prefixdir="utilities", filename="config.h"})

    add_defines("CPPHTTPLIB_OPENSSL_SUPPORT", "CPPHTTPLIB_ZLIB_SUPPORT")

    if is_plat("windows") then
        add_cxflags("/bigobj")
        add_cxflags("-wd4819")
        add_cxflags("-wd4251") -- template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834") -- C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4244") -- discable double to int
        add_links("bcrypt")
    else
        add_rpathdirs("$ORIGIN")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end

    if get_config("hdf5") then
        add_packages("hdf5")
    end
    if get_config("mysql") then
        add_packages("mysql")
    end

    if is_plat("windows") then
        if is_kind("shared") then
            add_defines("HKU_API=__declspec(dllexport)")
            add_defines("HKU_UTILS_API=__declspec(dllexport)")
        end
    end

    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
    end

    if is_plat("macosx") then
        -- macosx下boost序列化需要
        if is_kind("shared") then 
            add_defines("HKU_API=__attribute__((visibility(\"default\")))")
            add_defines("HKU_UTILS_API=__attribute__((visibility(\"default\")))")
        end
        add_cxflags("-frtti")
        add_shflags("-Wl,-flat_namespace,-undefined,dynamic_lookup")
        add_links("sqlite3")
        add_frameworks("CoreFoundation")
    end

    add_headerfiles("../(hikyuu/**.h)|**doc.h")

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./**.cpp|data_driver/**.cpp|utilities/db_connect/mysql/**.cpp|indicator_talib/**.cpp")
   
    add_files("./*.cpp", "./serialization/*.cpp", {unity_group="base"})
    add_files("./indicator/**.cpp", {unity_group="indicator"})

    add_files("./analysis/**.cpp", {unity_group="analysis"})
    add_files("./global/**.cpp", {unity_group="global"})
    add_files("./plugin/**.cpp", {unity_group="plugin"})
    add_files("./strategy/**.cpp", {unity_group="strategy"})
    add_files("./trade_manage/**.cpp", {unity_group="trade_manage"})
    
    add_files("./trade_sys/**.cpp|allocatefunds/**.cpp|condition/**.cpp|system/**.cpp")
    add_files("./trade_sys/allocatefunds/**.cpp", {unity_group="allocatefunds"})
    add_files("./trade_sys/condition/*.cpp", "./trade_sys/condition/imp/logic/*.cpp", {unity_group="condition"})
    add_files("./trade_sys/condition/imp/*.cpp", {unity_group="condition_imp"})
    add_files("./trade_sys/environment/*.cpp", "./trade_sys/environment/imp/logic/*.cpp", {unity_group="environment"})
    add_files("./trade_sys/environment/imp/*.cpp", {unity_group="environment_imp"})
    add_files("./trade_sys/system/**.cpp", {unity_group="system"})
    add_files("./trade_sys/moneymanager/**.cpp", {unity_group="moneymanager"})
    add_files("./trade_sys/multifactor/*.cpp", "./trade_sys/multifactor/imp/*.cpp", {unity_group="multifactor"})
    add_files("./trade_sys/multifactor/filter/*.cpp", {unity_group="multifactor_filter"})
    add_files("./trade_sys/multifactor/normalize/*.cpp", {unity_group="multifactor_norm"})
    add_files("./trade_sys/portfolio/**.cpp", {unity_group="portfolio"})
    add_files("./trade_sys/profitgoal/**.cpp", {unity_group="profitgoal"})
    add_files("./trade_sys/selector/*.cpp", "./trade_sys/selector/imp/logic/*.cpp", {unity_group="selector"})
    add_files("./trade_sys/selector/imp/optimal/*.cpp", {unity_group="selector"})
    add_files("./trade_sys/selector/imp/*.cpp", {unity_group="selector_imp"})
    add_files("./trade_sys/signal/*.cpp", "./trade_sys/signal/crt/*.cpp", "./trade_sys/signal/imp/logic/*.cpp", {unity_group="signal"})
    add_files("./trade_sys/signal/imp/*.cpp", {unity_group="signal_imp"})
    add_files("./trade_sys/slippage/**.cpp", {unity_group="slippage"})
    add_files("./trade_sys/stoploss/**.cpp", {unity_group="stoploss"})
    add_files("./trade_sys/system/**.cpp", {unity_group="system"})

    add_files("./utilities/*.cpp", "./utilities/datetime/*.cpp", "./utilities/ini_parser/*.cpp", {unity_group="utilities"})
    add_files("./utilities/thread/*.cpp", "./utilities/db_connect/*.cpp", "./utilities/http_client/*.cpp", {unity_group="utilities"})

    add_files("./data_driver/*.cpp", {unity_group="data_driver"})
    add_files("./data_driver/block_info/qianlong/**.cpp", {unity_group="data_driver"})
    add_files("./data_driver/kdata/cvs/**.cpp", {unity_group="data_driver"})
    if get_config("hdf5") or get_config("sqlite") then
        add_files("./utilities/db_connect/sqlite/**.cpp", {unity_group="sqlite"})
        add_files("./data_driver/base_info/sqlite/**.cpp", {unity_group="sqlite"})
        add_files("./data_driver/block_info/sqlite/**.cpp", {unity_group="sqlite"})
    end
    if get_config("mysql") then
        add_files("./data_driver/base_info/mysql/**.cpp", {unity_group="mysql"})
        add_files("./data_driver/block_info/mysql/**.cpp", {unity_group="mysql"})
    end
    if get_config("sqlite") or get_config("hdf5") then
        add_files("./data_driver/kdata/sqlite/**.cpp", {unity_group="sqlite"})
    end
    if get_config("hdf5") then
        add_files("./data_driver/kdata/hdf5/**.cpp", {unity_group="hdf5"})
    end
    if get_config("mysql") then
        add_files("./data_driver/kdata/mysql/**.cpp", {unity_group="mysql"})
    end
    if get_config("tdx") then
        add_files("./data_driver/kdata/tdx/**.cpp", {unity_group="tdx"})
    end
    if get_config("mysql") then
        add_files("./utilities/db_connect/mysql/**.cpp", {unity_group="mysql"})
    end
    if has_config("ta_lib") then
        add_files("./indicator_talib/**.cpp", {unity_group="talib"})
    end

    after_build(function(target)
        local destpath = get_config("builddir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" .. get_config("arch")
        print(destpath)
        import("core.project.task")
        task.run("copy_dependents", {}, target, destpath, true)
    end)
 
    after_install(function(target)
        local dst_path = target:installdir() .. "/include/hikyuu/python/"
        os.cp("$(projectdir)/hikyuu_pywrap/pybind_utils.h", dst_path)
        os.cp("$(projectdir)/hikyuu_pywrap/pickle_support.h", dst_path)

        local destpath = target:installdir()
        import("core.project.task")
        task.run("copy_dependents", {}, target, destpath, true)
    end)
target_end()
