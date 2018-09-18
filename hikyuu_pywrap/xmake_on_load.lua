
function main(target)
    if is_plat("windows") then
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
        return
    end
    
    if is_plat("macosx") then
        local ldflag = os.iorun("python3-config --ldflag")
        local libs = os.iorun("python3-config --libs")
        target:add("shflags", ldflag, libs)
    end
    target:add("rpathdirs", "$ORIGIN", "$ORIGIN/lib", "$ORIGIN/../lib")

    -- get python include directory.
    local pydir = os.iorun("python3-config --includes")
    local lcPos = string.find(pydir,"\n")
    pydir = (string.sub(pydir,1,lcPos-1))
    target:add("cxflags", pydir)

    -- get suffix configure for link libboost_pythonX.so
    local suffix = get_config("boost_python_suffix")
    if suffix == nil then
        raise("You need to config --boost_python_suffix specify libboost_python suffix")
    end

    suffix = string.upper(suffix)
    if suffix == "3X" then
        local ver = os.iorun("python3 --version")
        local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
        target:add("links", boost_python_lib)
    else
        target:add("links", "boost_python"..suffix)
    end
end
