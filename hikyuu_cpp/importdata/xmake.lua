target("importdata")
    set_kind("binary")
    
    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4244")
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("SQLITE_API=__declspec(dllimport)")
    end

    add_deps("hikyuu_utils")

    if is_plat("windows") then
        add_deps("sqlite3")
        add_packages("hdf5")
    end
    
    if is_plat("linux") then
        if is_arch("x86_64") then
            add_linkdirs("/usr/lib/x86_64-linux-gnu")
            add_linkdirs("/usr/lib/x86_64-linux-gnu/hdf5/serial")
        end
        add_includedirs("/usr/include/hdf5")
        add_includedirs("/usr/include/hdf5/serial")
        add_links("sqlite3")
        add_links("hdf5")
        add_links("hdf5_hl")
        add_links("hdf5_cpp")
        add_links("boost_system")
        add_links("boost_filesystem")
        add_links("boost_date_time")
    end
    
    -- add files
    add_files("./*.cpp")
