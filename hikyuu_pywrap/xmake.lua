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
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "cpp-httplib")
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
        -- detect installed python3
        import("lib.detect.find_tool")
        local python = assert(find_tool("python3", {version = true}), "python not found, please install it first! note: python version must > 3.0")
        assert(python.version > "3", python.version .. " python version must > 3.0, please use python3.0 or later!")
        local pyver = python.version:match("%d+.%d+"):gsub("%p+", "")
        if is_plat("windows") then
            -- find python include and libs directory
            local pydir = os.iorun(python.program .. " -c \"import sys; print(sys.executable)\"")
            -- local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
            target:add("links", "boost_python"..pyver.."-mt")            
            return
        else
            if is_plat("macosx") then
                local libdir = os.iorun("python3-config --prefix"):trim() .. "/lib"
                target:add("linkdirs", libdir)
                local out, err = os.iorun("python3 --version")
                local ver = (out .. err):trim():match("%d+.%d+")
                local python_lib = format("python%sm", ver)
                local pyver = tonumber(ver)
                if pyver >= 3.8 then
                    python_lib = format("python%s", ver)
                end
                target:add("links", python_lib)
            end

            -- get python include directory.
            local pydir = try { function () return os.iorun("python3-config --includes"):trim() end }
            assert(pydir, "python3-config not found!")
            target:add("cxflags", pydir)
            target:add("links", "boost_python"..pyver.."-mt")
        end
    end)

    after_build(function(target)
        if is_plat("macosx") then
            local out, err = os.iorun("python3 --version")
            local ver = (out .. err):trim()
            local boost_python_lib = format("libboost_python%s%s.dylib", string.sub(ver,8,8), string.sub(ver,10,10))
            os.run(format("install_name_tool -change @rpath/libhikyuu.dylib @loader_path/libhikyuu.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/libboost_date_time.dylib @loader_path/libboost_date_time.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/libboost_filesystem.dylib @loader_path/libboost_filesystem.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/libboost_system.dylib @loader_path/libboost_system.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/libboost_serialization.dylib @loader_path/libboost_serialization.dylib %s/%s", target:targetdir(), "core.so"))
            os.run(format("install_name_tool -change @rpath/%s @loader_path/%s %s/%s", boost_python_lib, boost_python_lib, target:targetdir(), "core.so"))
        end

        local dst_dir = "$(projectdir)/hikyuu/cpp/"
        if is_plat("windows") then
            os.cp(target:targetdir() .. '/core.pyd', dst_dir)
            os.cp(target:targetdir() .. '/*.dll', dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetdir() .. '/core.so', dst_dir)
            os.cp(target:targetdir() .. '/libhikyuu.dylib', dst_dir)
        else
            os.trycp(target:targetdir() .. '/*.so', dst_dir)
            os.trycp(target:targetdir() .. '/*.so.*', dst_dir)
        end
    end)


