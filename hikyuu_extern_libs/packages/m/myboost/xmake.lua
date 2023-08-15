package("myboost")
    set_base("boost")

    on_load(function (package)
        function get_linkname(package, libname)
            local linkname
            if package:is_plat("windows") then
                linkname = (package:config("shared") and "boost_" or "libboost_") .. libname
            else
                linkname = "boost_" .. libname
            end
            if libname == "python" or libname == "numpy" then
                linkname = linkname .. package:config("pyver"):gsub("%p+", "")
            end
            if package:config("multi") then
                linkname = linkname .. "-mt"
            end
            if package:is_plat("windows") then
                local vs_runtime = package:config("vs_runtime")
                if package:config("shared") then
                    if package:debug() then
                        linkname = linkname .. "-gd"
                    end
                elseif vs_runtime == "MT" then
                    linkname = linkname .. "-s"
                elseif vs_runtime == "MTd" then
                    linkname = linkname .. "-sgd"
                elseif vs_runtime == "MDd" then
                    linkname = linkname .. "-gd"
                end
            else
                if package:debug() then
                    linkname = linkname .. "-d"
                end
            end
            return linkname
        end
        -- we need the fixed link order
        local sublibs = {log = {"log_setup", "log"},
                         python = {"python", "numpy"},
                         stacktrace = {"stacktrace_backtrace", "stacktrace_basic"}}
        for _, libname in ipairs(libnames) do
            local libs = sublibs[libname]
            if libs then
                for _, lib in ipairs(libs) do
                    package:add("links", get_linkname(package, lib))
                end
            else
                package:add("links", get_linkname(package, libname))
            end
        end
        -- disable auto-link all libs
        if package:is_plat("windows") then
            package:add("defines", "BOOST_ALL_NO_LIB")
        end

        if package:config("python") then
            if not package:config("shared") then
                package:add("defines", "BOOST_PYTHON_STATIC_LIB")
            end
            if package:is_plat("windows") then
                package:add("deps", "python " .. package:config("pyver") .. ".x")
            else
                package:add("deps", "python " .. package:config("pyver") .. ".x", {configs = {headeronly = true}})
            end
        end
    end)    

    on_install("macosx", "linux", "windows", "bsd", "mingw", "cross", function (package)
        import("core.base.option")

        -- get toolchain
        local toolchain
        if package:is_plat("windows") then
            toolchain = package:toolchain("clang-cl") or package:toolchain("msvc") or
                import("core.tool.toolchain").load("msvc", {plat = package:plat(), arch = package:arch()})
        end

        -- force boost to compile with the desired compiler
        local win_toolset
        local file = io.open("user-config.jam", "a")
        if file then
            local cxx = package:build_getenv("cxx")
            if package:is_plat("macosx") then
                -- we uses ld/clang++ for link stdc++ for shared libraries
                -- and we need `xcrun -sdk macosx clang++` to make b2 to get `-isysroot` automatically
                local cc = package:build_getenv("ld")
                if cc and cc:find("clang", 1, true) and cc:find("Xcode", 1, true) then
                    cc = "xcrun -sdk macosx clang++"
                end
                file:print("using darwin : : %s ;", cc)
            elseif package:is_plat("windows") then
                local vs_toolset = toolchain:config("vs_toolset")
                local msvc_ver = ""
                win_toolset = "msvc"
                if toolchain:name() == "clang-cl" then
                    win_toolset = "clang-win"
                    cxx = cxx:gsub("(clang%-cl)$", "%1.exe", 1)
                    msvc_ver = ""
                elseif vs_toolset then
                    local i = vs_toolset:find("%.")
                    msvc_ver = i and vs_toolset:sub(1, i + 1)
                end

                -- Specifying a version will disable b2 from forcing tools
                -- from the latest installed msvc version.
                file:print("using %s : %s : \"%s\" ;", win_toolset, msvc_ver, cxx:gsub("\\", "\\\\"))
            else
                file:print("using gcc : : %s ;", cxx:gsub("\\", "/"))
            end
            file:close()
        end

        local bootstrap_argv =
        {
            "--prefix=" .. package:installdir(),
            "--libdir=" .. package:installdir("lib"),
            "--without-icu"
        }

        local runenvs
        if package:is_plat("windows") then
            runenvs = toolchain:runenvs()
            -- for bootstrap.bat, all other arguments are useless
            bootstrap_argv = { "msvc" }
            os.vrunv("bootstrap.bat", bootstrap_argv, {envs = runenvs})
        elseif package:is_plat("mingw") and is_host("windows") then
            bootstrap_argv = { "gcc" }
            os.vrunv("bootstrap.bat", bootstrap_argv)
            -- todo looking for better solution to fix the confict between user-config.jam and project-config.jam
            io.replace("project-config.jam", "using[^\n]+", "")
        else
            os.vrunv("./bootstrap.sh", bootstrap_argv)
        end
        os.vrun("./b2 headers")

        local njobs = option.get("jobs") or tostring(os.default_njob())
        local argv =
        {
            "--prefix=" .. package:installdir(),
            "--libdir=" .. package:installdir("lib"),
            "-d2",
            "-j" .. njobs,
            "--hash",
            "-q", -- quit on first error
            "--layout=tagged-1.66", -- prevent -x64 suffix in case cmake can't find it
            "--user-config=user-config.jam",
            "-sNO_LZMA=1",
            "-sNO_ZSTD=1",
            "install",
            "threading=" .. (package:config("multi") and "multi" or "single"),
            "debug-symbols=" .. (package:debug() and "on" or "off"),
            "link=" .. (package:config("shared") and "shared" or "static"),
            "variant=" .. (package:is_debug() and "debug" or "release"),
            "runtime-debugging=" .. (package:is_debug() and "on" or "off")
        }

        if package:config("lto") then
            table.insert(argv, "lto=on")
        end
        if package:is_arch("aarch64", "arm+.*") then
            table.insert(argv, "architecture=arm")
        end
        if package:is_arch(".+64.*") then
            table.insert(argv, "address-model=64")
        else
            table.insert(argv, "address-model=32")
        end
        if package:is_plat("windows") then
            local vs_runtime = package:config("vs_runtime")
            if package:config("shared") then
                table.insert(argv, "runtime-link=shared")
            elseif vs_runtime and vs_runtime:startswith("MT") then
                table.insert(argv, "runtime-link=static")
            else
                table.insert(argv, "runtime-link=shared")
            end
            table.insert(argv, "cxxflags=-std:c++14")
            table.insert(argv, "toolset=" .. win_toolset)
        elseif package:is_plat("mingw") then
            table.insert(argv, "toolset=gcc")
        else
            table.insert(argv, "cxxflags=-std=c++14")
            if package:config("pic") ~= false then
                table.insert(argv, "cxxflags=-fPIC")
            end
        end
        for _, libname in ipairs(libnames) do
            if package:config("all") or package:config(libname) then
                table.insert(argv, "--with-" .. libname)
            end
        end

        if package:is_plat("linux") then
            table.insert(argv, "pch=off")
        end

        if package:is_plat("windows") and package:config("python") then
            local pypkg = package:dep("python")
            runenvs["PATH"] = pypkg:installdir() .. "/bin" .. ";" .. runenvs["PATH"]
            -- print("=========================")
            -- print(runenvs["PATH"])
            -- print("=========================")
        end

        os.vrunv("./b2", argv, {envs = runenvs})
    end)    
package_end()