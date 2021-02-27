target("hikyuu_server")
    set_kind("binary")
    
    add_packages("fmt", "spdlog", "flatbuffers", "nng")
    add_deps("hikyuu")

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
        add_defines("SQLITE_API=__declspec(dllimport)")
        add_defines("HKU_API=__declspec(dllimport)")
        add_includedirs("../../hikyuu_extern_libs/src/sqlite3")
        add_deps("sqlite3")
        add_packages("mysql")
    end
    
    if is_plat("linux") then
        if is_arch("x86_64")  then
            if os.exists("/usr/lib64/mysql") then
                add_linkdirs("/usr/lib64/mysql")
            end
            add_linkdirs("/usr/lib/x86_64-linux-gnu")
        end
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
    
target_end()
