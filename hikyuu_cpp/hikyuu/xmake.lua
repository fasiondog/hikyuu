
target("hikyuu")
    set_kind("$(kind)")

    if get_config("leak_check") then
        if is_plat("macosx") then
            add_cxflags("-fsanitize=address")
            add_ldflags("-fsanitize=address")
        elseif is_plat("linux") then
            -- 需要 export LD_PRELOAD=libasan.so
            set_policy("build.sanitizer.address", true)
            set_policy("build.sanitizer.leak", true)
            -- set_policy("build.sanitizer.memory", true)
            -- set_policy("build.sanitizer.thread", true)
        end
    end

    add_packages("boost", "fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json")
    if is_plat("windows", "linux", "cross", "macosx") then
        if get_config("sqlite") or get_config("hdf5") then
            add_packages("sqlite3")
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
        add_links("iconv", "sqlite3")
    end

    add_headerfiles("../(hikyuu/**.h)|**doc.h")

    -- add files
    -- add_files("./**.cpp|data_driver/**.cpp|utilities/db_connect/mysql/*.cpp")
    add_files("./**.cpp|data_driver/**.cpp|utilities/db_connect/mysql/**.cpp|utilities/mo/**.cpp|indicator_talib/**.cpp")
    add_files("./data_driver/*.cpp")
    if get_config("hdf5") or get_config("sqlite") then
        add_files("./data_driver/base_info/sqlite/**.cpp")
        add_files("./data_driver/block_info/sqlite/**.cpp")
    end
    if get_config("mysql") then
        add_files("./data_driver/base_info/mysql/**.cpp")
        add_files("./data_driver/block_info/mysql/**.cpp")
    end
    add_files("./data_driver/block_info/qianlong/**.cpp")
    add_files("./data_driver/kdata/cvs/**.cpp")
    if get_config("sqlite") or get_config("hdf5") then
        add_files("./data_driver/kdata/sqlite/**.cpp")
    end
    if get_config("hdf5") then
        add_files("./data_driver/kdata/hdf5/**.cpp")
    end
    if get_config("mysql") then
        add_files("./data_driver/kdata/mysql/**.cpp")
    end
    if get_config("tdx") then
        add_files("./data_driver/kdata/tdx/**.cpp")
    end
    if get_config("mysql") then
        add_files("./utilities/db_connect/mysql/**.cpp")
    end
    if has_config("mo") then
        add_files("./utilities/mo/**.cpp")
    end
    if has_config("ta_lib") then
        add_files("./indicator_talib/**.cpp")
    end

    after_build(function(target)
        local destpath = get_config("buildir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" .. get_config("arch")
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
