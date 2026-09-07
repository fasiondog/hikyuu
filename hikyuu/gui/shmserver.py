#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-09-07
#    Author: fasiondog

import time

import click

import hikyuu as hku


@click.command()
@click.option('--datadir', default="", help='数据目录，为空时使用 hikyuu.ini 中 [hikyuu] datadir')
@click.option('--publish_shm', default=True, type=bool, help='是否发布共享内存快照（K线热数据 + 基础信息）')
@click.option('--recv_spot', default=True, type=bool, help='本进程是否接收实时行情并镜像写入快照尾部')
@click.option('--config', 'config_file', default="", help='指定 hikyuu 配置文件路径，为空则使用默认 ~/.hikyuu/hikyuu.ini')
def main(datadir, publish_shm, recv_spot, config_file):
    """在当前进程内启动 shm(共享内存)数据服务（独立 VIP 插件，需有效授权），常驻供其他 hikyuu 进程零拷贝读取。

    服务不会自动产生：需在本进程显式启动；其他进程 import hikyuu 时若同一 datadir 已有服务且 use_shm_server=True，
    将自动作为客户端接入。按 Ctrl-C 停止服务。
    """
    # 服务端进程只作发布者、不接入其他 shm 服务：显式关闭客户端探测（use_shm_server=False），
    # 否则 init 阶段会先当客户端探测既有服务、白等约 10s 后打印 "fallback to standalone mode" 告警。
    # 同时确保按 --config（或默认）配置完成数据初始化（import hikyuu 本身不加载数据）。
    load_kwargs = {"use_shm_server": False}
    if config_file:
        load_kwargs["config_file"] = config_file
    hku.load_hikyuu(**load_kwargs)

    try:
        if not hku.start_shm_server(datadir, publish_shm=publish_shm, recv_spot=recv_spot):
            click.echo("start_shm_server 返回 False：插件缺失 / 授权无效或数据未就绪，本进程未成为服务端。", err=True)
            return

        click.echo("shm 数据服务已启动，按 Ctrl-C 停止。")
        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break

    finally:
        hku.stop_shm_server()


if __name__ == "__main__":
    main()
