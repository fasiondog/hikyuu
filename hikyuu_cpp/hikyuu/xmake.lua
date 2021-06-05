target("hikyuu")
    if is_mode("debug") then 
        set_kind("static")
    else
        set_kind("shared")
    end
    
    add_packages("fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json", "cpp-httplib")
    if is_plat("windows") then 
        add_packages("sqlite3")
    end

    add_includedirs("..")

    -- set version for release
    set_configdir("./")
    add_configfiles("$(projectdir)/config.h.in")
    add_configfiles("$(projectdir)/version.h.in")

    if is_plat("windows") then
        add_cxflags("-wd4819")  
        add_cxflags("-wd4251")  --template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834")  --C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4996")
        add_cxflags("-wd4244")  --discable double to int
        add_cxflags("-wd4566")
    else
        add_rpathdirs("$ORIGIN")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end
    
    if is_plat("windows") then 
        add_defines("HKU_API=__declspec(dllexport)")
        if is_mode("release") then
            add_packages("hdf5")
        else
            add_packages("hdf5_D")
        end
        add_packages("mysql")
    end
    
    if is_plat("linux") then
        add_includedirs("/usr/include/hdf5")
        add_includedirs("/usr/include/hdf5/serial")
        if is_arch("x86_64")  then
            if os.exists("/usr/lib64/mysql") then
                add_linkdirs("/usr/lib64/mysql")
            end
            add_linkdirs("/usr/lib/x86_64-linux-gnu")
            add_linkdirs("/usr/lib/x86_64-linux-gnu/hdf5/serial")
        end
    end
    
    if is_plat("macosx") then
        --add_linkdirs("/usr/local/opt/libiconv/lib")
        add_links("iconv")
        add_includedirs("/usr/local/opt/hdf5/include")
        add_linkdirs("/usr/local/opt/hdf5/lib")
        if os.exists("/usr/local/opt/mysql-client") then
            add_includedirs("/usr/local/opt/mysql-client/include")
            add_linkdirs("/usr/local/opt/mysql-client/lib")
            add_rpathdirs("/usr/local/opt/mysql-client/lib")
        end
        if os.exists("/usr/local/mysql/lib") then
            add_includedirs("/usr/local/include")
            add_linkdirs("/usr/local/mysql/lib")
            add_rpathdirs("/usr/local/mysql/lib")
        end
        add_links("mysqlclient")
    end

    if is_plat("windows") then 
        -- nng 静态链接需要的系统库
        add_syslinks("ws2_32", "advapi32")
    end

    if is_plat("linux") or is_plat("macosx") then
        add_links("sqlite3")
        add_links("hdf5")
        add_links("hdf5_hl")
        add_links("hdf5_cpp")
        --add_links("mysqlclient")
        add_links("boost_date_time")
        add_links("boost_filesystem")
        add_links("boost_serialization")
        add_links("boost_system")
    end
    
    -- add files
    add_files("./**.cpp")
    
    add_headerfiles("../(hikyuu/**.h)|**doc.h")

    on_load(function(target)
        assert(os.getenv("BOOST_ROOT"), [[Missing environment variable: BOOST_ROOT
You need to specify where the boost headers is via the BOOST_ROOT variable!]])

        assert(os.getenv("BOOST_LIB"), [[Missing environment variable: BOOST_LIB
You need to specify where the boost library is via the BOOST_LIB variable!]])
    end)

    before_build(function(target)
        if is_plat("macosx") then
            if not os.exists("/usr/local/include/mysql") then
                if os.exists("/usr/local/mysql/include") then
                    os.run("ln -s /usr/local/mysql/include /usr/local/include/mysql")
                else
                    print("Not Found MySQL include dir!")
                end
            end
        end    
    end)

    after_build(function(target)
        if is_plat("linux") then
            os.cp("$(env BOOST_LIB)/libboost_*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        end

        -- 不同平台的库后缀名
        local lib_suffix = ".so"
        if is_plat("windows") then 
            lib_suffix = ".dll"
        elseif is_plat("macosx") then
            lib_suffix = ".dylib"
        end

        local libdir = get_config("buildir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" .. get_config("arch") .. "/lib"
        -- 将依赖的库拷贝至build的输出目录
        for libname, pkg in pairs(target:pkgs()) do
            local pkg_path = pkg:get("includedirs")
            if pkg_path == nil then 
                pkg_path = pkg:get("sysincludedirs") -- xmake 2.3.9 改为了 sysincludedirs
            end
            if pkg_path and type(pkg_path) == "string" then
                pkg_lib_dir = string.sub(pkg_path, 0, string.len(pkg_path)-7) .. "bin"
                if pkg_lib_dir then
                    os.trycp(pkg_lib_dir .. "/*" .. lib_suffix, libdir)
                end
            end
        end
    end)
    
target_end()
