"""pytdxdata 快速上手：报价 / K线 / 分时 / 板块汇总。

运行：
    python examples/quickstart.py

说明：数据来自通达信公开行情服务器（无需账号，15 分钟延时）。
标的一律用字符串（``sh600000`` / ``sz000001`` / ``hk00700`` / ``usAAPL`` / ``cffex:IFL0``）。
"""

import asyncio

from pytdxdata import TdxData
from datetime import datetime

import time
from concurrent import futures
from pytdx.hq import TdxHq_API
from pytdx.config.hosts import hq_hosts

def convert2_datetime(label, format_label="%Y%m%d"):
    return datetime.strptime(label, format_label)

# async def main() -> None:
#     # TdxData 为异步上下文管理器：进入时建连，退出时释放
#     async with TdxData() as td:
#         # 1) 批量五档报价（可一次传多只，超过 80 只自动切批）
#         quotes = await td.get_quotes(["sh600000", "sz000001"])
#         print("== 报价 ==")
#         for q in quotes:
#             print(
#                 f"{q.code}  现价={q.price}  昨收={q.pre_close}  "
#                 f"买一={q.bid[0]}  卖一={q.ask[0]}  成交量={q.vol}"
#             )

#         # 2) 日K线（最近 10 根）
#         bars = await td.get_bars("sz000034", count=10, start_date=convert2_datetime("20261021"),
#                                   end_date=convert2_datetime("20261023"))
#         print("\n== 日K（最近 3 根）==")
#         for b in bars[-3:]:
#             print(
#                 f"{b.datetime:%Y-%m-%d}  "
#                 f"开={b.open} 高={b.high} 低={b.low} 收={b.close} 量={b.vol}"
#             )

#         # 3) 当日分时
#         minutes = await td.get_minutes("sh600000")
#         latest = minutes[-1].price if minutes else "-"
#         print(f"\n== 分时 ==\n共 {len(minutes)} 个点，最新价={latest}")

#         # 4) 板块汇总（示例：通达信行业板块 881001）
#         summary = await td.get_board_summary("881001")
#         print("\n== 板块汇总 881001 ==")
#         for key, value in summary.items():
#             print(f"  {key}: {value}")



def ping(ip, port=7709, multithread=False, timeout=1):
    api = TdxHq_API(multithread=multithread)
    success = False
    starttime = time.time()
    success = False
    try:
        if api.connect(ip, port, time_out=timeout):
            # x = api.get_security_count(0)
            # x = api.get_index_bars(7, 1, '000001', 800, 100)
            x = api.get_security_bars(7, 0, '000001', 800, 100)
            if x:
                success = True
            api.close()
    except Exception as e:
        print(e)
        pass
    endtime = time.time()
    return (success, endtime - starttime, ip, port)


def search_best_tdx():
    def ping2(host):
        return ping(host[0], host[1], host[2])

    hosts = [(host[1], host[2], True) for host in hq_hosts]
    with futures.ThreadPoolExecutor() as executor:
        res = executor.map(ping2, hosts, timeout=2)
    x = [i for i in res if i[0] == True]
    x.sort(key=lambda item: item[1])

    # 校验 host 是否存在错误，取同样数据，相同结果最多的集合
    values = {}
    for host in x:
        api = TdxHq_API(multithread=False)
        if api.connect(host[2], host[3]):
            x = api.get_security_bars(9, 0, '159915', 0, 1)
            if x and len(x) > 0:
                if x[0]['close'] not in values:
                    values[x[0]['close']] = [host]
                else:
                    values[x[0]['close']].append(host)
            api.close()
    ret = []
    for _, host in values.items():
        if len(host) > len(ret):
            ret = host
    return ret

if __name__ == "__main__":
    # asyncio.run(main())
    search_best_tdx()