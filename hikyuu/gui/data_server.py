#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-04-15
#    Author: fasiondog

from hikyuu import start_data_server, stop_data_server
import time

if __name__ == "__main__":
    try:
        start_data_server()

        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        stop_data_server()
