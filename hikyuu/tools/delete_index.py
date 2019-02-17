#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables as tbl

def delete_h5_index(filename):
    """清除h5数据文件中的周线、月线、季线、半年线、年线数据索引
    """
    print("begin clear week, month, quarter, halfyear, year index from ", filename)
    with tbl.open_file(filename, "a") as h5file:
        try:
            h5file.remove_node("/week", recursive=True)
        except:
            print("remove week failed!")
        else:
            print("success remove week")

        try:
            h5file.remove_node("/month", recursive=True)
        except:
            print("remove month failed!")
        else:
            print("success remove month")

        try:
            h5file.remove_node("/quarter", recursive=True)
        except:
            print("remove quarter failed!")
        else:
            print("success remove quarter")

        try:
            h5file.remove_node("/halfyear", recursive=True)
        except:
            print("remove halfyear failed!")
        else:
            print("success remove halfyear")

        try:
            h5file.remove_node("/year", recursive=True)
        except:
            print("remove year failed!")
        else:
            print("success remove year")


if __name__ == "__main__":
    delete_h5_index("C:\\stock\\sh_day.h5")
    delete_h5_index("C:\\stock\\sz_day.h5")