import("lib.detect.find_path")
import("lib.detect.find_library")
import("lib.detect.find_program")
import("lib.detect.find_programver")

function _find_binary(package, opt)
    local result = package:find_tool("python3", opt)
    if not result then
        result = package:find_tool("python", opt)
    end
    if result then
        -- check if pip, setuptools and wheel are installed
        local ok = try { function ()
            os.vrunv(result.program, {"-c", "import pip"})
            os.vrunv(result.program, {"-c", "import setuptools"})
            os.vrunv(result.program, {"-c", "import wheel"})
            return true
        end}
        if not ok then
            return false
        end
    end
end

function _find_library(package, opt)

    -- init search options
    opt = opt or {}
    opt.paths = opt.paths or {}
    table.insert(opt.paths, "$(env PATH)")
    table.insert(opt.paths, "$(env CONDA_PREFIX)")

    -- find python
    local program = find_program("python3", opt)
    if not program then
        program = find_program("python", opt)
    end
    local version = nil
    if program then
        opt.command = function ()
            local outs, errs = os.iorunv(program, {"--version"})
            return ((outs or "") .. (errs or "")):trim()
        end
        version = find_programver(program, opt)
    end
    if not program or not version then
        return false
    end

    -- find library and header
    local exepath = path.directory(program)
    local link = nil
    local libpath = nil
    local includepath = nil
    if package:is_plat("windows") then
        link = format("python" .. table.concat(table.slice(version:split("%."), 1, 2), ""))
        libpath = find_library(link, {exepath}, {suffixes = {"libs"}})
        linkdirs = {}
        includepath = find_path("Python.h", {exepath}, {suffixes = {"include"}})
    else
        local pyver = table.concat(table.slice(version:split("%."), 1, 2), ".")
        link = format("python" .. pyver)
        libpath = find_library(link, {path.directory(exepath)}, {suffixes = {"lib", "lib64", "lib/x86_64-linux-gnu"}})
        includepath = find_path("Python.h", {path.directory(exepath)}, {suffixes = {"include/python" .. pyver}})
    end

    if libpath and includepath then
        local result = {
            version = version,
            includedirs = includepath
        }
        if not package:config("headeronly") then
            result.links = libpath.link
            result.linkdirs = libpath.linkdir
        end        
        return result
    end
end

function main(package, opt)
    if opt.system then
        if package:is_binary() then
            return _find_binary(package, opt)
        else
            return _find_library(package, opt)
        end
    end
end
