import("core.platform.platform")
import("core.base.option")

function main(target)
    if target:isphony() then
        return 
    end

    -- 目前什么也不做，但需要保留，以便改变 xmake install 的默认行为
end
