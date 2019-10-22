import("core.project.config")

function main(target)
    local targetname = target:name()
    if targetname ~= "demo" and targetname ~= "unit-test" and targetname ~= "small-test" then
        return
    end
    
    if "demo" == targetname then
        local with_demo = config.get("with-demo")
        if not with_demo then
            raise("You need to config first: xmake f --with-demo=y")
        end
    end
    
    if "unit-test" == targetname or "small-test" == targetname then
        print("copying test_data ...")
        os.rm("$(buildir)/$(mode)/$(plat)/$(arch)/lib/test_data")
        os.cp("$(projectdir)/test_data", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end
    
    if is_plat("windows") then
        --os.cp("$(env BOOST_LIB)/boost_date_time*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/boost_filesystem*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(env BOOST_LIB)/boost_serialization*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/boost_system*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/boost_unit_test_framework*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/release/$(plat)/$(arch)/*.dll","$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/release/$(plat)/$(arch)/*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end
    
    if is_plat("linux") then
        --os.cp("$(env BOOST_LIB)/libboost_date_time*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_filesystem*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(env BOOST_LIB)/libboost_system*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_unit_test_framework*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end    

    if is_plat("macosx") then
        --os.cp("$(env BOOST_LIB)/libboost_date_time*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_filesystem*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(env BOOST_LIB)/libboost_serialization*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_system*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_unit_test_framework*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        -- macOSX下unit_test_framework依赖于timer和chrono
        --os.cp("$(env BOOST_LIB)/libboost_timer*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        --os.cp("$(env BOOST_LIB)/libboost_chrono*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end    
    
end