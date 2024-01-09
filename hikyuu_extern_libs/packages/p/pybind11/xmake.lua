package("pybind11")

    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/pybind/pybind11")
    set_description("Seamless operability between C++11 and Python.")
    set_license("BSD-3-Clause")

    add_urls("https://github.com/pybind/pybind11/archive/$(version).zip",
             "https://github.com/pybind/pybind11.git")
    add_versions("v2.11.1", "b011a730c8845bfc265f0f81ee4e5e9e1d354df390836d2a25880e123d021f89")

    on_install(function (package)
        os.cp("include", package:installdir())
    end)

    -- on_test(function (package)
    --     assert(package:check_cxxsnippets({test = [[
    --         #include <pybind11/pybind11.h>
    --         int add(int i, int j) {
    --             return i + j;
    --         }
    --         PYBIND11_MODULE(example, m) {
    --             m.def("add", &add, "A function which adds two numbers");
    --         }
    --     ]]}, {configs = {languages = "c++11"}}))
    -- end)