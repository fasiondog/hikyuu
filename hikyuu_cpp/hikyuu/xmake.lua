option("check_bound")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu_options")

option("support_serialization")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu_options")

option("serialization")
    set_default("XML")
    set_values("XML", "BINARY", "TEXT")
    set_showmenu(true)
    set_category("hikyuu_options")


target("hikyuu")
    set_kind("shared")
    
    set_config_header("hku_config.h", {prefix = "HKU"})
    add_options("serialization")
    if is_option("support_serialization") then
        add_defines_h("HKU_SUPPORT_SERIALIZATION")
    end
    add_defines_h("HKU_SUPPORT_".."$(serialization)".."_ARCHIVE")
    if is_option("check_bound") then
        add_defines_h("CHECK_ACCESS_BOUND")
    end
    
    if is_plat("windows") then
        add_cxflags("-wd4819")  
        add_cxflags("-wd4251")  --template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834")  --C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4996")
        add_cxflags("-wd4244")  --discable double to int
    end
    
    if is_plat("windows") then 
        add_defines("SQLITE_API=__declspec(dllimport)")
        add_defines("HKU_API=__declspec(dllexport)")
        add_defines("PY_VERSION_HEX=0x03000000")
        --add_links("python35")
    end
                
    add_deps("hikyuu_utils")
    add_packages("spdlog")
    
    if is_plat("windows") then
        add_deps("sqlite3")
        add_packages("hdf5")
        add_packages("mysql")
    else
        add_links("sqlite3")
        add_links("hdf5_serial")
        add_links("hdf5_serial_hl")
        add_links("hdf5_cpp")
    end
    
    -- add files
    add_files("./**.cpp")
    
    add_headers("../(hikyuu/**.h)|**doc.h")
    
    after_build(function(target)
        if is_plat("windows") then
            os.cp("$(env BOOST_LIB)/boost_chrono*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_date_time*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_filesystem*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_log*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_python*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_serialization*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_system*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_thread*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(env BOOST_LIB)/boost_unit_test_framework*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/release/$(plat)/$(arch)/*.dll","$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/release/$(plat)/$(arch)/*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        end
    end)