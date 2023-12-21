target("demo")
    set_kind("binary")
    set_default(false)
    
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

