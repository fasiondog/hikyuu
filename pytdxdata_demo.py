
import asyncio

from pytdxdata import TdxData
from datetime import datetime

import time
from concurrent import futures
from pytdx.hq import TdxHq_API
from pytdx.config.hosts import hq_hosts

def convert2_datetime(label, format_label="%Y%m%d"):
    return datetime.strptime(label, format_label)

async def main() -> None:
    # TdxData 为异步上下文管理器：进入时建连，退出时释放
    async with TdxData() as td:

        # # 4) 板块汇总（示例：通达信行业板块 881001）
        # summary = await td.get_board_summary("881001")
        # print("\n== 板块汇总 881001 ==")
        # for key, value in summary.items():
        #     print(f"  {key}: {value}")
        # 其他类型
        # all_boards =await td.get_boards()
        # time.sleep(1)
        # # 其他类型
        # industry_boards =await td.get_boards(kind='industry')
        # time.sleep(1)
        # concept_boards =await td.get_boards(kind='concept',count=10)
        # time.sleep(1)
        # # 细分业务
        # style_boards =await td.get_boards(kind='style')

        # board_members = await td.get_board_members('880465')
        bars =  await td.get_bars("880439",  start_date=convert2_datetime("20261027"),
                                  end_date=convert2_datetime("20261029"),
                                  )
        # 成交额 亿
        
        amounty = bars[0].amount / (10000*10000)
        print(f"成交额 亿: {amounty}")
        print(1)

if __name__ == "__main__":
    asyncio.run(main())