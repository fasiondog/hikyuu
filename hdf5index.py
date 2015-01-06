#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
"""
建立HDF5索引
"""
import datetime
import tables



class IndexRecord(tables.IsDescription):
    datetime = tables.UInt64Col()        #IGNORE:E1101
    start = tables.UInt64Col()       #IGNORE:E1101


def UpdateWeekIndex(h5file): 
    try:
        group = h5file.getNode("/","week")
    except:
        group = h5file.createGroup("/","week")
        
    def getNewDate(olddate):
        y = olddate/100000000
        m = olddate/1000000 - y*100
        d = olddate/10000 - (y*10000+m*100)
        tempdate = datetime.date(y,m,d)
        tempweekdate = tempdate - datetime.timedelta(tempdate.weekday())
        newdate = tempweekdate.year*100000000 + tempweekdate.month*1000000 + tempweekdate.day*10000
        return newdate

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            pre_index_date = getNewDate(int(table[0]['datetime']))
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()
            

def UpdateMonthIndex(h5file): 
    try:
        group = h5file.getNode("/","month")
    except:
        group = h5file.createGroup("/","month")
        
    def getNewDate(olddate):
        y = olddate/100000000
        m = olddate/1000000 - y*100
        return(y*100000000 + m*1000000 + 10000)

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()            


def UpdateYearIndex(h5file): 
    try:
        group = h5file.getNode("/","year")
    except:
        group = h5file.createGroup("/","year")

    def getNewDate(olddate):
        y = olddate/100000000
        return(y*100000000 + 1010000)

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()            


def UpdateHalfYearIndex(h5file): 
    try:
        group = h5file.getNode("/","halfyear")
    except:
        group = h5file.createGroup("/","halfyear")
    
    def getNewDate(olddate):
        halfyearDict={1:1,2:1,3:1,4:1,5:1,6:1,7:7,8:7,9:7,10:7,11:7,12:7}
        y = olddate/100000000
        m = olddate/1000000 - y*100
        return( y*100000000 + halfyearDict[m]*1000000 + 10000 )
        
    
    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()            


def UpdateQuarterIndex(h5file): 
    try:
        group = h5file.getNode("/","quarter")
    except:
        group = h5file.createGroup("/","quarter")

    def getNewDate(olddate):
        quarterDict={1:1,2:1,3:1,4:4,5:4,6:4,7:7,8:7,9:7,10:10,11:10,12:10}
        y = olddate/100000000
        m = olddate/1000000 - y*100
        return( y*100000000 + quarterDict[m]*1000000 + 10000 )
        
    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()  


def UpdateDayIndex(h5file): 
    try:
        group = h5file.getNode("/","day")
    except:
        group = h5file.createGroup("/","day")

    def getNewDate(olddate):
        newdate = olddate/10000*10000
        return newdate
    
    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()          
        

def UpdateHourIndex(h5file): 
    try:
        group = h5file.getNode("/","min60")
    except:
        group = h5file.createGroup("/","min60")

    def getNewDate(olddate):
        min = olddate-olddate/10000*10000
        if min<=1030:
            newdate = olddate/10000*10000 + 1030
        elif min<=1130:
            newdate = olddate/10000*10000 + 1130
        elif min<=1400:
            newdate = olddate/10000*10000 + 1400
        else:
            newdate = olddate/10000*10000 + 1500
        return newdate

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()          


def UpdateFifteenMinIndex(h5file): 
    try:
        group = h5file.getNode("/","min15")
    except:
        group = h5file.createGroup("/","min15")

    def getNewDate(olddate):
        min = olddate-olddate/10000*10000
        if min<=945:
            newdate = olddate/10000*10000 + 945
        elif min<=1000:
            newdate = olddate/10000*10000 + 1000
        elif min<=1015:
            newdate = olddate/10000*10000 + 1015
        elif min<=1030:
            newdate = olddate/10000*10000 + 1030
        elif min<=1045:
            newdate = olddate/10000*10000 + 1045
        elif min<=1100:
            newdate = olddate/10000*10000 + 1100
        elif min<=1115:
            newdate = olddate/10000*10000 + 1115
        elif min<=1130:
            newdate = olddate/10000*10000 + 1130
        elif min<=1315:
            newdate = olddate/10000*10000 + 1315
        elif min<=1330:
            newdate = olddate/10000*10000 + 1330
        elif min<=1345:
            newdate = olddate/10000*10000 + 1345
        elif min<=1400:
            newdate = olddate/10000*10000 + 1400
        elif min<=1415:
            newdate = olddate/10000*10000 + 1415
        elif min<=1430:
            newdate = olddate/10000*10000 + 1430
        elif min<=1445:
            newdate = olddate/10000*10000 + 1445
        else:
            newdate = olddate/10000*10000 + 1500
        return newdate

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()    


def UpdateHalfHourIndex(h5file): 
    try:
        group = h5file.getNode("/","min30")
    except:
        group = h5file.createGroup("/","min30")

    def getNewDate(olddate):
        min = olddate-olddate/10000*10000
        if min<=1000:
            newdate = olddate/10000*10000 + 1000
        elif min<=1030:
            newdate = olddate/10000*10000 + 1030
        elif min<=1100:
            newdate = olddate/10000*10000 + 1100
        elif min<=1130:
            newdate = olddate/10000*10000 + 1130
        elif min<=1330:
            newdate = olddate/10000*10000 + 1330
        elif min<=1400:
            newdate = olddate/10000*10000 + 1400
        elif min<=1430:
            newdate = olddate/10000*10000 + 1430
        else:
            newdate = olddate/10000*10000 + 1500
        return newdate

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()    


def UpdateFiveMinIndex(h5file): 
    try:
        group = h5file.getNode("/","min5")
    except:
        group = h5file.createGroup("/","min5")

    def getNewDate(olddate):
        newdate = olddate/100*100
        min = olddate-newdate
        if min == 0:
            pass
        elif min <= 5:
            newdate += 5 
        elif min<=10:
            newdate += 10
        elif min<=15:
            newdate += 15
        elif min<=20:
            newdate += 20
        elif min<=25:
            newdate += 25
        elif min<=30:
            newdate += 30
        elif min<=35:
            newdate += 35
        elif min<=40:
            newdate += 40
        elif min<=45:
            newdate += 45
        elif min<=50:
            newdate += 50
        elif min<=55:
            newdate += 55
        else:
            newdate += 100
        return newdate

    for table in h5file.walkNodes("/data"):
        if type(table) != tables.table.Table:
            continue
        
        #print table.name
        try:
            index_table = h5file.getNode(group,table.name)
        except:
            index_table = h5file.createTable(group,table.name, IndexRecord)

        total = table.nrows
        if 0 == total:
            continue

        index_total = index_table.nrows
        index_row = index_table.row
        if index_total:
            index_last_date = int(index_table[-1]['datetime'])
            last_date = getNewDate(int(table[-1]['datetime']))
            if index_last_date == last_date:
                continue
            startix = int(index_table[-1]['start'])
            pre_index_date = int(index_table[-1]['datetime'])
        else:
            startix = 0
            date = int(table[0]['datetime'])
            pre_index_date = getNewDate(date)
            index_row['datetime'] = pre_index_date
            index_row['start'] = 0
            index_row.append()
            #week_table.flush()
            
        index = startix
        for row in table[startix:]:
            date = int(row['datetime'])
            cur_index_date = getNewDate(date)
            if cur_index_date != pre_index_date:
                index_row['datetime'] = cur_index_date
                index_row['start'] = index
                index_row.append()
                pre_index_date = cur_index_date
            index += 1
        index_table.flush()  


def UpdateDayDataAllIndex(h5file):
    UpdateWeekIndex(h5file)
    UpdateMonthIndex(h5file)
    UpdateQuarterIndex(h5file)
    UpdateHalfYearIndex(h5file)
    UpdateYearIndex(h5file)
    
def Update5MinDataAllIndex(h5file):    
    UpdateFifteenMinIndex(h5file)
    UpdateHalfHourIndex(h5file)    
    UpdateHourIndex(h5file)
    #UpdateDayIndex(h5file)
    #UpdateWeekIndex(h5file)
    #UpdateMonthIndex(h5file)
    #UpdateQuarterIndex(h5file)
    #UpdateHalfYearIndex(h5file)
    #UpdateYearIndex(h5file)

def Update1MinDataAllIndex(h5file):
    UpdateFiveMinIndex(h5file)
    #UpdateFifteenMinIndex(h5file)
    #UpdateHalfHourIndex(h5file)    
    #UpdateHourIndex(h5file)
    #UpdateDayIndex(h5file)
    #UpdateWeekIndex(h5file)
    #UpdateMonthIndex(h5file)
    #UpdateQuarterIndex(h5file)
    #UpdateHalfYearIndex(h5file)
    #UpdateYearIndex(h5file)


if __name__ == "__main__":
    import time
    starttime = time.time()
    
    print "\nUpdate SH Day Data index ================> "
    h5file = tables.openFile('d:/workspace/hikyuu/test/data/sh_day.h5', mode='a', 
                                      filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    UpdateDayDataAllIndex(h5file)
    h5file.close()
   
    print "\nUpdate SH 5min Data index ================> "
    h5file = tables.openFile('d:/workspace/hikyuu/test/data/sh_5min.h5', mode='a', 
                                      filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    Update5MinDataAllIndex(h5file)
    h5file.close()
    
    print "\nUpdate SZ Day Data index ================> "
    h5file = tables.openFile('d:/workspace/hikyuu/test/data/sz_day.h5', mode='a', 
                                      filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    UpdateDayDataAllIndex(h5file)
    h5file.close()

    print "\nUpdate SZ 5min Data index ================> "
    h5file = tables.openFile('d:/workspace/hikyuu/test/data/sz_5min.h5', mode='a', 
                                      filters=tables.Filters(complevel=9,complib='zlib', shuffle=True))
    Update5MinDataAllIndex(h5file)
    h5file.close()
    
  
    endtime = time.time()
    print "\nTotal time:"
    print "%.2fs" % (endtime-starttime)
    print "%.2fm" % ((endtime-starttime)/60)    
