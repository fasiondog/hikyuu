option("test")
    set_default("small")
    set_values("small", "all")
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Complie with unit-test")
option_end()

add_requires("doctest")

target("unit-test")
    if get_config("test") == "all" then
        set_kind("binary")
    else
        set_kind("phony")
    end

    add_packages("fmt", "spdlog", "doctest")

    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
        add_cxflags("-wd4244")
        add_cxflags("-wd4805")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") then
        add_defines("HKU_API=__declspec(dllimport)")
    end

    add_defines("PY_VERSION_HEX=0x03000000")
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
    if get_config("test") == "small" then
        set_kind("binary")
    else
        set_kind("phony")
    end
    add_packages("fmt", "spdlog", "doctest")
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

    add_defines("PY_VERSION_HEX=0x03000000")
    add_defines("TEST_ALL_IN_ONE")

    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_links("boost_unit_test_framework")
        --add_links("boost_system")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    --add_files("./hikyuu/hikyuu/**.cpp");
    add_files("./hikyuu/hikyuu/test_StockManager.cpp");
    add_files("./hikyuu/test_main.cpp")
    
target_end()