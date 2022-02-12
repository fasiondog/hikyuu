import("lib.detect.find_tool")

function main(target)
    if is_plat("windows") then
        -- detect installed python3
        local python = assert(find_tool("python3", {version = true}), "python not found, please install it first! note: python version must > 3.0")
        assert(python.version > "3", python.version .. " python version must > 3.0, please use python3.0 or later!")

        -- find python include and libs directory
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
        return
    end

    target:add("rpathdirs", "$ORIGIN", "$ORIGIN/lib", "$ORIGIN/../lib")
    
    if is_plat("macosx") then
        local libdir = os.iorun("python3-config --prefix"):trim() .. "/lib"
        target:add("linkdirs", libdir)
        local out, err = os.iorun("python3 --version")
        local ver = (out .. err):trim()
        local python_lib = format("python%s.%sm", string.sub(ver,8,8), string.sub(ver,10,10))
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
end
