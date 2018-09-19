
function main(target)
    if is_plat("windows") then
        local pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
        pydir = path.directory(pydir)
        target:add("includedirs", pydir .. "/include")
        target:add("linkdirs", pydir .. "/libs")
        return
    end
    
    local python = "python" .. get_config("python")

    if is_plat("macosx") then
        local libdir = os.iorun(format("%s-config --prefix", python))
        local lcPos = string.find(libdir,"\n")
        libdir = (string.sub(libdir,1,lcPos-1).."/lib")
        target:add("linkdirs", libdir)
        target:add("links", python.."m")
    end

    target:add("rpathdirs", "$ORIGIN", "$ORIGIN/lib", "$ORIGIN/../lib")

    -- get python include directory.
    local pydir = os.iorun(format("%s-config --includes", python))
    local lcPos = string.find(pydir,"\n")
    pydir = (string.sub(pydir,1,lcPos-1))
    target:add("cxflags", pydir)

    -- get suffix configure for link libboost_pythonX.so
    local suffix = get_config("boost-python-suffix")
    if suffix == nil then
        raise("You need to config --boost-python-suffix specify libboost_python suffix")
    end

    suffix = string.upper(suffix)
    if suffix == "3X" then
        --此处奇怪,macOSX下print(os.iorun("python3 --version"))正常，但赋值给变量的话，变量长度一直为0
        --local ver = os.iorun("python3 --version")
        --local boost_python_lib = "boost_python"..string.sub(ver,8,8)..string.sub(ver,10,10)
        local ver = get_config("python")
        local boost_python_lib = "boost_python" .. string.sub(ver,1,1) .. string.sub(ver,3,3)
        target:add("links", boost_python_lib)
    else
        target:add("links", "boost_python"..suffix)
    end
end
