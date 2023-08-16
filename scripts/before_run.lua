import("core.project.config")

function main(target)
  local targetname = target:name()

  if "demo" == targetname then
    local with_demo = config.get("with-demo")
    if not with_demo then raise("You need to config first: xmake f --with-demo=y") end
  end

  if "unit-test" == targetname or "small-test" == targetname then
    print("copying test_data ...")
    os.rm("$(buildir)/$(mode)/$(plat)/$(arch)/lib/test_data")
    os.cp("$(projectdir)/test_data", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
  end

  if is_plat("windows") then os.cp("$(env BOOST_LIB)/boost_*.dll", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/") end

  -- if is_plat("linux") and os.getenv(BOOST_LIB) > "" then
  --   -- 不确定是否需要加入这段才能在fedora下使用
  --   os.cp("$(env BOOST_LIB)/libboost_*.so.*", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/")
  -- end

  if is_plat("macosx") then os.cp("$(env BOOST_LIB)/libboost_*.dylib", "$(buildir)/$(mode)/$(plat)/$(arch)/lib/") end
end
