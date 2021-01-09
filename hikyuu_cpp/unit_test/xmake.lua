option("test")
    set_default("small")
    set_values("small", "all")
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Complie with unit-test")
option_end()

add_requires("doctest")

target("unit-test")
    set_kind("binary")
    if get_config("test") == "small" then
        set_default(false)
    end

    add_packages("fmt", "spdlog", "doctest", "mysql")

    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
        add_cxflags("-wd4244")
        add_cxflags("-wd4805")
        add_cxflags("-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") and is_mode("release") then
        add_defines("HKU_API=__declspec(dllimport)")
    end

    add_defines("TEST_ALL_IN_ONE")

    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_links("boost_unit_test_framework")
        --add_links("boost_system")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    add_files("**.cpp")
    
target_end()

target("small-test")
    set_kind("binary")
    if get_config("test") == "all" then
        set_default(false)
    end
    add_packages("fmt", "spdlog", "doctest", "mysql")
    add_includedirs("..")

    --add_defines("BOOST_TEST_DYN_LINK")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
        add_cxflags("-wd4244")
        add_cxflags("-wd4805")
        add_cxflags("-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") and is_mode("release") then
        add_defines("HKU_API=__declspec(dllimport)")
    end

    add_defines("TEST_ALL_IN_ONE")

    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_links("boost_unit_test_framework")
        --add_links("boost_system")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    add_files("./hikyuu/hikyuu/**.cpp");
    add_files("./hikyuu/test_main.cpp")
    add_files("./hikyuu/utilities/test_TimerManager.cpp")
    
target_end()