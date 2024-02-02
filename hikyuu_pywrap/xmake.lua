
add_requires("pybind11", {system = false, alias = "pybind11"})

target("core")
    set_kind("shared")
    set_default(false)
    -- if is_mode("debug") then 
    --     set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hikyuu才会去编译，但是target还存在，里面的files会保留到vcproj
    --     --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    -- end

    add_options("stacktrace")

    add_deps("hikyuu")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11", "cpp-httplib")
    if is_plat("windows") then
        set_filename("core.pyd")
        add_cxflags("-wd4251")
    else 
        set_filename("core.so")
    end

    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_cxflags("-wd4566")
    end
    
    local cc = get_config("cc")
    local cxx = get_config("cxx")
    if (cc and string.find(cc, "clang")) or (cxx and string.find(cxx, "clang")) then
        add_cxflags("-Wno-error=parentheses-equality -Wno-error=missing-braces")
    end

    add_includedirs("../hikyuu_cpp")
    add_files("./**.cpp")

    add_rpathdirs("$ORIGIN", "$ORIGIN/lib", "$ORIGIN/../lib")

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
        local pydir = nil;
        if os.getenv("CONDA_PREFIX") ~= nil then
          local py3config = os.getenv("CONDA_PREFIX") .. "/bin/python3-config"
          pydir = try { function () return os.iorun(py3config .. " --includes"):trim() end }
        else
          pydir = try { function () return os.iorun("python3-config --includes"):trim() end }
        end
        assert(pydir, "python3-config not found!")
        target:add("cxflags", pydir)   
    end)

    after_build(function(target)
        if is_plat("macosx") then
            os.run(format("install_name_tool -change @rpath/libhikyuu.dylib @loader_path/libhikyuu.dylib %s/%s", target:targetdir(), "core.so"))
        end

        local dst_dir = "$(projectdir)/hikyuu/cpp/"
        local dst_obj = dst_dir .. "core.so"
        if not is_plat("cross") then
            import("lib.detect.find_tool")
            local python = assert(find_tool("python", {version = true}), "python not found, please install it first! note: python version must > 3.0")
            local tmp = string.split(python.version, "%.")
            dst_obj = dst_dir .. "core" .. tmp[1] .. tmp[2]
        end
        -- print(dst_obj)

        if is_plat("windows") then
            os.cp(target:targetdir() .. '/core.pyd', dst_obj .. ".pyd")
            os.cp(target:targetdir() .. '/*.dll', dst_dir)
            os.cp(target:targetdir() .. '/hikyuu.lib', dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetdir() .. '/core.so', dst_obj .. ".so")
            os.cp(target:targetdir() .. '/*.dylib', dst_obj)
        else
            os.trycp(target:targetdir() .. '/*.so', dst_dir)
            os.trycp(target:targetdir() .. '/*.so.*', dst_dir)
            if not is_plat("cross") then
                os.trymv(target:targetdir() .. '/core.so', dst_obj .. ".so")
            end
        end
    end)


