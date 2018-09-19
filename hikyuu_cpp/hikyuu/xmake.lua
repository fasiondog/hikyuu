target("hikyuu")
    set_kind("shared")
    
    -- set version for release
    set_config_header("version.h", {prefix = "HKU"})

    add_deps("hikyuu_utils")

    if is_plat("windows") then
        add_cxflags("-wd4819")  
        add_cxflags("-wd4251")  --template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834")  --C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4996")
        add_cxflags("-wd4244")  --discable double to int
    else
        add_rpathdirs("$ORIGIN")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") then 
        add_defines("SQLITE_API=__declspec(dllimport)")
        add_defines("HKU_API=__declspec(dllexport)")
        add_defines("PY_VERSION_HEX=0x03000000")
        add_deps("sqlite3")
        add_packages("hdf5")
        add_packages("mysql")
    end
    
    if is_plat("linux") then
        add_includedirs("/usr/include/hdf5")
        add_includedirs("/usr/include/hdf5/serial")
        if is_arch("x86_64")  then
            add_linkdirs("/usr/lib/x86_64-linux-gnu")
            add_linkdirs("/usr/lib/x86_64-linux-gnu/hdf5/serial")
        end
    end
    
    if is_plat("macosx") then
        --add_linkdirs("/usr/local/opt/libiconv/lib")
        add_links("iconv")
        add_includedirs("/usr/local/opt/hdf5/include")
        add_linkdirs("/usr/local/opt/hdf5/lib")
        add_includedirs("/usr/local/opt/mysql-client/include")
        add_linkdirs("/usr/local/opt/mysql-client/lib")
    end

    if is_plat("linux") or is_plat("macosx") then
        add_links("sqlite3")
        add_links("hdf5")
        add_links("hdf5_hl")
        add_links("hdf5_cpp")
        add_links("mysqlclient")
        add_links("boost_date_time")
        add_links("boost_filesystem")
        add_links("boost_serialization")
        add_links("boost_system")
    end
    
    -- add files
    add_files("./**.cpp")
    
    add_headers("../(hikyuu/**.h)|**doc.h")

    on_load(function(target)
        assert(os.getenv("BOOST_ROOT"), [[Missing environment variable: BOOST_ROOT
You need to specify where the boost headers is via the BOOST_ROOT variable!]])

        assert(os.getenv("BOOST_ROOT"), [[Missing environment variable: BOOST_LIB
You need to specify where the boost library is via the BOOST_LIB variable!]])
    end)
    
target_end()
