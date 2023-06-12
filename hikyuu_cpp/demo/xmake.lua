option("with-demo")
    set_default(false)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Complie with demo")
option_end()

target("demo")
    set_kind("binary")
    if is_mode("debug") then
        set_default(false)
    end
    
    add_packages("boost", "spdlog", "fmt", "flatbuffers")
    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
    end

    if is_plat("windows") and is_mode("release") then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("SQLITE_API=__declspec(dllimport)")
    end
   
    -- add files
    add_files("./*.cpp")

    add_deps("hikyuu")
    
target_end()

