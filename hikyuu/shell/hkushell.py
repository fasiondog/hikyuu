#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2021 hikyuu.org
#
# Create on: 2021-03-09
#    Author: fasiondog

from hkucmd import *
from cmdserver import server

HKUShell.add_command(server)

if __name__ == "__main__":
    import colorama
    colorama.init(autoreset=True)
    HKUShell().cmdloop()