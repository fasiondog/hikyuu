package("hdf5")

    set_homepage("https://www.hdfgroup.org/solutions/hdf5/")
    set_description("High-performance data management and storage suite")
    set_license("BSD-3-Clause")

    if is_plat("windows") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-win-x64.zip",
            "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-win-x64.zip")
        add_versions("1.12.2", "388d455c917b153f3410e8ca0c857ee37a575d859a70ecb6e16d4fb43b1d201c")
    elseif is_plat("linux") and is_arch("x86_64") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-linux-x64.zip",
            "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-linux-x64.zip")
        add_versions("1.12.2", "e0f4357ea7bfa0132c3edba9b517635736191f920ce7a3aeef5e89dbe5b2dd27")
    elseif is_plat("linux", "cross") and is_arch("aarch64", "arm64.*") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-linux-aarch64.zip",
                 "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5-$(version)-linux-aarch64.zip")
        add_versions("1.12.2", "d73a880d9dfede0d5db1e30555fa251ca82efa437a0d93b46f5e64e87e71fc63")
    end

    on_load("windows", "linux", "cross", function (package)
        package:add("defines", "H5_BUILT_AS_DYNAMIC_LIB")
    end)

    on_install("windows", "linux", "cross", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        if package:is_plat("windows") then
            os.cp("bin", package:installdir())
        end
    end)

