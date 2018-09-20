option("boost-python-suffix")
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

local cc = get_config("cc")
local cxx = get_config("cxx")
if (cc and string.find(cc, "clang")) or (cxx and string.find(cxx, "clang")) then
    add_cxflags("-Wno-error=parentheses-equality")
end

on_load("xmake_on_load")

target("_hikyuu")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_hikyuu.pyd")
    end
    if is_plat("linux") then
        set_filename("_hikyuu.so")
    end
    if is_plat("macosx") then 
        set_filename("_hikyuu.dylib")
    end
    add_files("./*.cpp")

target("_indicator")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_indicator.pyd")
    end
    if is_plat("linux") then
        set_filename("_indicator.so")
    end
    if is_plat("macosx") then
        set_filename("_indicator.dylib")
    end
    add_files("./indicator/*.cpp")
    
target("_trade_manage")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_manage.pyd")
    end
    if is_plat("linux") then 
        set_filename("_trade_manage.os")
    end
    if is_plat("macosx") then
        set_filename("_trade_manage.dylib")
    end
    add_files("./trade_manage/*.cpp")

target("_trade_sys")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_sys.pyd")
    end
    if is_plat("linux") then
        set_filename("_trade_sys.so")
    end 
    if is_plat("macosx") then
        set_filename("_trade_sys.dylib")
    end
    add_files("./trade_sys/*.cpp")
    
target("_trade_instance")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_instance.pyd")
    end
    if is_plat("linux") then
        set_filename("_trade_instance.so")
    end 
    if is_plat("macosx") then
        set_filename("_trade_instance.dylib")
    end
    add_files("./trade_instance/*.cpp")
    
target("_data_driver")
    set_kind("shared")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_data_driver.pyd")
    end
    if is_plat("linux") then
        set_filename("_data_driver.so")
    end
    if is_plat("macosx") then
        set_filename("_data_driver.dylib")
    end
    add_files("./data_driver/*.cpp")

