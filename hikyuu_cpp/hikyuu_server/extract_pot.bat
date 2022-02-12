; 实际不需要使用该命令，因为不能自动更新；实际直接使用 poedit 打开 i8n/zh_CN.po 进行更新
; 此处保留，用于 poedit 设置关键字时参考
xgettext -k_tr -k"_ctr:1c,2" -s -c --package-name=hikyuu -f i8n/POTFILES.in -o i8n/hikyuu.pot