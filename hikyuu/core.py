#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 20240126
#    Author: fasiondog

import sys

try:
    if sys.version_info[1] == 8:
        from .cpp.core38 import *
    elif sys.version_info[1] == 9:
        from .cpp.core39 import *
    elif sys.version_info[1] == 10:
        from .cpp.core310 import *
    elif sys.version_info[1] == 11:
        from .cpp.core311 import *
    elif sys.version_info[1] == 12:
        from .cpp.core312 import *
    else:
        from .cpp.core import *
except:
    from .cpp.core import *

__version__ = get_version()
