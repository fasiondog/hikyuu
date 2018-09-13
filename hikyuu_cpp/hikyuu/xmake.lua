target("hikyuu")
    set_kind("shared")
    
    -- set version for release
    set_config_header("version.h", {prefix = "HKU"})
    
    if is_plat("windows") then
        add_cxflags("-wd4819")  
        add_cxflags("-wd4251")  --template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834")  --C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4996")
        add_cxflags("-wd4244")  --discable double to int
    else
        add_shflags("-Wl,-rpath=./", {force = true})
        --add_rpathdirs("@loader_path")
        add_cxflags("-Wno-sign-compare")
    end

    add_deps("hikyuu_utils")
    
    if is_plat("windows") then 
        add_defines("SQLITE_API=__declspec(dllimport)")
        add_defines("HKU_API=__declspec(dllexport)")
        add_defines("PY_VERSION_HEX=0x03000000")
    end
                
    if is_plat("windows") then
        add_deps("sqlite3")
        add_packages("hdf5")
        add_packages("mysql")
    else
        if is_arch("x86_64") then
            add_linkdirs("/usr/lib/x86_64-linux-gnu")
        end
        add_includedirs("/usr/include/hdf5")
        add_includedirs("/usr/include/hdf5/serial")
        add_links("sqlite3")
        add_links("hdf5_serial")
        add_links("hdf5_serial_hl")
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
        local boostroot = val("env BOOST_ROOT")
        if boostroot == "" then
            local info = "Missing environment variable: BOOST_ROOT\n"
            local desc = "You need to specify where the boost headers is via the BOOST_ROOT variable!"
            raise(info..desc)
        end
        local boostlib = val("env BOOST_LIB")
        if boostlib == "" then
            local info = "Missing environment variable: BOOST_LIB\n"
            local desc = "You need to specify where the boost library is via the BOOST_LIB variable!"
            raise(info..desc)
        end
    end)
    
target_end()
