task("hkutest")
    set_category("plugin")

    -- on run
    on_run(function ()

        -- trace
        print("test xmake!")
        print("$(buildir)/$(mode)/$(plat)/$(arch)/lib")
        --os.cp("./test_data", "$(buildir)/$(mode)/$(plat)/$(arch)/lib")

    end)

    -- set menu
    set_menu {
                -- usage
                usage = "xmake hkutest [options]"

                -- description
            ,   description = "run unit test for hikyuu!"

                -- options
            ,   options = {}
            }
