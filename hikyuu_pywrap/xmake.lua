
add_requires("pybind11", {system = false, alias = "pybind11"})

target("core")
    set_kind("shared")
    set_default(false)
    -- if is_mode("debug") then 
    --     set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hikyuu才会去编译，但是target还存在，里面的files会保留到vcproj
    --     --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    -- end

    add_deps("hikyuu")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11")
    if is_plat("windows") then
        set_filename("core.pyd")
        add_cxflags("-wd4251")
    else 
        set_filename("core.so")
    end

    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("-wd4566")
    end
    
    local cc = get_config("cc")
    local cxx = get_config("cxx")
    if (cc and string.find(cc, "clang")) or (cxx and string.find(cxx, "clang")) then
        add_cxflags("-Wno-error=parentheses-equality -Wno-error=missing-braces")
    end

    if is_plat("linux", "cross") then 
        add_rpathdirs("$ORIGIN", "$ORIGIN/cpp")
    end

    if is_plat("macosx") then
        add_linkdirs("/usr/lib")

        -- macosx 下不能主动链接 python，所以需要使用如下编译选项
        add_shflags("-undefined dynamic_lookup")
    end    

    add_includedirs("../hikyuu_cpp")
    add_files("./**.cpp")

    on_load("windows", "linux", "macosx", function(target)
        import("lib.detect.find_tool")
        if is_plat("windows") then
            -- detect installed python3
            local python = assert(find_tool("python", {version = true}), "python not found, please install it first! note: python version must > 3.0")
            assert(python.version > "3", python.version .. " python version must > 3.0, please use python3.0 or later!")
            -- find python include and libs directory
            local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            pydir = path.directory(pydir)
            if pydir:endswith("Scripts") then
                -- if venv is activated, find the real python directory
                file = io.open(pydir .. "/../pyvenv.cfg", "r")
                for line in file:lines() do
                    if string.find(line, "home =") then
                        -- 使用 string.gmatch 函数抽取路径
                        for path in string.gmatch(line, "home = (.*)") do
                            pydir = path
                        end
                    end
                end
                file:close()
            end            
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
            return
        end
    
        -- get python include directory.
        local pydir = try { function () return os.iorun("python3-config --includes"):trim() end }
        target:add("cxflags", pydir)
    end)

    after_build(function(target)
        local dst_dir = "$(projectdir)/hikyuu/cpp/"
        local dst_obj = dst_dir .. "core.so"

        -- need xmake 445e43b40846b29b9abb1293b32b27b7104f54fa
        if not is_plat("cross") then
          local stmt = [[python -c 'import sys; v = sys.version_info; print(str(v.major)+str(v.minor))']]
          local python_version = os.iorun(stmt):trim()
          dst_obj = dst_dir .. "core" ..  python_version
        end

        if is_plat("windows") then
            os.cp(target:targetdir() .. '/core.pyd', dst_obj .. ".pyd")
            os.cp(target:targetdir() .. '/*.dll', dst_dir)
            os.cp(target:targetdir() .. '/hikyuu.lib', dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetdir() .. '/core.so', dst_obj .. ".so")
            os.cp(target:targetdir() .. '/*.dylib', dst_dir)
        else
            os.trycp(target:targetdir() .. '/*.so', dst_dir)
            os.trycp(target:targetdir() .. '/*.so.*', dst_dir)
            if not is_plat("cross") then
                os.trymv(dst_dir .. '/core.so', dst_obj .. ".so")
            end
        end

        if is_plat("macosx") then
            dst_obj = dst_obj .. ".so"
            for _, filepath in ipairs(os.files(dst_dir .. "/*.dylib")) do
                -- print(path.filename(filepath))
                local filename = path.filename(filepath)
                os.run(format("install_name_tool -change @rpath/%s @loader_path/%s %s", filename, filename, dst_obj))
            end
            os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
            os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))

            if get_config("kind") == "shared" then
                dst_obj = dst_dir .. "libhikyuu.dylib"
                os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
                os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))
            -- else
            --     os.cp(target:targetdir() .. '/libhikyuu.a', dst_dir)
            end

            filename = "libmysqlclient.21.dylib"
            os.run(format("install_name_tool -change @loader_path/../lib/libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_dir .. filename))
            os.run(format("install_name_tool -change @loader_path/../lib/libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_dir .. filename))
        end
    end)


