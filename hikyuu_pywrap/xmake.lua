
add_requires("pybind11", {system = false, alias = "pybind11"})

target("core")
    set_kind("shared")
    set_default(false)
    -- if is_mode("debug") then 
    --     set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hikyuu才会去编译，但是target还存在，里面的files会保留到vcproj
    --     --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    -- end

    add_deps("hikyuu")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11", "utf8proc", "nlohmann_json")
    if is_plat("windows") then
        set_filename("core.pyd")
        add_cxflags("/bigobj")
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
        add_shflags("-undefined dynamic_lookup", "-headerpad_max_install_names")
    end    

    add_includedirs("../hikyuu_cpp")

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./**.cpp")

    add_files("./*.cpp", {unity_group="base"})
    add_files("./analysis/**.cpp", {unity_group="analysis"})
    add_files("./data_driver/**.cpp", {unity_group="data_driver"})
    add_files("./global/**.cpp", {unity_group="global"})
    add_files("./indicator/**.cpp", {unity_group="indicator"})
    add_files("./plugin/**.cpp", {unity_group="plugin"})
    add_files("./strategy/**.cpp", {unity_group="strategy"})
    add_files("./trade_manage/**.cpp", {unity_group="trade_manage"})
    add_files("./trade_sys/**.cpp", {unity_group="trade_sys"})

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
        local pydir_include = nil;
        local pydir_lib = nil;
        if os.getenv("CONDA_PREFIX") ~= nil then
            print("CONDA_PREFIX: " .. os.getenv("CONDA_PREFIX"))
            local py3config = os.getenv("CONDA_PREFIX") .. "/bin/python3-config"
            pydir_include = os.iorun(py3config .. " --includes"):trim()
            pydir_lib = os.iorun(py3config .. " --libs"):trim()
        else
            local stmt
            if is_plat("macosx") then
                stmt = [[python -c 'import sys; v = sys.version_info; print(f"{str(v.major)}.{str(v.minor)}")']]
            else
                stmt = [[python3 -c 'import sys; v = sys.version_info; print(f"{str(v.major)}.{str(v.minor)}")']]
            end
            local python_version = os.iorun(stmt):trim()
            local py3config = "python" .. python_version .. "-config"
            -- print("py3config: " .. py3config)
            pydir_include = os.iorun(py3config .. " --includes"):trim()
            pydir_lib = os.iorun(py3config .. " --libs"):trim()
        end
        assert(pydir_include, "python3-config not found!")
        print("pydir_include: " .. pydir_include)
        -- print("pydir_lib: " .. pydir_lib)
        target:add("cxflags", pydir_include, pydir_lib)    
    end)

    after_build(function(target)
        local dst_dir = "$(projectdir)/hikyuu/cpp/"
        local dst_obj = dst_dir .. "core.so"

        if not is_plat("cross") then
          local stmt = [[python -c 'import sys; v = sys.version_info; print(str(v.major)+str(v.minor))']]
          if is_plat("linux") then
            stmt = [[python3 -c 'import sys; v = sys.version_info; print(str(v.major)+str(v.minor))']]
          end
          local python_version = os.iorun(stmt):trim()
          print("python_version: " .. python_version)
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
                os.run(format("install_name_tool -change %s @loader_path/%s %s", filename, filename, dst_obj))
            end
            os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
            os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))

            if get_config("mysql") then
                if get_config("kind") == "shared" then
                    dst_obj = dst_dir .. "libhikyuu.dylib"
                    os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
                    os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))
                else
                    os.cp(target:targetdir() .. '/*.a', dst_dir)
                end

                filename = "libmysqlclient.21.dylib"
                os.run(format("install_name_tool -change @loader_path/../lib/libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_dir .. filename))
                os.run(format("install_name_tool -change @loader_path/../lib/libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_dir .. filename))
            end
        end

        os.cp("$(projectdir)/i18n/zh_CN/*.mo", "$(projectdir)/hikyuu/cpp/i18n/zh_CN/")
    end)


