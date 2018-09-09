option("with-demo")
    set_default(false)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Complie with demo")
option_end()

target("demo")
    if has_config("with-demo") then
        set_kind("binary")
    else
        set_kind("phony")
    end
    
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
target_end()

