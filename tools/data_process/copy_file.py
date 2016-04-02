#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables as tb

dir = "c:/stock/"
tb.copy_file(dir + "sh_day.h5", dir + "sh_day_new.h5")
