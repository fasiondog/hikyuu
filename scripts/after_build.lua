import("core.platform.platform")
import("core.base.option")

function main(target)
    if target:isphony() then
        return 
    end

    local targetname = target:name()
    local targetfile = target:targetfile()

    if "sqlite3" == targetname then
        os.cp(targetfile, './hikyuu')
        return
    end

    if "hikyuu_utils" == targetname then
        os.cp(targetfile, './hikyuu')
        return
    end

    if "hikyuu" == targetname then
        os.cp(targetfile, './hikyuu')
        if is_plat("windows") then
            os.cp("$(env BOOST_LIB)/boost_python3*.dll", './hikyuu')
            os.cp("$(env BOOST_LIB)/boost_serialization*.dll", './hikyuu')
            if is_mode("release") then
                os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", './hikyuu')
            else
                os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5_D.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", './hikyuu')
            end
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", './hikyuu')
            return
        end

        if is_plat("linux") then
            os.cp("$(env BOOST_LIB)/libboost_python3*.so.*", './hikyuu')
            os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", './hikyuu')
            return
        end

        if is_plat("macosx") then
            os.cp("$(env BOOST_LIB)/libboost_python3*.dylib", './hikyuu')
            os.cp("$(env BOOST_LIB)/libboost_serialization*.dylib", './hikyuu')
            return
        end

        return
    end

    if "importdata" == targetname then
        os.cp(targetfile, './hikyuu')
        return
    end

    if "_hikyuu" == targetname then
        os.cp(targetfile, './hikyuu')
        return
    end

    if "_indicator" == targetname then
        os.cp(targetfile, './hikyuu/indicator')
        return
    end

    if "_trade_manage" == targetname then
        os.cp(targetfile, './hikyuu/trade_manage')
        return
    end

    if "_trade_sys" == targetname then
        os.cp(targetfile, './hikyuu/trade_sys')
        return
    end

    if "_trade_instance" == targetname then
        os.cp(targetfile, './hikyuu/trade_instance')
        return
    end

    if "_data_driver" == targetname then
        os.cp(targetfile, './hikyuu/data_driver')
        return
    end

end