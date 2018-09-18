import("core.platform.platform")
import("core.base.option")

function main(target)
    if target:isphony() then
        return 
    end

    local targetname = target:name()
    if "demo" == targetname or "unit-test" == targetname then
        return
    end

    local installdir = option.get("installdir") or os.getenv("INSTALLDIR") or os.getenv("DESTDIR") or platform.get("installdir")
    local targetfile = target:targetfile()
    if "_data_driver" == targetname then
        os.cp(targetfile, installdir .. "/data_driver/")
        return
    end

    if "_indicator" == targetname then
        os.cp(targetfile, installdir .. "/indicator/")
        return
    end

    if "_trade_manage" == targetname then
        os.cp(targetfile, installdir .. "/trade_manage/")
        return
    end
    
    if "_trade_sys" == targetname then
        os.cp(targetfile, installdir .. "/trade_sys/")
        return
    end

    if "_trade_instance" == targetname then
        os.cp(targetfile, installdir .. "/trade_instance/")
        return
    end
    
    os.cp(targetfile, installdir .. "/")
    
    if "hikyuu" == targetname then
        if is_plat("windows") then
            os.cp("$(env BOOST_LIB)/boost_date_time*.dll", installdir .. "/")
            os.cp("$(env BOOST_LIB)/boost_filesystem*.dll", installdir .. "/")
            os.cp("$(env BOOST_LIB)/boost_python3*.dll", installdir .. "/")
            os.cp("$(env BOOST_LIB)/boost_serialization*.dll", installdir .. "/")
            os.cp("$(env BOOST_LIB)/boost_system*.dll", installdir .. "/")
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/release/$(plat)/$(arch)/*.dll", installdir .. "/")
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/release/$(plat)/$(arch)/*.dll", installdir .. "/")
            return
        end

        if is_plat("linux") then
            os.cp("$(env BOOST_LIB)/libboost_date_time*.so.*", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_filesystem*.so.*", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_python3*.so.*", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_system*.so.*", installdir .. "/")
            return
        end

        if is_plat("macosx") then
            os.cp("$(env BOOST_LIB)/libboost_date_time*.dylib", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_filesystem*.dylib", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_python3*.dylib", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_serialization*.dylib", installdir .. "/")
            os.cp("$(env BOOST_LIB)/libboost_system*.dylib", installdir .. "/")
            return
        end
    end
end
