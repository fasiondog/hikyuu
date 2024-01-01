set_xmakever("2.8.2")

option("hdf5")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable hdf5 kdata engine.")
option_end()

option("mysql")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable mysql kdata engine.")
    if is_plat("macosx") then
        if os.exists("/usr/local/opt/mysql-client/lib") then
            add_includedirs("/usr/local/opt/mysql-client/include/mysql")
            add_includedirs("/usr/local/opt/mysql-client/include")
            add_linkdirs("/usr/local/opt/mysql-client/lib")
            add_rpathdirs("/usr/local/opt/mysql-client/lib")
        end
        if os.exists("/usr/local/mysql/lib") then
            add_linkdirs("/usr/local/mysql/lib")
            add_rpathdirs("/usr/local/mysql/lib")
        end
        if not os.exists("/usr/local/include/mysql") then
            if os.exists("/usr/local/mysql/include") then
                os.run("ln -s /usr/local/mysql/include /usr/local/include/mysql")
            else
                print("Not Found MySQL include dir!")
            end
        end
        add_links("mysqlclient")
    elseif is_plat("windows") then
        add_defines("NOMINMAX")
    end        
option_end()

option("sqlite")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable sqlite kdata engine.")
option_end()

option("tdx")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable tdx kdata engine.")
option_end()

option("stacktrace")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable check/assert with stack trace info.")
    add_defines("HKU_ENABLE_STACK_TRACE")
option_end()

option("feedback")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable send feedback.")
option_end()


-- project
set_project("hikyuu")

add_rules("mode.debug", "mode.release")
if not is_plat("windows") then add_rules("mode.coverage", "mode.asan", "mode.msan", "mode.tsan", "mode.lsan") end

-- version
set_version("1.3.1", {build = "%Y%m%d%H%M"})
set_configvar("LOG_ACTIVE_LEVEL", 0) -- 激活的日志级别
-- if is_mode("debug") then
--    set_configvar("LOG_ACTIVE_LEVEL", 0)  -- 激活的日志级别
-- else
--    set_configvar("LOG_ACTIVE_LEVEL", 2)  -- 激活的日志级别
-- end
set_configvar("USE_SPDLOG_LOGGER", 1) -- 是否使用spdlog作为日志输出
set_configvar("USE_SPDLOG_ASYNC_LOGGER", 0) -- 使用异步的spdlog
set_configvar("CHECK_ACCESS_BOUND", 1)
if is_plat("macosx") then
    set_configvar("SUPPORT_SERIALIZATION", 0)
else
    set_configvar("SUPPORT_SERIALIZATION", is_mode("release") and 1 or 0)
end
set_configvar("SUPPORT_TEXT_ARCHIVE", 0)
set_configvar("SUPPORT_XML_ARCHIVE", 1)
set_configvar("SUPPORT_BINARY_ARCHIVE", 1)
set_configvar("HKU_DISABLE_ASSERT", 0)
set_configvar("ENABLE_MSVC_LEAK_DETECT", 0)
set_configvar("HKU_ENABLE_SEND_FEEDBACK", get_config("feedback") and 1 or 0)

set_configvar("HKU_ENABLE_HDF5_KDATA", get_config("hdf5") and 1 or 0)
set_configvar("HKU_ENABLE_MYSQL_KDATA", get_config("mysql") and 1 or 0)
set_configvar("HKU_ENABLE_SQLITE_KDATA", get_config("sqlite") and 1 or 0)
set_configvar("HKU_ENABLE_TDX_KDATA", get_config("tdx") and 1 or 0)

set_warnings("all")

-- set language: C99, c++ standard
set_languages("cxx17", "c99")

local boost_version = "1.81.0"
local hdf5_version = "1.12.2"
local fmt_version = "10.0.0"
local flatbuffers_version = "23.5.26"
local mysql_version = "8.0.31"
if is_plat("windows") or (is_plat("linux", "cross") and is_arch("aarch64", "arm64.*")) then 
    mysql_version = "8.0.21" 
end

add_repositories("project-repo hikyuu_extern_libs")
if is_plat("windows") then
    if get_config("hdf5") then
        if is_mode("release") then
            add_requires("hdf5 " .. hdf5_version)
        else
            add_requires("hdf5_D " .. hdf5_version)
        end
    end
    if get_config("mysql") then
        add_requires("mysql " .. mysql_version)
    end

elseif is_plat("linux", "cross") then
    if get_config("hdf5") then
        add_requires("hdf5 " .. hdf5_version, { system = false })
    end
    if get_config("mysql") then
        add_requires("mysql " .. mysql_version, { system = false })
    end
  
elseif is_plat("macosx") then
    if get_config("hdf5") then
        add_requires("brew::hdf5")
    end
    if get_config("mysql") then
        add_requires("brew::mysql-client")
    end
end

add_requires("myboost " .. boost_version, {
  system = false,
  alias = "boost",
  debug = is_mode("debug"),
  configs = {
    shared = is_plat("windows") and true or false,
    date_time = true,
    filesystem = true,
    serialization = true,
    system = false,
    -- python = false,
    -- pyver = get_config("pyver"),
  },
})
-- if is_plat("windows") then
--   add_requireconfs("myboost.python", {override = true, system=false})
-- end

add_requires("spdlog", {system = false, configs = {header_only = true, fmt_external = true, vs_runtime = "MD"}})
add_requireconfs("spdlog.fmt", {override = true, version = fmt_version, configs = {header_only = true}})
add_requires("sqlite3", {system = false, configs = {shared = true, vs_runtime = "MD", cxflags = "-fPIC"}})
add_requires("flatbuffers v" .. flatbuffers_version, {system = false, configs = {vs_runtime = "MD"}})
add_requires("nng", {system = false, configs = {vs_runtime = "MD", cxflags = "-fPIC"}})
add_requires("nlohmann_json", {system = false})
add_requires("cpp-httplib", {system = false, configs = {zlib = true, ssl = true}})
add_requires("zlib", {system = false})

add_defines("SPDLOG_DISABLE_DEFAULT_LOGGER") -- 禁用 spdlog 默认ogger

set_objectdir("$(buildir)/$(mode)/$(plat)/$(arch)/.objs")
set_targetdir("$(buildir)/$(mode)/$(plat)/$(arch)/lib")

-- modifed to use boost static library, except boost.python, serialization
if is_plat("windows") then add_defines("BOOST_ALL_DYN_LINK") end

-- is release now
if is_mode("release") then
  if is_plat("windows") then
    -- Unix-like systems hidden symbols will cause the link dynamic libraries to failed!
    set_symbols("hidden")
  end
end

-- for the windows platform (msvc)
if is_plat("windows") then
  -- add some defines only for windows
  add_defines("NOCRYPT", "NOGDI")
  add_cxflags("-EHsc", "/Zc:__cplusplus", "/utf-8")
  add_cxflags("-wd4819") -- template dll export warning
  add_defines("WIN32_LEAN_AND_MEAN")
  if is_mode("release") then
    add_cxflags("-MD")
  elseif is_mode("debug") then
    add_cxflags("-Gs", "-RTC1", "/bigobj")
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
--
-- add_vectorexts("sse", "sse2", "sse3", "ssse3", "mmx", "avx")
if not is_plat("cross") and (os.host() == "linux" and is_arch("x86_64", "x64")) then
  -- fedora或者ubuntu，并且不是交叉编译
  add_vectorexts("sse", "sse2", "ssse3", "avx", "avx2")
end

includes("./hikyuu_cpp/hikyuu")
includes("./hikyuu_pywrap")
includes("./hikyuu_cpp/unit_test")
includes("./hikyuu_cpp/demo")
includes("./hikyuu_cpp/hikyuu_server")

before_install("scripts.before_install")
on_install("scripts.on_install")
before_run("scripts.before_run")
