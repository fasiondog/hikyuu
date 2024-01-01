package("mypybind11")
    set_base("pybind11")

    add_deps("cmake", "python 3.x")
    on_install("windows", "macosx", "linux", "cross", function (package)
        import("package.tools.cmake").install(package, {"-DPYBIND11_TEST=OFF"})
    end)


