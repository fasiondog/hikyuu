#!/usr/bin/python
# cp936

import sqlite3
import tables as tbl

dirname = 'c:\stock'    

connect = sqlite3.connect(dirname + '\stock.db')
cur = connect.cursor()
a = cur.execute("select market.market, stock.code from stock join market on stock.marketid = market.marketid")
stk_list = [ s[0]+s[1] for s in a]

def clear_empty_table(h5file, group_name):
    group = h5file.get_node(group_name)
    node_list = []
    for node in group:
        if 0 == node.nrows:
            node_list.append(node.name)
            #print(node.name, " nrows = 0")
        elif node.name not in stk_list:
            node_list.append(node.name)
            #print(node.name, " is not in stk_list")
        else:
            pass
    for node in node_list:
        try:
            h5file.remove_node(group, node)
        except:
            pass
    print("共清理：%i" % len(node_list))
    
#--------------------------------------------------
# 清理深证日线数据
#--------------------------------------------------
srcfilename = dirname + '\sz_day.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理日线数据")
clear_empty_table(h5file, '/data')

print("\n清理周线数据")
clear_empty_table(h5file, '/week')

print("\n清理月线数据")
clear_empty_table(h5file, '/month')

print("\n清理季线数据")
clear_empty_table(h5file, '/quarter')

print("\n清理半年线数据")
clear_empty_table(h5file, '/halfyear')

print("\n清理年线数据")
clear_empty_table(h5file, '/year')

h5file.close()

#--------------------------------------------------
# 清理上证日线数据
#--------------------------------------------------
srcfilename = dirname + '\sh_day.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理日线数据")
clear_empty_table(h5file, '/data')

print("\n清理周线数据")
clear_empty_table(h5file, '/week')

print("\n清理月线数据")
clear_empty_table(h5file, '/month')

print("\n清理季线数据")
clear_empty_table(h5file, '/quarter')

print("\n清理半年线数据")
clear_empty_table(h5file, '/halfyear')

print("\n清理年线数据")
clear_empty_table(h5file, '/year')

h5file.close()

#--------------------------------------------------
# 清理上证5分钟线数据
#--------------------------------------------------
srcfilename = dirname + '\sh_5min.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理5分钟线数据")
clear_empty_table(h5file, '/data')

#print("\n清理15分钟线数据")
#clear_empty_table(h5file, '/min15')

#print("\n清理30分钟线数据")
#clear_empty_table(h5file, '/min30')

#print("\n清理60分钟线数据")
#clear_empty_table(h5file, '/min60')

h5file.close()

#--------------------------------------------------
# 清理上证5分钟线数据
#--------------------------------------------------
srcfilename = dirname + '\sz_5min.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理5分钟线数据")
clear_empty_table(h5file, '/data')

#print("\n清理15分钟线数据")
#clear_empty_table(h5file, '/min15')

#print("\n清理30分钟线数据")
#clear_empty_table(h5file, '/min30')

#print("\n清理60分钟线数据")
#clear_empty_table(h5file, '/min60')

h5file.close()

#--------------------------------------------------
# 清理上证1分钟线数据
#--------------------------------------------------
srcfilename = dirname + '\sh_1min.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理1分钟线数据")
clear_empty_table(h5file, '/data')

h5file.close()

#--------------------------------------------------
# 清理上证5分钟线数据
#--------------------------------------------------
srcfilename = dirname + '\sz_1min.h5'
print('--------------------------------------------------')
print(srcfilename)

h5file = tbl.open_file(srcfilename, "a")

print("清理1分钟线数据")
clear_empty_table(h5file, '/data')

h5file.close()