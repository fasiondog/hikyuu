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
    
    -- add files
    add_files("./*.cpp")
