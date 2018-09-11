-- project
set_project("hikyuu")

-- version
set_version("1.0.9")

-- set warning all as error
set_warnings("all", "error")
--set_warnings("all")

-- set language: C99, c++ standard
set_languages("C99", "cxx17")

add_plugindirs("./xmake_plugins")

-- disable some compiler errors
add_cxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing")
add_mxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing")

set_objectdir("$(buildir)/$(mode)/$(plat)/$(arch)/.objs")
set_targetdir("$(buildir)/$(mode)/$(plat)/$(arch)/lib")
set_headerdir("$(buildir)/$(mode)/$(plat)/$(arch)/inc")

add_includedirs("hikyuu_extern_libs/inc")

add_includedirs("$(env BOOST_ROOT)")
add_linkdirs("$(env BOOST_LIB)")

add_defines("BOOST_ALL_DYN_LINK")

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

    add_cxflags("-EHsc")
    
    add_cxflags("-wd4819")  --template dll export warning
    
    if is_mode("release") then
        add_cxflags("-MD") 
    elseif is_mode("debug") then
        add_cxflags("-Gs", "-RTC1") 
        add_cxflags("-MDd") 
    end
end

--add_vectorexts("sse", "sse2", "sse3", "ssse3", "mmx", "neon", "avx", "avx2")

if is_plat("windows") then
    add_subdirs("./hikyuu_extern_libs/src/sqlite3")
end
add_subdirs("./hikyuu_cpp/hikyuu_utils")
add_subdirs("./hikyuu_cpp/importdata")
add_subdirs("./hikyuu_cpp/hikyuu")
add_subdirs("./hikyuu_pywrap")
add_subdirs("./hikyuu_cpp/unit_test")
add_subdirs("./hikyuu_cpp/demo")

before_install("scripts.before_install")
on_install("scripts.on_install")
before_run("scripts.before_run")
