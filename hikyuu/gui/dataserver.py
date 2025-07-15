#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-04-15
#    Author: fasiondog

from hikyuu import start_data_server, stop_data_server
import time
import sys
import click


@click.command()
@click.option('--addr', '-addr', default="tcp://127.0.0.1:9201", help='行情接收处理服务地址')
@click.option('--work_num', '-n', default=2, type=int, help='行情接收处理线程数')
@click.option('--save', '-save', default=False, type=bool, help='保存行情数据(仅支持clickhouse)')
@click.option('--buf', '-buf', default=False, type=bool, help='缓存行情数据')
def main(addr, work_num, save, buf):
    try:
        start_data_server(addr, work_num, save_tick=save, buf_tick=buf)

        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        stop_data_server()


if __name__ == "__main__":
    main()
