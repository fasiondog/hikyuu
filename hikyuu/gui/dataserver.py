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
@click.option('--addr', '-addr', default="tcp://0.0.0.0:9201", help='The service address provided to the outside, use tcp://0.0.0.0:port for external access')
@click.option('--work_num', '-n', default=2, type=int, help='The number of quote receiving and processing threads')
@click.option('--save', '-save', default=False, type=bool, help='Save the quote data (ClickHouse only)')
@click.option('--buf', '-buf', default=False, type=bool, help='Cache the quote data')
@click.option('--parquet_path', '-parquet_path', default="", help='Parquet file storage path; an empty string means do not save')
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
