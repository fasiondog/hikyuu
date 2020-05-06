option("boost-python-suffix")
    set_default("3X")
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Set suffix of libboost_python. ",
                    "Boost.python 1.67 later use 3x like libboost_python35, ",
                    "but older is libboost_python3",
                    "    - 3X autocheck for 35, 36, 37, 3x")
option_end()

add_includedirs("../hikyuu_cpp")

-- Can't use static boost.python lib, the function that using 'arg' will load failed!
--add_defines("BOOST_PYTHON_STATIC_LIB")

if is_plat("windows") then
    add_defines("HKU_API=__declspec(dllimport)")
    add_cxflags("-wd4566")
end

local cc = get_config("cc")
local cxx = get_config("cxx")
if (cc and string.find(cc, "clang")) or (cxx and string.find(cxx, "clang")) then
    add_cxflags("-Wno-error=parentheses-equality -Wno-error=missing-braces")
end

on_load("xmake_on_load")

target("_hikyuu")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hikyuu才会去编译，但是target还存在，里面的files会保留到vcproj
        --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_hikyuu.pyd")
    else 
        set_filename("_hikyuu.so")
    end
    add_cxflags("-wd4251")
    add_files("./*.cpp")
    add_files("./task/*.cpp")

target("_indicator")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false)
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_indicator.pyd")
    else
        set_filename("_indicator.so")
    end
    add_files("./indicator/*.cpp")
    
target("_trade_manage")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false)
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_manage.pyd")
    else 
        set_filename("_trade_manage.so")
    end
    add_files("./trade_manage/*.cpp")

target("_trade_sys")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false)
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_sys.pyd")
    else
        set_filename("_trade_sys.so")
    end
    add_files("./trade_sys/*.cpp")
    
target("_trade_instance")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false)
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_trade_instance.pyd")
    else 
        set_filename("_trade_instance.so")
    end
    add_files("./trade_instance/*.cpp")
    
target("_data_driver")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false)
    end
    add_packages("fmt", "spdlog")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("_data_driver.pyd")
    else 
        set_filename("_data_driver.so")
    end
    add_files("./data_driver/*.cpp")

