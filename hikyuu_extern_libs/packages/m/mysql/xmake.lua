package("mysql")

    set_homepage("https://dev.mysql.com/doc/refman/5.7/en/")
    set_description("Open source relational database management system.")

    if is_plat("windows") then
        add_urls("https://github.com/fasiondog/hikyuu/releases/download/1.1.9/mysql-$(version)-win-x64.zip",
                 "https://gitee.com/fasiondog/hikyuu/attach_files/935339/download/mysql-$(version)-win-x64.zip")
        add_versions("8.0.21", "de21694aa230a00b52b28babbce9bb150d990ba1f539edf8d193586dce3844ae")
    elseif is_plat("linux") then
        add_urls("https://github.com/fasiondog/hikyuu/releases/download/1.1.9/mysql-$(version)-linux-x86_64.zip",
                 "https://gitee.com/fasiondog/hikyuu/releases/download/1.1.9/mysql-$(version)-linux-x86_64.zip")
        add_versions("8.0.31", "1775a94f4a59cfb03593e6e70891de33a0cc8713573afdfc9ca0482415a8ecd3")
    end

    on_install("windows", "linux", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        if package:is_plat("windows") then
            os.cp("bin", package:installdir())
        end
    end)

