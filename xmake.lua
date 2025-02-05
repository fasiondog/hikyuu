set_xmakever("2.8.2")

-- project
set_project("hikyuu")

add_rules("mode.debug", "mode.release")

-- version
set_version("2.3.1", {build = "%Y%m%d%H%M"})

set_warnings("all")

-- set language: C99, c++ standard
set_languages("c++17")


option("mysql")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable mysql kdata engine.")
    if is_plat("windows") then
        add_defines("NOMINMAX")
    end        
option_end()

option("hdf5", {description = "Enable hdf5 kdata engine.", default = true})
option("sqlite", {description = "Enable sqlite kdata engine.", default = true})
option("tdx", {description = "Enable tdx kdata engine.", default = true})
option("sql_trace", {description = "trace print sql", default = false})

-- 注意：stacktrace 在 windows 下会严重影响性能
option("stacktrace", {description = "Enable check/assert with stack trace info.", default = false})
option("spend_time", {description = "Enable spend time.", default = true})
option("feedback", {description = "Enable send feedback.", default = true})
option("low_precision", {description = "Enable low precision.", default = false})
option("log_level", {description = "set log level.", default = 2, values = {1, 2, 3, 4, 5, 6}})
option("async_log", {description = "Use async log.", default = false})
option("leak_check", {description = "Enable leak check for test", default = false})

-- 使用 serialize 时，建议使用静态库方式编译，boost serializasion 对 dll 的方式支持不好
-- windows下如果使用 serialize 且希望使用动态库，需要设置 runtimes 参数为 "MD"
-- "MT" 方式下，serialize 会挂
option("serialize", {description = "Enable support serialize object and pickle in python", default = true})

-- 和 hku_utils 编译选项保持一致，以便互相替换
option("mo", {description = "International language support", default = false})
-- option("http_client", {description = "use http client", default = true})
option("http_client_ssl", {description = "enable https support for http client", default = false})
option("http_client_zip", {description = "enable http support gzip", default = false})
-- option("node", {description = "enable node reqrep server/client", default = true})

option("ta_lib")
    add_deps("low_precision")
    set_default(true)
    set_showmenu(true)
    set_category("hikyuu")
    set_description("Enable ta-lib support.")
    -- low_precision 时，需禁用，ta-lib不支持输出为 float
    after_check(function (option)
      if option:dep("low_precision"):enabled() then
          cprint('${red}[warning] "low_precision" is enabled, ta-lib will be disabled')
          option:enable(false)
      end
    end)        
option_end()


-- SPDLOG_ACTIVE_LEVEL 需要单独加
local log_level = get_config("log_level")
if log_level == nil then
    log_level = 2
end
add_defines("SPDLOG_ACTIVE_LEVEL=" .. log_level)

if is_mode("debug") then
    set_configvar("HKU_DEBUG_MODE", 1)
else
    set_configvar("HKU_DEBUG_MODE", 0)
end
set_configvar("CHECK_ACCESS_BOUND", 1)
set_configvar("SUPPORT_SERIALIZATION", get_config("serialize") and 1 or 0)
set_configvar("SUPPORT_TEXT_ARCHIVE", 0)
set_configvar("SUPPORT_XML_ARCHIVE", 1)
set_configvar("SUPPORT_BINARY_ARCHIVE", 1)
set_configvar("ENABLE_MSVC_LEAK_DETECT", 0)
set_configvar("HKU_ENABLE_LEAK_DETECT", get_config("leak_check") and 1 or 0)
set_configvar("HKU_ENABLE_SEND_FEEDBACK", get_config("feedback") and 1 or 0)

set_configvar("HKU_ENABLE_HDF5_KDATA", get_config("hdf5") and 1 or 0)
set_configvar("HKU_ENABLE_MYSQL", get_config("mysql") and 1 or 0)
set_configvar("HKU_ENABLE_MYSQL_KDATA", get_config("mysql") and 1 or 0)
set_configvar("HKU_ENABLE_SQLITE", (get_config("sqlite") or get_config("hdf5")) and 1 or 0)
set_configvar("HKU_ENABLE_SQLITE_KDATA", get_config("sqlite") and 1 or 0)
set_configvar("HKU_ENABLE_TDX_KDATA", get_config("tdx") and 1 or 0)

set_configvar("HKU_USE_LOW_PRECISION", get_config("low_precision") and 1 or 0)
set_configvar("HKU_ENABLE_TA_LIB", get_config("ta_lib") and 1 or 0)

set_configvar("HKU_SUPPORT_DATETIME", 1)
set_configvar("HKU_ENABLE_SQLCIPHER", 0)
set_configvar("HKU_SQL_TRACE", get_config("sql_trace"))
set_configvar("HKU_ENABLE_INI_PARSER", 1)
set_configvar("HKU_ENABLE_STACK_TRACE", get_config("stacktrace") and 1 or 0)
set_configvar("HKU_CLOSE_SPEND_TIME", get_config("spend_time") and 0 or 1)
set_configvar("HKU_USE_SPDLOG_ASYNC_LOGGER", get_config("async_log") and 1 or 0)
set_configvar("HKU_LOG_ACTIVE_LEVEL", get_config("log_level"))
set_configvar("HKU_ENABLE_MO", get_config("mo") and 1 or 0)
set_configvar("HKU_ENABLE_HTTP_CLIENT", 1)
set_configvar("HKU_ENABLE_HTTP_CLIENT_SSL", get_config("http_client_ssl") and 1 or 0)
set_configvar("HKU_ENABLE_HTTP_CLIENT_ZIP", get_config("http_client_zip") and 1 or 0)
set_configvar("HKU_ENABLE_NODE", 1)

local boost_version = "1.87.0"
local hdf5_version = "1.12.2"
local fmt_version = "11.1.3"
local spdlog_version = "1.15.1"
local flatbuffers_version = "24.3.25"
local nng_version = "1.8.0"
local sqlite_version = "3.48.0+0"
local mysql_version = "8.0.31"
if is_plat("windows") or (is_plat("linux", "cross") and is_arch("aarch64", "arm64.*")) then 
    mysql_version = "8.0.21" 
elseif is_plat("macosx") then
    mysql_version = "8.0.40"
end

add_repositories("hikyuu-repo https://github.com/fasiondog/hikyuu_extern_libs.git")
-- add_repositories("hikyuu-repo https://gitee.com/fasiondog/hikyuu_extern_libs.git")
 if get_config("hdf5") then
        add_requires("hdf5 " .. hdf5_version, { system = false })
 end
 if get_config("mysql") then
     add_requires("mysql " .. mysql_version, { system = false })
 end

add_requires("boost " .. boost_version, {
  debug = is_mode("debug"),
  configs = {
    shared = is_plat("windows"),
    runtimes = get_config("runtimes"),
    multi = true,
    date_time = true,
    filesystem = false,
    serialization = get_config("serialize"),
    system = false,
    python = false,
    cmake = false,
  },
})

add_requires("fmt " .. fmt_version, {configs = {header_only = true}})
add_requires("spdlog " .. spdlog_version, {configs = {header_only = true, fmt_external = true}})
add_requireconfs("spdlog.fmt", {override = true, version=fmt_version, configs = {header_only = true}})
add_requires("sqlite3 " .. sqlite_version, {configs = {shared = true, safe_mode="2", cxflags = "-fPIC"}})
add_requires("flatbuffers v" .. flatbuffers_version, {system = false, configs= {runtimes = get_config("runtimes")}})
add_requires("nng " .. nng_version, {configs = {NNG_ENABLE_TLS = has_config("http_client_ssl"), cxflags = "-fPIC"}})
add_requires("nlohmann_json")

if has_config("http_client_zip") then
    add_requires("gzip-hpp")
end

if has_config("ta_lib") then
    add_requires("ta-lib")
end

add_defines("SPDLOG_DISABLE_DEFAULT_LOGGER") -- 禁用 spdlog 默认ogger

set_objectdir("$(buildir)/$(mode)/$(plat)/$(arch)/.objs")
set_targetdir("$(buildir)/$(mode)/$(plat)/$(arch)/lib")

-- on windows dll, must use runtimes MD
if is_plat("windows") and get_config("kind") == "shared" then 
    set_config("runtimes", "MD")
    set_runtimes("MD")
end

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
  if is_mode("debug") then
    add_cxflags("-Gs", "-RTC1", "/bigobj")
  end
end

if is_plat("linux", "cross", "macosx") then
  -- disable some compiler errors
  add_cxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing")
  add_cxflags("-ftemplate-depth=1023", "-pthread")
  add_shflags("-pthread")
  add_ldflags("-pthread")
end

if is_plat("macosx") then
    add_cxflags("-Wno-deprecated-declarations")
end

-- if not is_plat("cross") and (os.host() == "linux" and is_arch("x86_64", "x64")) then
--   -- fedora或者ubuntu，并且不是交叉编译
--   add_vectorexts("sse", "sse2", "ssse3", "avx", "avx2")
-- --   add_defines("HKU_ENABLE_SSE2", "HKU_ENABLE_SSE3", "HKU_ENABLE_SSE41", "HKU_ENABLE_AVX", "HKU_ENABLE_AVX2")
-- end

includes("./copy_dependents.lua")
includes("./hikyuu_cpp/hikyuu")
includes("./hikyuu_pywrap")
includes("./hikyuu_cpp/unit_test")
includes("./hikyuu_cpp/demo")