
add_deps("hikyuu")

if is_plat("windows") then
    add_defines("HKU_API=__declspec(dllimport)")
end

target("_hikyuu")
    set_kind("shared")
    --add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_hikyuu.pyd")
    else
        set_filename("_hikyuu.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/lib")
        add_links("boost_python3")
    end

    add_files("./*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3-config --includes")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
            local libs = os.iorun("python3-config --libs")
            local lcPos = string.find(libs,"\n")
            libs = (string.sub(libs,1,lcPos-1))
            target:add("shflags", libs)
        end
    end)

target("_indicator")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_indicator.pyd")
    else
        set_filename("_indicator.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
        add_links("boost_python3")
    end
    add_files("./indicator/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3 -c \"import sysconfig; print(sysconfig.get_path('include'))\"")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
        end
    end)
    
target("_trade_manage")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_manage.pyd")
    else
        set_filename("_trade_manage.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
        add_links("boost_python3")
    end
    add_files("./trade_manage/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3 -c \"import sysconfig; print(sysconfig.get_path('include'))\"")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
        end
    end)

target("_trade_sys")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_sys.pyd")
    else
        set_filename("_trade_sys.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
        add_links("boost_python3")
    end
    add_files("./trade_sys/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3 -c \"import sysconfig; print(sysconfig.get_path('include'))\"")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
        end
    end)
    
target("_trade_instance")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_instance.pyd")
    else
        set_filename("_trade_instance.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
        add_links("boost_python3")
    end
    add_files("./trade_instance/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3 -c \"import sysconfig; print(sysconfig.get_path('include'))\"")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
        end
    end)
    
target("_data_driver")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_data_driver.pyd")
    else
        set_filename("_data_driver.so")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
        add_links("boost_python3")
    end
    add_files("./data_driver/*.cpp")
    on_load(function(target)
        if is_plat("windows") then
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
        else
            local pydir = os.iorun("python3 -c \"import sysconfig; print(sysconfig.get_path('include'))\"")
            local lcPos = string.find(pydir,"\n")
            pydir = (string.sub(pydir,1,lcPos-1))
            target:add("includedirs", pydir)
        end
    end)
    
