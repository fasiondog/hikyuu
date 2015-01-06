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

def fenge(src_file_name, dest_file_name, start_date, end_date):
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

   
if __name__ == "__main__":
    import time
    starttime = time.time()
    
    year_list = [2000,2001,2002,2003,2004,2005,2006,2007,2008,2009,2010,2011]
#    month_list = [1,2,3,4,5,6,7,8,9,10,11,12]
    for year in year_list:
        src_file_name = 'd:\\stock\\sz_1min.h5'
        dest_file_name = "d:\\stock\\sz\\1min\\" + str(year) + ".h5"
        start_date = year * 100000000L
        end_date = start_date + 100000000L
        fenge(src_file_name, dest_file_name, start_date, end_date)
#        for month in month_list:
#            dest_file_name = "d:\\stock\\sh\\5min\\" + str(year) + "\\" + str(year) + "_" + str(month) + ".h5"
#            start_date = year * 100000000L + month * 1000000L
#            end_date = start_date + 1000000L
#            fenge(src_file_name, dest_file_name, start_date, end_date)
    
    endtime = time.time()
    print "%.2fs" % (endtime-starttime)
    print "%.2fm" % ((endtime-starttime)/60)
    
    