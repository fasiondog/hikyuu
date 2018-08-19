if is_plat("windows") then
    add_defines("HKU_API=__declspec(dllimport)")
end
add_deps("hikyuu")

target("_hikyuu")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_hikyuu.pyd")
    else
        set_filename("_hikyuu.so")
    end
    add_files("./*.cpp")
    add_options(python)
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)

target("_indicator")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_indicator.pyd")
    else
        set_filename("_indicator.so")
    end
    add_files("./indicator/*.cpp")
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)
    
target("_trade_manage")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_manage.pyd")
    else
        set_filename("_trade_manage.so")
    end
    add_files("./trade_manage/*.cpp")
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)

target("_trade_sys")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_sys.pyd")
    else
        set_filename("_trade_sys.so")
    end
    add_files("./trade_sys/*.cpp")
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)
    
target("_trade_instance")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_trade_instance.pyd")
    else
        set_filename("_trade_instance.so")
    end
    add_files("./trade_instance/*.cpp")
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)
    
target("_data_driver")
    set_kind("shared")
    if is_plat("windows") then
        set_filename("_data_driver.pyd")
    else
        set_filename("_data_driver.so")
    end
    add_files("./data_driver/*.cpp")
    on_load(function(target)
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
    end)
    