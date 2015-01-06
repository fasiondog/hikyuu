#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables

class FiveMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        #IGNORE:E1101
    openPrice = tables.UInt32Col()       #IGNORE:E1101
    highPrice = tables.UInt32Col()       #IGNORE:E1101
    lowPrice = tables.UInt32Col()        #IGNORE:E1101
    closePrice = tables.UInt32Col()      #IGNORE:E1101
    transAmount = tables.UInt64Col()     #IGNORE:E1101
    transCount = tables.UInt64Col()      #IGNORE:E1101

class IndexDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        #IGNORE:E1101
    start = tables.UInt64Col()        #IGNORE:E1101

def fenge(src_file_name, dest_file_name):
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    all_table = [x for x in src_hdf5.walkNodes("/data")]
    for i in range(1,len(all_table)):
        src_table = all_table[i]
        print src_table.name
        dest_table = dest_hdf5.createTable("/", src_table.name, FiveMinDataRecordH5File, src_table.name)
        dest_row = dest_table.row
        for x in src_table:
            if x['lowPrice'] <= x['openPrice'] <=x['highPrice'] \
               and x['lowPrice'] <= x['closePrice'] <= x['highPrice']:
    		        dest_row['datetime'] = x['datetime']
    		        dest_row['openPrice'] = x['openPrice']
    		        dest_row['highPrice'] = x['highPrice']
    		        dest_row['lowPrice'] = x['lowPrice']
    		        dest_row['closePrice'] = x['closePrice']
    		        dest_row['transAmount'] = x['transAmount']
    		        dest_row['transCount'] = x['transCount']
    		        dest_row.append()
        dest_table.flush()

    src_hdf5.close()
    dest_hdf5.close()


def fenge2(src_file_name, dest_file_name, table_name_list, lastdate):
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    for src_table_name in table_name_list:
        print src_table_name
        src_table = src_hdf5.getNode("/data", src_table_name)
        try:
            group = dest_hdf5.getNode("/", "data")
        except:
            group = dest_hdf5.createGroup("/", "data")
        dest_table = dest_hdf5.createTable(group, src_table.name, FiveMinDataRecordH5File, src_table.name)
        dest_row = dest_table.row
        for x in src_table:
            if x['datetime'] > lastdate:
                break
            if x['lowPrice'] <= x['openPrice'] <=x['highPrice'] \
               and x['lowPrice'] <= x['closePrice'] <= x['highPrice']:
    		        dest_row['datetime'] = x['datetime']
    		        dest_row['openPrice'] = x['openPrice']
    		        dest_row['highPrice'] = x['highPrice']
    		        dest_row['lowPrice'] = x['lowPrice']
    		        dest_row['closePrice'] = x['closePrice']
    		        dest_row['transAmount'] = x['transAmount']
    		        dest_row['transCount'] = x['transCount']
    		        dest_row.append()
            else:
                print x['datetime']
        dest_table.flush()

    src_hdf5.close()
    dest_hdf5.close()


def fenge3(src_file_name, dest_path):
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    all_table = [x for x in src_hdf5.walkNodes("/data")]
    for i in range(1,len(all_table)):
        src_table = all_table[i]
        print src_table.name
        dest_file_name = dest_path + "/" + src_table.name[2:] + ".h5"
        dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

        dest_table = dest_hdf5.createTable("/", "data", FiveMinDataRecordH5File, "data")
        dest_row = dest_table.row
        for x in src_table:
            if x['lowPrice'] <= x['openPrice'] <=x['highPrice'] \
               and x['lowPrice'] <= x['closePrice'] <= x['highPrice']:
    		        dest_row['datetime'] = x['datetime']
    		        dest_row['openPrice'] = x['openPrice']
    		        dest_row['highPrice'] = x['highPrice']
    		        dest_row['lowPrice'] = x['lowPrice']
    		        dest_row['closePrice'] = x['closePrice']
    		        dest_row['transAmount'] = x['transAmount']
    		        dest_row['transCount'] = x['transCount']
    		        dest_row.append()
        dest_table.flush()
        dest_hdf5.close()

    src_hdf5.close()

def fenge4(src_file_name, dest_file_name, table_name_list, groupname):
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    for src_table_name in table_name_list:
        print src_table_name
        src_table = src_hdf5.getNode(groupname, src_table_name)
        dest_table = dest_hdf5.createTable(groupname, src_table.name, IndexDataRecordH5File, src_table.name)
        dest_row = dest_table.row
        for x in src_table:
            dest_row['datetime'] = x['datetime']
            dest_row['start'] = x['start']
            dest_row.append()
        dest_table.flush()

    src_hdf5.close()
    dest_hdf5.close()

def fenge5(src_file_name, dest_file_name):
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    all_table = [x for x in src_hdf5.walkNodes("/data")]
    for i in range(1,len(all_table)):
        src_table = all_table[i]
        print src_table.name
        #src_table = src_hdf5.getNode("/data", src_table_name)
        try:
            group = dest_hdf5.getNode("/", "data")
        except:
            group = dest_hdf5.createGroup("/", "data")
        dest_table = dest_hdf5.createTable(group, src_table.name, FiveMinDataRecordH5File, src_table.name)
        dest_row = dest_table.row
        for x in src_table:
            if x['lowPrice'] <= x['openPrice'] <=x['highPrice'] \
               and x['lowPrice'] <= x['closePrice'] <= x['highPrice']:
    		        dest_row['datetime'] = x['datetime']
    		        dest_row['openPrice'] = x['openPrice']
    		        dest_row['highPrice'] = x['highPrice']
    		        dest_row['lowPrice'] = x['lowPrice']
    		        dest_row['closePrice'] = x['closePrice']
    		        dest_row['transAmount'] = x['transAmount']
    		        dest_row['transCount'] = x['transCount']
    		        dest_row.append()
            else:
                print x['datetime']
        dest_table.flush()

    src_hdf5.close()
    dest_hdf5.close()	
	
if __name__ == "__main__":
    import time
    starttime = time.time()
    """
    src_file_name = "d:\\stock\\sh_day.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sh_day.h5"
    src_table_name_list = ["SH000001","SH600000","SH600004","SH600005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)

    src_file_name = "d:\\stock\\sz_day.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sz_day.h5"
    src_table_name_list = ["SZ000001","SZ000002","SZ000004","SZ000005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)

    src_file_name = "d:\\stock\\sh_5min.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sh_5min.h5"
    src_table_name_list = ["SH000001","SH600000","SH600004","SH600005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)

    src_file_name = "d:\\stock\\sz_5min.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sz_5min.h5"
    src_table_name_list = ["SZ000001","SZ000002","SZ000004","SZ000005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)

    src_file_name = "d:\\stock\\sh_1min.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sh_1min.h5"
    src_table_name_list = ["SH000001","SH600000","SH600004","SH600005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)

    src_file_name = "d:\\stock\\sz_1min.h5"
    dest_file_name = "d:\\workspace\\hikyuu\\test\\data\\sz_1min.h5"
    src_table_name_list = ["SZ000001","SZ000002","SZ000004","SZ000005"]
    fenge2(src_file_name, dest_file_name, src_table_name_list)
    """

    src_file_name = "d:\\stock\\sh_1min.h5"
    dest_path = "d:\\stock\\sh\\1min"
    fenge3(src_file_name, dest_path)

    src_file_name = "d:\\stock\\sz_5min.h5"
    dest_path = "d:\\stock\\sz\\5min"
    fenge3(src_file_name, dest_path)

    src_file_name = "d:\\stock\\sz_1min.h5"
    dest_path = "d:\\stock\\sz\\1min"
    fenge3(src_file_name, dest_path)

    endtime = time.time()
    print "%.2fs" % (endtime-starttime)
    print "%.2fm" % ((endtime-starttime)/60)

