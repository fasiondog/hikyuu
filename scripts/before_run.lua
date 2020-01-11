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
        os.cp("$(env BOOST_LIB)/boost_serialization*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll","$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
        os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end
    
    if is_plat("linux") then
        os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end    

    if is_plat("macosx") then
        os.cp("$(env BOOST_LIB)/libboost_serialization*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end    
    
end