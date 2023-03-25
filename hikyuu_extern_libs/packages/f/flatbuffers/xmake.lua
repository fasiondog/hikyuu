package("flatbuffers")

    set_homepage("http://google.github.io/flatbuffers/")
    set_description("FlatBuffers is a cross platform serialization library architected for maximum memory efficiency.")
    set_license("Apache-2.0")

    add_urls("https://github.com/google/flatbuffers/archive/$(version).zip",
             "https://github.com/google/flatbuffers.git")
    add_versions("v1.12.0", "4b8b21adbfe8a74b90604161afcf87c125a26b86c99327e7a04525080606536c")
    add_versions("v2.0.0", "ffd68aebdfb300c9e82582ea38bf4aa9ce65c77344c94d5047f3be754cc756ea")
    add_versions("v23.1.21", "48597d6a6f8ca67a02ae8d8494b3bfc9136eb93da60a538d5bfc024f7c564f97")

    add_deps("cmake")
    on_install("windows", "linux", "macosx", "mingw", "android", "iphoneos", "cross", function(package)
        local configs = {"-DFLATBUFFERS_BUILD_TESTS=OFF"}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DFLATBUFFERS_BUILD_SHAREDLIB=" .. (package:config("shared") and "ON" or "OFF"))
        if package:is_cross() then
            table.insert(configs, "-DFLATBUFFERS_BUILD_FLATC=OFF")
            table.insert(configs, "-DFLATBUFFERS_BUILD_FLATHASH=OFF")
        end
        import("package.tools.cmake").install(package, configs)
        package:addenv("PATH", "bin")
    end)

    on_test(function(package)
        if not package:is_cross() then
            os.vrun("flatc --version")
        end
        assert(package:check_cxxsnippets({test = [[
            void test() {
                flatbuffers::FlatBufferBuilder builder;
                builder.CreateString("MyMonster");
                flatbuffers::DetachedBuffer dtbuilder = builder.Release();
            }
        ]]}, {configs = {languages = "c++14"}, includes = "flatbuffers/flatbuffers.h"}))
    end)
