#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables
import datetime

class FiveMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        #IGNORE:E1101
    openPrice = tables.UInt32Col()       #IGNORE:E1101
    highPrice = tables.UInt32Col()       #IGNORE:E1101
    lowPrice = tables.UInt32Col()        #IGNORE:E1101
    closePrice = tables.UInt32Col()      #IGNORE:E1101
    transAmount = tables.UInt64Col()     #IGNORE:E1101
    transCount = tables.UInt64Col()      #IGNORE:E1101

def fenge(src_file_name, dest_file_name, start_date, end_date):
    """
    将原始数据按日期范围分隔，方便同步备份
    """    
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    start = start_date
    end = end_date
    all_table = [x for x in src_hdf5.walkNodes("/data")]
    for i in range(1,len(all_table)):
        src_table = all_table[i]
        print src_table.name
        dest_table = dest_hdf5.createTable("/", src_table.name, FiveMinDataRecordH5File, src_table.name)
        dest_row = dest_table.row
        for x in src_table:
            if x['datetime'] >= end:
                break
            if start <= x['datetime'] \
               and x['lowPrice'] <= x['openPrice'] <=x['highPrice'] \
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


def fenge2(src_file_name, dest_dir_name):
    """
    将每个stock的数据拆分成独立的文件
    """
    src_hdf5 = tables.openFile(src_file_name, mode='r', filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

    all_table = [x for x in src_hdf5.walkNodes("/")]
    for i in range(1,len(all_table)):
        src_table = all_table[i]
        #print src_table.name
        dest_file_name = dest_dir_name + src_table.name[2:] + ".h5"
        dest_hdf5 = tables.openFile(dest_file_name, mode = "w", filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))

        dest_table = dest_hdf5.createTable("/", "data", FiveMinDataRecordH5File, src_table.name)
        dest_row = dest_table.row
         
        pre_date = 0
        for x in src_table:
            if x['datetime'] <= pre_date:
                print src_table, pre_date, x['datetime']
                continue

            try:
                tmp_date = x['datetime']
                year = tmp_date / 100000000
                month  = (tmp_date - year * 100000000) / 1000000
                day  = (tmp_date - tmp_date / 1000000 * 1000000) / 10000;
                ttdate = datetime.date(year, month, day)

            except ValueError:
                print src_table, tmp_date

            else:

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
                  print src_table, x['datetime'], "warning 2"
        dest_table.flush()
        dest_hdf5.close()

    src_hdf5.close()
   
if __name__ == "__main__":
    import time
    starttime = time.time()
    
    #src_file_name = '/home/fasiondog/workspace/hikyuu/test/data/sz_day.h5'
    #dest_dir_name = "/home/fasiondog/workspace/hikyuu/test/data/sz/day/"
    #fenge2(src_file_name, dest_dir_name)
	
    src_file_name = 'd:\\stock\\sz_1min.h5'
    dest_file_name = u'D:\\快盘\\80.stock\\stock\\sz\\1min\\2012.h5'
    start_date = 201201010000L
    end_date = 201301010000L
    fenge(src_file_name, dest_file_name, start_date, end_date)
	
    endtime = time.time()
    print "%.2fs" % (endtime-starttime)
    print "%.2fm" % ((endtime-starttime)/60)
    
    
