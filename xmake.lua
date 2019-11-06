set_xmakever("2.2.5")

-- project
set_project("hikyuu")

-- version
set_version("1.1.5")
set_configvar("USE_ASYNC_LOGGER", 0)
set_configvar("SPDLOG_ACTIVE_LEVEL", 2)
set_configvar("CHECK_ACCESS_BOUND", 1)
set_configvar("SUPPORT_SERIALIZATION", 1)
set_configvar("SUPPORT_TEXT_ARCHIVE", 0)
set_configvar("SUPPORT_XML_ARCHIVE", 1)
set_configvar("SUPPORT_BINARY_ARCHIVE", 1)

-- set warning all as error
if is_plat("windows") then
    set_warnings("all", "error")
else
    set_warnings("all")
end

-- set language: C99, c++ standard
set_languages("cxx17", "C99")

add_plugindirs("./xmake_plugins")

add_requires("fmt", {configs = {vs_runtime = "MD"}})
add_requires("spdlog", {configs = {header_only = true}})

-- use fmt, spdlog
add_defines("FMT_HEADER_ONLY=1")

set_objectdir("$(buildir)/$(mode)/$(plat)/$(arch)/.objs")
set_targetdir("$(buildir)/$(mode)/$(plat)/$(arch)/lib")

add_includedirs("hikyuu_extern_libs/inc")

add_includedirs("$(env BOOST_ROOT)")
add_linkdirs("$(env BOOST_LIB)")

-- modifed to use boost static library, except boost.python
--add_defines("BOOST_ALL_DYN_LINK")
add_defines("BOOST_SERIALIZATION_DYN_LINK")

if is_host("linux") then
    if is_arch("x86_64") then
        add_linkdirs("/usr/lib/x86_64-linux-gnu")
    end
end

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
end

-- is release now
if is_mode("release") then
    if is_plat("windows") then
        --Unix-like systems hidden symbols will cause the link dynamic libraries to failed!
        set_symbols("hidden") 
    end
    set_optimize("fastest")
    set_strip("all")
end

-- for the windows platform (msvc)
if is_plat("windows") then 
    add_packagedirs("./hikyuu_extern_libs/pkg")
    -- add some defines only for windows
    add_defines("NOCRYPT", "NOGDI")
    add_cxflags("-EHsc", "/Zc:__cplusplus")
    add_cxflags("-wd4819")  --template dll export warning
    add_defines("WIN32_LEAN_AND_MEAN")
    if is_mode("release") then
        add_cxflags("-MD") 
    elseif is_mode("debug") then
        add_cxflags("-Gs", "-RTC1") 
        add_cxflags("-MDd")
    end
end 

if not is_plat("windows") then
    -- disable some compiler errors
    add_cxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing")
    add_cxflags("-ftemplate-depth=1023", "-pthread")
    add_shflags("-pthread")
    add_ldflags("-pthread")
end

add_vectorexts("sse", "sse2", "sse3", "ssse3", "mmx", "avx")

if is_plat("windows") then
    add_subdirs("./hikyuu_extern_libs/src/sqlite3")
end
add_subdirs("./hikyuu_cpp/hikyuu")
add_subdirs("./hikyuu_pywrap")
add_subdirs("./hikyuu_cpp/unit_test")
add_subdirs("./hikyuu_cpp/demo")

after_build("scripts.after_build")
before_install("scripts.before_install")
on_install("scripts.on_install")
before_run("scripts.before_run")
