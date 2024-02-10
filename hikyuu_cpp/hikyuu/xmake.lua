
target("hikyuu")
    set_kind("$(kind)")
    -- if is_mode("debug", "coverage", "asan", "msan", "tsan", "lsan") then
    --     set_kind("static")
    -- else
    --     set_kind("shared")
    -- end

    add_options("hdf5", "mysql", "sqlite", "tdx", "feedback", "stacktrace")

    add_packages("boost", "fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json", "cpp-httplib")
    if is_plat("windows", "linux", "cross") then
        add_packages("sqlite3")
    end

    add_includedirs("..")

    -- set version for release
    set_configdir("./")
    add_configfiles("$(projectdir)/config.h.in")
    add_configfiles("$(projectdir)/version.h.in")

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

    if is_plat("windows") then
        if is_kind("shared") then
            add_defines("HKU_API=__declspec(dllexport)")
        end
        if get_config("hdf5") then
            if is_mode("release") then
                add_packages("hdf5")
            else
                add_packages("hdf5_D")
            end
        end
        if get_config("mysql") then
            add_packages("mysql")
        end
    end

    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
        if get_config("hdf5") then
            add_packages("hdf5")
        end
        if get_config("mysql") then
            add_packages("mysql")
        end
    end

    if is_plat("macosx") then
        add_links("iconv", "sqlite3")
        if get_config("mysql") then
            add_packages("mysqlclient")
        end
        if get_config("hdf5") then
            add_includedirs("/usr/local/opt/hdf5/include")
            add_linkdirs("/usr/local/opt/hdf5/lib")
            add_links("hdf5", "hdf5_cpp")
        end
    end

    add_headerfiles("../(hikyuu/**.h)|**doc.h")

    -- add files
    add_files("./**.cpp|data_driver/**.cpp|utilities/db_connect/mysql/*.cpp")
    add_files("./data_driver/*.cpp")
    if get_config("hdf5") or get_config("sqlite") then
        add_files("./data_driver/base_info/sqlite/**.cpp")
    end
    if get_config("mysql") then
        add_files("./data_driver/base_info/mysql/**.cpp")
    end
    add_files("./data_driver/block_info/**.cpp")
    add_files("./data_driver/kdata/cvs/**.cpp")
    if get_config("sqlite") then
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

    after_build(function(target)
        -- 不同平台的库后缀名
        local lib_suffix = ".so"
        if is_plat("windows") then
            lib_suffix = ".dll"
        elseif is_plat("macosx") then
            lib_suffix = ".dylib"
        end

        local libdir = get_config("buildir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" ..
                        get_config("arch") .. "/lib"
        -- 将依赖的库拷贝至build的输出目录
        for libname, pkg in pairs(target:pkgs()) do
            local pkg_path = pkg:installdir()
            if pkg_path ~= nil then
                print("copy dependents: " .. pkg_path)
                os.trycp(pkg_path .. "/bin/*" .. lib_suffix, libdir)
                os.trycp(pkg_path .. "/lib/*" .. lib_suffix, libdir)
                os.trycp(pkg_path .. "/lib/*.so.*", libdir)
            end
        end
    end)

    after_install(function(target)
        local dst_path = target:installdir() .. "/include/hikyuu/python/"
        os.cp("$(projectdir)/hikyuu_pywrap/pybind_utils.h", dst_path)
        os.cp("$(projectdir)/hikyuu_pywrap/pickle_support.h", dst_path)
    end)

target_end()
