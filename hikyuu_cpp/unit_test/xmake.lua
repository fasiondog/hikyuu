target("unit-test")
    set_kind("binary")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
        add_cxflags("-wd4244")
        add_cxflags("-wd4805")
    else
        add_cxflags("-Wno-unused-variable")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") then
        add_defines("HKU_API=__declspec(dllimport)")
    end

    add_defines("PY_VERSION_HEX=0x03000000")
    add_defines("TEST_ALL_IN_ONE")

    add_deps("hikyuu")

    if is_plat("linux") then
        add_links("boost_unit_test_framework")
        --add_links("boost_system")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    add_files("**.cpp")

    before_run(function(target)
        print("copying test_data ...")
        os.rm("$(buildir)/$(mode)/$(plat)/$(arch)/lib/test_data")
        os.cp("$(projectdir)/test_data", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end)

target_end()
