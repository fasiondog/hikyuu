-- Last Modified: 2023-06-13 17:18:39
target("hikyuu")
if is_mode("debug", "coverage", "asan", "msan", "tsan", "lsan") then
    set_kind("static")
else
    set_kind("shared")
end

add_packages("boost", "fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json", "cpp-httplib")
if is_plat("windows", "linux", "cross") then
    add_packages("sqlite3")
end

add_includedirs("..")

-- set version for release
set_configdir("./")
add_configfiles("$(projectdir)/config.h.in")
add_configfiles("$(projectdir)/version.h.in")

if is_plat("windows") then
    add_cxflags("-wd4819")
    add_cxflags("-wd4251") -- template dll export warning
    add_cxflags("-wd4267")
    add_cxflags("-wd4834") -- C++17 discarding return value of function with 'nodiscard' attribute
    add_cxflags("-wd4244") -- discable double to int
else
    add_rpathdirs("$ORIGIN")
    add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
end

if is_plat("windows") then
    add_defines("HKU_API=__declspec(dllexport)")
    if is_mode("release") then
        add_packages("hdf5")
    else
        add_packages("hdf5_D")
    end
    add_packages("mysql")
end

if is_plat("linux", "cross") then
    add_packages("hdf5", "mysql", "boost")
end

if is_plat("macosx") then
    -- add_linkdirs("/usr/local/opt/libiconv/lib")
    add_links("iconv")
    add_includedirs("/usr/local/opt/hdf5/include")
    add_linkdirs("/usr/local/opt/hdf5/lib")
    add_links("hdf5", "hdf5_cpp")
    if os.exists("/usr/local/opt/mysql-client") then
        add_includedirs("/usr/local/opt/mysql-client/include")
        add_linkdirs("/usr/local/opt/mysql-client/lib")
        add_rpathdirs("/usr/local/opt/mysql-client/lib")
    end
    if os.exists("/usr/local/mysql/lib") then
        add_includedirs("/usr/local/include")
        add_linkdirs("/usr/local/mysql/lib")
        add_rpathdirs("/usr/local/mysql/lib")
    end
    add_links("mysqlclient")
    add_links("sqlite3")
end

-- add files
add_files("./**.cpp")

add_headerfiles("../(hikyuu/**.h)|**doc.h")

before_build(function(target)
    if is_plat("macosx") then
        if not os.exists("/usr/local/include/mysql") then
            if os.exists("/usr/local/mysql/include") then
                os.run("ln -s /usr/local/mysql/include /usr/local/include/mysql")
            else
                print("Not Found MySQL include dir!")
            end
        end
    end
end)

after_build(function(target)
    -- 不同平台的库后缀名
    local lib_suffix = ".so"
    if is_plat("windows") then
        lib_suffix = ".dll"
    elseif is_plat("macosx") then
        lib_suffix = ".dylib"
    end

    local libdir = get_config("buildir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" ..
                       get_config("arch") .. "/lib"
    -- 将依赖的库拷贝至build的输出目录
    for libname, pkg in pairs(target:pkgs()) do
        local pkg_path = pkg:installdir()
        if pkg_path ~= nil then
            print("copy dependents: " .. pkg_path)
            os.trycp(pkg_path .. "/bin/*" .. lib_suffix, libdir)
            os.trycp(pkg_path .. "/lib/*" .. lib_suffix, libdir)
            os.trycp(pkg_path .. "/lib/*.so.*", libdir)
        end
    end
end)

target_end()
