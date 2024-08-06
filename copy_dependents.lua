
-- 拷贝依赖的第三方库头文件及lib到指定目录
task("copy_dependents")
    set_category("plugin")

    -- 设置运行脚本
    -- destpath 目标目录
    -- onlylib 只拷贝lib库
    on_run(function(target, destpath, onlylib)
        local libdir = destpath .. '/lib'

        -- 将依赖的库拷贝至build的输出目录
        for libname, pkg in pairs(target:pkgs()) do
            if pkg:installdir() == nil then
                print(libname ..  ": Not found installdir, maybe it is system lib!");
                goto continue
            end
                        
            print("dependent package: " .. pkg:installdir())
            --local linkdirs = pkg:get("linkdirs") 
            -- 部分库没有 linkdirs ，如：MNN, Paddle-lite，所以使用 includedirs
            local pkg_path = pkg:get("includedirs")
            if pkg_path == nil then 
                pkg_path = pkg:get("sysincludedirs") -- xmake 2.3.9 改为了 sysincludedirs
            end

            if pkg_path == nil then
                goto continue
            end

            -- 安装模式下拷贝所有依赖库的头文件
            if not onlylib then
                if type(pkg_path) == 'string' then
                    local pos = string.find(pkg_path, "opencv")
                    if pos == nil then
                        os.trycp(pkg_path, destpath)
                    else
                        os.trycp(pkg_path .. "/opencv2", destpath .. "/include")
                    end
                elseif type(pkg_path) == 'table' then
                    for i=1, #pkg_path do
                        local pos = string.find(pkg_path[i], "hku_utils")
                        if pos == nil then
                            pos = string.find(pkg_path[i], "opencv")
                            if pos == nil then
                                os.trycp(pkg_path[i], destpath)
                            else
                                os.trycp(pkg_path[i] .. "/opencv2", destpath .. "/include")
                            end
                        else
                            for _, filedir in ipairs(os.dirs(pkg_path[i] .. "/*")) do
                                local pos = string.find(filedir, "hikyuu")
                                if pos == nil then
                                    os.trycp(filedir, destpath .. "/include")
                                else
                                    os.trycp(filedir .. "/utilities", destpath .. "/include/hikyuu")
                                end
                            end
                        end
                    end
                end
            end 
            
            -- 拷贝依赖的库文件
            os.trycp(pkg:installdir() .. "/lib/*", libdir)
            if is_plat("windows") then
                os.trycp(pkg:installdir() .. "/bin/*.dll", libdir)
            end

            :: continue ::
        end        
    end)

    set_menu {
        -- usage
        usage = "xmake copy_dependents [options]"

        -- description
    ,   description = "拷贝依赖的第三方库头文件及lib到指定目录!"

        -- options
    ,   options = {}
    }     
task_end()