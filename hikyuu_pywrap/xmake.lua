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

--on_load("xmake_on_load")

target("core")
    set_kind("shared")
    if is_mode("debug") then 
        set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hikyuu才会去编译，但是target还存在，里面的files会保留到vcproj
        --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    end
    add_packages("fmt", "spdlog", "flatbuffers")
    add_deps("hikyuu")
    if is_plat("windows") then
        set_filename("core.pyd")
        add_cxflags("-wd4251")
    else 
        set_filename("core.so")
    end
    add_files("./**.cpp")

    add_rpathdirs("$ORIGIN", "$ORIGIN/lib", "$ORIGIN/../lib")

    on_load(function(target)
        import("lib.detect.find_tool")
        if is_plat("windows") then
            -- detect installed python3
            local python = assert(find_tool("python", {version = true}), "python not found, please install it first! note: python version must > 3.0")
            assert(python.version > "3", python.version .. " python version must > 3.0, please use python3.0 or later!")

            -- find python include and libs directory
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
            return
        end

        if is_plat("macosx") then
            local libdir = os.iorun("python3-config --prefix"):trim() .. "/lib"
            target:add("linkdirs", libdir)
            local out, err = os.iorun("python3 --version")
            local ver = (out .. err):trim()
            --local python_lib = format("python%s.%sm", string.sub(ver,8,8), string.sub(ver,10,10))
            local python_lib = format("python%s.%s", string.sub(ver,8,8), string.sub(ver,10,10))
            target:add("links", python_lib)
        end

        -- get python include directory.
        local pydir = try { function () return os.iorun("python3-config --includes"):trim() end }
        assert(pydir, "python3-config not found!")
        target:add("cxflags", pydir)

        -- get suffix configure for link libboost_pythonX.so
        local suffix = get_config("boost-python-suffix")
        if suffix == nil then
            raise("You need to config --boost-python-suffix specify libboost_python suffix")
        end

        suffix = string.upper(suffix)
        if suffix == "3X" then
            local out, err = os.iorun("python3 --version")
            local ver = (out .. err):trim()
            local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
            target:add("links", boost_python_lib)
        else
            target:add("links", "boost_python"..suffix)
        end    
    end)

    after_build(function(target)
        if is_plat("macosx") then
            local out, err = os.iorun("python3 --version")
            local ver = (out .. err):trim()
            local boost_python_lib = format("libboost_python%s%s.dylib", string.sub(ver,8,8), string.sub(ver,10,10))
            os.run(format("install_name_tool -change @rpath/libhikyuu.dylib @loader_path/libhikyuu.dylib %s/%s", target:targetdir(), "core.so"))
            --os.run(format("install_name_tool -change @rpath/libboost_date_time.dylib @loader_path/libboost_date_time.dylib %s/%s", target:targetdir(), "core.so"))
            --os.run(format("install_name_tool -change @rpath/libboost_filesystem.dylib @loader_path/libboost_filesystem.dylib %s/%s", target:targetdir(), "core.so"))
            --os.run(format("install_name_tool -change @rpath/libboost_system.dylib @loader_path/libboost_system.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/libboost_serialization.dylib @loader_path/libboost_serialization.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/%s @loader_path/%s %s/%s", boost_python_lib, boost_python_lib, target:targetdir(), "core.so"))
        end

        local dst_dir = "$(projectdir)/hikyuu/cpp/"
        if is_plat("windows") then
            os.cp(target:targetdir() .. '/core.pyd', dst_dir)
            os.cp(target:targetdir() .. '/hikyuu.dll', dst_dir)
            os.cp(target:targetdir() .. '/sqlite3.dll', dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetdir() .. '/core.so', dst_dir)
            os.cp(target:targetdir() .. '/libhikyuu.dylib', dst_dir)
        else
            os.cp(target:targetdir() .. '/core.so', dst_dir)
            os.cp(target:targetdir() .. '/libhikyuu.so', dst_dir)
        end

        --os.cp("$(env BOOST_LIB)/boost_date_time*.dll", dst_dir)
        --os.cp("$(env BOOST_LIB)/boost_filesystem*.dll", dst_dir)
        os.cp("$(env BOOST_LIB)/boost_python3*.dll", dst_dir)
        os.cp("$(env BOOST_LIB)/boost_serialization*.dll", dst_dir)
        --os.cp("$(env BOOST_LIB)/boost_system*.dll", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_date_time*.so.*", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_filesystem*.so.*", dst_dir)
        os.cp("$(env BOOST_LIB)/libboost_python3*.so.*", dst_dir)
        os.cp("$(env BOOST_LIB)/libboost_serialization*.so.*", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_system*.so.*", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_date_time*.dylib", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_filesystem*.dylib", dst_dir)
        os.cp("$(env BOOST_LIB)/libboost_python3*.dylib", dst_dir)
        os.cp("$(env BOOST_LIB)/libboost_serialization*.dylib", dst_dir)
        --os.cp("$(env BOOST_LIB)/libboost_system*.dylib", dst_dir)

        if is_plat("windows") then
            if is_mode("release") then
                os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", dst_dir)
            else
                os.cp("$(projectdir)/hikyuu_extern_libs/pkg/hdf5_D.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", dst_dir)
            end
            os.cp("$(projectdir)/hikyuu_extern_libs/pkg/mysql.pkg/lib/$(mode)/$(plat)/$(arch)/*.dll", dst_dir)
        end
    end)


