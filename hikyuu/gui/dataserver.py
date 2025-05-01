#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-04-15
#    Author: fasiondog

from hikyuu import start_data_server, stop_data_server
import time
import sys

if __name__ == "__main__":
    data_server_addr = "tcp://0.0.0.0:9201" if len(sys.argv) <= 1 else sys.argv[1]
    work_num = 2 if len(sys.argv) <= 2 else int(sys.argv[2])

    try:
        start_data_server(data_server_addr, work_num)

        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        stop_data_server()
