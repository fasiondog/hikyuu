import("core.platform.platform")
import("core.base.option")

function main(target)
    if target:isphony() then
        return 
    end
    
    local targetname = target:name()
    if "_hikyuu" ~= targetname then
        return
    end

    local installdir = option.get("installdir") or os.getenv("INSTALLDIR") or os.getenv("DESTDIR") or platform.get("installdir")
    
    if is_plat("windows") then
        os.exec("xcopy /S /Q /Y /I hikyuu_python " .. installdir)
    end
    if is_plat("linux") then
        os.exec("cp -f -r -T hikyuu_python " .. installdir)
    end
    if is_plat("macosx") then
        os.exec("cp -f -r hikyuu_python " .. installdir)
    end
end