package("mysql")

    set_homepage("https://dev.mysql.com/doc/refman/5.7/en/")
    set_description("Open source relational database management system.")

    if is_plat("windows") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-win-x64.zip",
                 "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-win-x64.zip")
        add_versions("8.0.21", "de21694aa230a00b52b28babbce9bb150d990ba1f539edf8d193586dce3844ae")
    elseif is_plat("linux") and is_arch("x86_64") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-linux-x86_64.zip",
                 "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-linux-x86_64.zip")
        add_versions("8.0.31", "1775a94f4a59cfb03593e6e70891de33a0cc8713573afdfc9ca0482415a8ecd3")
    elseif is_plat("linux", "cross") and is_arch("aarch64", "arm64.*") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-linux-aarch64.zip",
                 "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/mysql-$(version)-linux-aarch64.zip")
        add_versions("8.0.21", "5ceb31ddc75bfaa0ec5324fa1564dae3abdb7ea9d3f0763f10d06e871d21d7d4")
    end

    on_install("windows", "linux", "cross", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        if package:is_plat("windows") then
            os.cp("bin", package:installdir())
        end
    end)

