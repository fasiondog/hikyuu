package("hdf5_D")

    set_homepage("https://www.hdfgroup.org/solutions/hdf5/")
    set_description("High-performance data management and storage suite")
    set_license("BSD-3-Clause")

    if is_plat("windows") then
        add_urls("https://gitee.com/fasiondog/hikyuu/attach_files/114377/download/hdf5_D-$(version)-win-x64.zip",
                 "https://gitee.com/fasiondog/hikyuu/attach_files/935348/download/hdf5_D-$(version)-win-x64.zip",
                 "https://github.com/fasiondog/hikyuu/releases/download/1.1.9/hdf5_D-$(version)-win-x64.zip")
        add_versions("1.12.2", "ead6e6e5d4422d36a66c714832828538e28e22a0d37965a731e77c4d6b30a66e")
        add_versions("1.10.4", "5b1bd27e054f885bf9cac0beffcacbe180e251c5d8c12c0652a96055f2784b46")
    end

    on_load("windows", function (package)
        package:add("defines", "H5_BUILT_AS_DYNAMIC_LIB")
    end)

    on_install("windows", function (package)
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        os.cp("bin", package:installdir())
    end)

