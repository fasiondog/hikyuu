target("test")
    set_kind("binary")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
    end
    
    if is_plat("windows") then
        add_defines("HKU_API=__declspec(dllimport)")
    end

    add_defines("PY_VERSION_HEX=0x03000000")
    add_defines("TEST_ALL_IN_ONE")

    add_deps("hikyuu")

    -- add files
    add_files("**.cpp")

    before_run(function(target)
        print("copying test_data ...")
        os.rm("$(buildir)/$(mode)/$(plat)/$(arch)/lib/test_data")
        os.cp("$(projectdir)/test_data", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end)