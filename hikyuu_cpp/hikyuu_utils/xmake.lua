target("hikyuu_utils")
    set_kind("shared")
    
    if is_plat("windows") then
        add_defines("HKU_API=__declspec(dllexport)")
    end

    add_headers("../(hikyuu_utils/*.h)|doc.h")
    add_headers("../(hikyuu_utils/iniparser/*.h)")
    
    -- add files
    add_files("**.cpp")
