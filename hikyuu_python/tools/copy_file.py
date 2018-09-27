#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables as tb

dir = "c:/stock/"

tb.copy_file(dir + "sh_day.h5", dir + "sh_day_new.h5")
tb.copy_file(dir + "sz_day.h5", dir + "sz_day_new.h5")

tb.copy_file(dir + "sh_5min.h5", dir + "sh_5min_new.h5")
tb.copy_file(dir + "sz_5min.h5", dir + "sz_5min_new.h5")

#tb.copy_file(dir + "sh_1min.h5", dir + "sh_1min_new.h5")
#tb.copy_file(dir + "sz_1min.h5", dir + "sz_1min_new.h5")