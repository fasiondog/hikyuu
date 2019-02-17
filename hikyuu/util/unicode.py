#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

#加入__unicode__和__repr__函数
import sys
if sys.version > '3':
    IS_PY3 = True
else:
    IS_PY3 = False

if IS_PY3:
    unicodeFunc = str
    def reprFunc(self):
        return "<%s>"%str(self)

else:        
    if sys.platform == 'win32':
        def unicodeFunc(self):
            return str(self).decode('gb18030')
        def reprFunc(self):
            return "<%s>"%unicode(self).encode('gb18030')
    else:
        def unicodeFunc(self):
            return str(self).decode('utf8')
        def reprFunc(self):
            return "<%s>"%unicode(self).encode('utf8')