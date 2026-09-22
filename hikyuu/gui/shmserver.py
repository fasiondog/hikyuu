#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-09-07
#    Author: fasiondog

import time

import click

import hikyuu as hku


@click.command()
@click.option('--datadir', default="", help='Data directory; when empty, the [hikyuu] datadir in hikyuu.ini is used')
@click.option('--publish_shm', default=True, type=bool, help='Whether to publish the shared memory snapshot (K-line hot data + basic info)')
@click.option('--recv_spot', default=True, type=bool, help='Whether this process receives realtime quotes and mirrors them to the tail of the snapshot')
@click.option('--config', 'config_file', default="", help='The hikyuu configuration file path; when empty, the default ~/.hikyuu/hikyuu.ini is used')
def main(datadir, publish_shm, recv_spot, config_file):
    """在当前进程内启动 shm(共享内存)数据服务（独立 VIP 插件，需有效授权），常驻供其他 hikyuu 进程零拷贝读取。

    服务不会自动产生：需在本进程显式启动；其他进程显式开启 use_shm_server=True 且同一
    datadir 已有服务时，将自动作为客户端接入。按 Ctrl-C 停止服务。
    """
    # 服务端进程只作发布者、不接入其他 shm 服务：显式关闭客户端探测（use_shm_server=False）。
    # 该开关默认即关闭，此处仍显式声明以防用户配置文件中已开启；否则 init 阶段会先当客户端
    # 探测既有服务、白等约 10s 后打印 "fallback to standalone mode" 告警。
    # 同时确保按 --config（或默认）配置完成数据初始化（import hikyuu 本身不加载数据）。
    load_kwargs = {"use_shm_server": False}
    if config_file:
        load_kwargs["config_file"] = config_file
    hku.load_hikyuu(**load_kwargs)

    try:
        if not hku.start_shm_server(datadir, publish_shm=publish_shm, recv_spot=recv_spot):
            click.echo("start_shm_server returned False: missing plug-in / invalid license or data not ready; this process did not become the server.", err=True)
            return

        click.echo("The shm data service has started, press Ctrl-C to stop.")
        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        hku.stop_shm_server()


if __name__ == "__main__":
    main()
