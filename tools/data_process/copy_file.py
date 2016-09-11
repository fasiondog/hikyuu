#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables as tb

dir = "c:/stock/"
tb.copy_file(dir + "sz_5min.h5", dir + "sz_5min_new.h5")
