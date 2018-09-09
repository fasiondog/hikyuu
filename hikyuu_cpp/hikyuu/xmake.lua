option("check_bound")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Set indicator check bound")

option("support_serialization")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Set support boost serialization")

option("serialization")
    set_default("XML")
    set_values("XML", "BINARY", "TEXT")
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Set the mode of serialization, before support_serialization is y",
                    "    - XML",
                    "    - BINARAY",
                    "    - TEXT")


target("hikyuu")
    set_kind("shared")
    
    set_config_header("hku_config.h", {prefix = "HKU"})
    add_options("serialization")
    if has_config("support_serialization") then
        add_defines_h("HKU_SUPPORT_SERIALIZATION")
    end
    add_defines_h("HKU_SUPPORT_".."$(serialization)".."_ARCHIVE")
    if has_config("check_bound") then
        add_defines_h("CHECK_ACCESS_BOUND")
    end
    
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
        add_linkdirs("/usr/lib/x86_64-linux-gnu")
        add_links("sqlite3")
        add_links("hdf5_serial")
        add_links("hdf5_serial_hl")
        add_links("hdf5_cpp")
        add_links("mysqlclient")
        add_links("boost_chrono")
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
    
    after_build(function(target)
        if is_plat("windows") then
            os.cp("$(env BOOST_LIB)/boost_chrono*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_date_time*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_filesystem*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_python3*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_serialization*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_system*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_thread*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_unit_test_framework*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/release/$(plat)/$(arch)/*.dll","$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/release/$(plat)/$(arch)/*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")

        else
            local boostlib = val("env BOOST_LIB")
            if boostlib ~= "" then
                os.cp("$(env BOOST_LIB)/libboost_chrono*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_date_time*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_filesystem*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_python3*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_system*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_thread*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
                os.cp("$(env BOOST_LIB)/libboost_unit_test_framework*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            end
        end
    end)
target_end()
