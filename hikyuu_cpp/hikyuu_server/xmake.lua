target("hkuserver")
    set_kind("binary")
    
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "nng", "nlohmann_json", "sqlite3", "zlib")
    add_deps("hikyuu")

    add_includedirs(".")
    add_includedirs("..")

    if is_plat("windows") then
        add_cxflags("-wd4819")  
        add_cxflags("-wd4251")  --template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834")  --C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4996")
        add_cxflags("-wd4244")  --discable double to int
        add_cxflags("-wd4566")
    else
        add_rpathdirs("$ORIGIN")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end
    
    if is_plat("windows") then 
        if is_mode("release") then
            add_defines("HKU_API=__declspec(dllimport)")
        end
        add_packages("mysql")
    end
    
    if is_plat("linux") then
        add_packages("mysql")
    end
    
    if is_plat("macosx") then
        --add_linkdirs("/usr/local/opt/libiconv/lib")
        add_links("iconv")
        add_includedirs("/usr/local/opt/mysql-client/include")
        add_linkdirs("/usr/local/opt/mysql-client/lib")
    end

    if is_plat("windows") then 
        -- nng 静态链接需要的系统库
        add_syslinks("ws2_32", "advapi32")
    end

    if is_plat("linux") or is_plat("macosx") then
        add_links("sqlite3")
        add_links("mysqlclient")
    end
    
    -- add files
    add_files("./**.cpp")

    after_build(function(target)
        os.cp("$(projectdir)/hikyuu_cpp/hikyuu_server/i8n/", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
    end)
    
target_end()
