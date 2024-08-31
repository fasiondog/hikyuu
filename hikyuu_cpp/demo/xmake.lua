target("demo1")
    set_kind("binary")
    set_default(false)
    
    add_packages("boost", "spdlog", "fmt")
    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
    end

    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
        add_defines("SQLITE_API=__declspec(dllimport)")
    end
   
    add_deps("hikyuu")

    add_files("./demo1.cpp")
target_end()


target("demo2")
    set_kind("binary")
    set_default(false)
    
    add_packages("boost", "spdlog", "fmt")
    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
    end

    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
        add_defines("SQLITE_API=__declspec(dllimport)")
    end
   
    add_deps("hikyuu")

    add_files("./demo2.cpp")
target_end()
