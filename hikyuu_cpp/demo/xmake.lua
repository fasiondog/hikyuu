target("demo")
    set_kind("binary")
    
    if is_plat("windows") then
        add_cxflags("-wd4267")
    end
    
    if is_plat("windows") then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("SQLITE_API=__declspec(dllimport)")
    end
   
    -- add files
    add_files("./*.cpp")

    add_deps("hikyuu")

