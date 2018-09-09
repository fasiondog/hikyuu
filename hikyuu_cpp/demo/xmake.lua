option("with-demo")
    set_default(false)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Complie with demo")
option_end()

target("demo")
    if is_option("with-demo") then
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
    
    before_run(function(target)
        import("core.project.config")
        local with_demo = config.get("with-demo")
        if not with_demo then
            raise("You need to config first: xmake f --with-demo=y")
        end
    end)
target_end()

