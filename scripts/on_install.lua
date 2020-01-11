import("core.platform.platform")
import("core.base.option")

function main(target)
    if target:isphony() then
        return 
    end

    -- 目前什么也不做，仅改变 xmake install 的默认行为
end
