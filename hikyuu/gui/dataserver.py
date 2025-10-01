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
@click.option('--addr', '-addr', default="tcp://0.0.0.0:9201", help='自身对外提供的服务地址, 如需外部机器访问，请使用 tcp://0.0.0.0:port')
@click.option('--work_num', '-n', default=2, type=int, help='行情接收处理线程数')
@click.option('--save', '-save', default=False, type=bool, help='保存行情数据(仅支持clickhouse)')
@click.option('--buf', '-buf', default=False, type=bool, help='缓存行情数据')
@click.option('--parquet_path', '-parquet_path', default="", help='Parquet 文件存储路径, 空字符串表示不保存')
def main(addr, work_num, save, buf, parquet_path):
    # 如需其他机器访问，addr 参数需为 tcp://0.0.0.0:port
    try:
        start_data_server(addr, work_num, save_tick=save, buf_tick=buf, parquet_path=parquet_path)

        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        stop_data_server()


if __name__ == "__main__":
    main()
