package("hdf5")

    set_homepage("https://www.hdfgroup.org/solutions/hdf5/")
    set_description("High-performance data management and storage suite")
    set_license("BSD-3-Clause")

    if is_plat("windows") then
        add_urls("https://github.com/fasiondog/hikyuu/releases/download/1.1.9/hdf5-$(version)-win-x64.zip",
            "https://gitee.com/fasiondog/hikyuu/releases/download/1.1.9/hdf5-$(version)-win-x64.zip")
        add_versions("1.12.2", "388d455c917b153f3410e8ca0c857ee37a575d859a70ecb6e16d4fb43b1d201c")
        add_versions("1.10.4", "253b23baada1d9c86cb4424595eba366b6844c384a5e0aafebf0893a1148f25f")
    elseif is_plat("linux") then
        add_urls("https://github.com/fasiondog/hikyuu/releases/download/1.1.9/hdf5-$(version)-linux-x64.zip",
            "https://gitee.com/fasiondog/hikyuu/releases/download/1.1.9/hdf5-$(version)-linux-x64.zip")
        add_versions("1.12.2", "8a6b571168481fba273e1f0c7838d417f38222b5d93250388d2ddff5ff7f2611")
    end

    on_load("windows", "linux", function (package)
        package:add("defines", "H5_BUILT_AS_DYNAMIC_LIB")
    end)

    on_install("windows", "linux", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        if package:is_plat("windows") then
            os.cp("bin", package:installdir())
        end
    end)

