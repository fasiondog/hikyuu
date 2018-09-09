option("boost_python_suffix")
    set_default("3X")
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Set suffix of libboost_python. ",
                    "Boost.python 1.67 later use 3x like libboost_python35, ",
                    "but older is libboost_python3",
                    "    - 3X autocheck for 35, 36, 37, 3x")
option_end()

if is_plat("windows") then
    add_defines("HKU_API=__declspec(dllimport)")
end

target("_hikyuu")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_hikyuu.pyd")
    else
        set_filename("_hikyuu.so")
        --add_rpathdirs("$ORIGIN/lib")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/lib")
    end

    add_files("./*.cpp")

    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
        end
    end)

target("_indicator")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_indicator.pyd")
    else
        set_filename("_indicator.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
    add_files("./indicator/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
        end
    end)
    
target("_trade_manage")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_manage.pyd")
    else
        set_filename("_trade_manage.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
    add_files("./trade_manage/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
        end
    end)

target("_trade_sys")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_sys.pyd")
    else
        set_filename("_trade_sys.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
    add_files("./trade_sys/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
            end
        end
    end)
    
target("_trade_instance")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_instance.pyd")
    else
        set_filename("_trade_instance.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
    add_files("./trade_instance/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
        end
    end)
    
target("_data_driver")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_data_driver.pyd")
    else
        set_filename("_data_driver.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
    add_files("./data_driver/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            -- get python include directory.
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("cxflags", pydir)

            -- get suffix configure for link libboost_pythonX.so
            import("core.project.config")
            local suffix = config.get("boost_python_suffix")
            suffix = string.upper(suffix)
            if suffix == "3X" then
                local ver = os.iorun("python3 --version")
                local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
                target:add("links", boost_python_lib)
            else
                local boost_python_lib = "libboost_python"..suffix..".so"
                local errinfo = "Can't find "..boost_python_lib
                errinfo = errinfo .. "\nYou need to use --boost_python_suffix specify libboost_python suffix"
                errinfo = errinfo .. '\nmore see "xmake f --help"'
                raise(errinfo)
            end
        end
    end)
    
