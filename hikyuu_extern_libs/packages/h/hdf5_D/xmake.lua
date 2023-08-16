package("hdf5_D")

    set_homepage("https://www.hdfgroup.org/solutions/hdf5/")
    set_description("High-performance data management and storage suite")
    set_license("BSD-3-Clause")

    if is_plat("windows") then
        add_urls("https://github.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5_D-$(version)-win-x64.zip",
            "https://gitee.com/fasiondog/hikyuu_extern_libs/releases/download/1.0.0/hdf5_D-$(version)-win-x64.zip")
        add_versions("1.12.2", "6ea3ab5a4b0bb0f48eaef28cfe747ac5c072c06519a4888c1a59cfaf75399049")
    end

    on_load("windows", function (package)
        package:add("defines", "H5_BUILT_AS_DYNAMIC_LIB")
    end)

    on_install("windows", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        os.cp("bin", package:installdir())
    end)

