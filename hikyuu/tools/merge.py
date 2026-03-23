#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables
import datetime
import sys
import os
from pathlib import Path
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import queue
import re


class GUILogRedirector:
    """将控制台输出重定向到 GUI 日志窗口的类"""

    def __init__(self, log_func):
        """
        初始化日志重定向器

        参数:
            log_func: 用于写入日志的函数
        """
        self.log_func = log_func
        self.original_stdout = sys.stdout
        self.original_stderr = sys.stderr
        self._in_write = False  # 防止递归标志

    def write(self, text):
        """写入日志（防止递归）"""
        if self._in_write:
            # 如果已经在 write 中，直接写入原始 stdout 避免递归
            self.original_stdout.write(text)
            return

        if text.strip():  # 忽略空行
            try:
                self._in_write = True
                self.log_func(text.rstrip())
            except Exception:
                # 如果出错，恢复到原始输出
                self.original_stdout.write(text)
            finally:
                self._in_write = False

    def flush(self):
        """刷新缓冲区"""
        pass

    def start(self):
        """开始重定向"""
        sys.stdout = self
        sys.stderr = self

    def stop(self):
        """停止重定向"""
        sys.stdout = self.original_stdout
        sys.stderr = self.original_stderr


class ProgressBar:
    """增强的进度条类，支持显示详细信息和自适应宽度"""

    def __init__(self, description="进度", width=60):
        """
        初始化进度条

        参数:
            description: 进度描述文本
            width: 进度条宽度（字符数）
        """
        self.description = description
        self.width = width
        self.last_printed = ""

    def update(self, current, total, sub_description=None):
        """
        更新进度条显示

        参数:
            current: 当前进度值
            total: 总数值
            sub_description: 子描述信息（如股票名称、年份等）
        """
        percent = current / total * 100
        filled_length = int(self.width * current / total)
        bar = '=' * filled_length + '-' * (self.width - filled_length)

        # 构建显示文本
        if sub_description:
            display_text = f"\r{self.description}: [{bar}] {percent:5.1f}% ({current}/{total}) - {sub_description}"
        else:
            display_text = f"\r{self.description}: [{bar}] {percent:5.1f}% ({current}/{total})"

        # 只在实际内容变化时打印，避免闪烁
        if display_text != self.last_printed:
            sys.stdout.write(display_text)
            sys.stdout.flush()
            self.last_printed = display_text

    def finish(self, message="完成"):
        """显示完成消息"""
        print(f"\n{message}")
        self.last_printed = ""


class FiveMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class OneMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class DayDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class TimeLineRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    price = tables.UInt32Col()           # IGNORE:E1101
    vol = tables.UInt64Col()             # IGNORE:E1101


class TransRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    price = tables.UInt32Col()           # IGNORE:E1101
    vol = tables.UInt64Col()             # IGNORE:E1101
    buyorsell = tables.UInt8Col()        # IGNORE:E1101


def get_table_desc(data_type):
    """根据数据类型获取表结构描述类

    参数:
        data_type: 数据类型 ('day', '5min', '1min', 'time', 'trans')

    返回:
        tables.IsDescription: 表结构描述类
    """
    table_map = {
        'day': DayDataRecordH5File,
        '5min': FiveMinDataRecordH5File,
        '1min': OneMinDataRecordH5File,
        'time': TimeLineRecordH5File,
        'trans': TransRecordH5File
    }

    if data_type not in table_map:
        raise ValueError(f"不支持的数据类型：{data_type}")

    return table_map[data_type]


def get_existing_stocks_in_merged_file(dest_filepath):
    """获取已存在合并文件中的股票列表及其最大 datetime 值

    参数:
        dest_filepath: 目标文件路径

    返回:
        dict: {stock_name: max_datetime}，如果文件不存在则返回空字典

    重要说明:
        必须遍历所有记录找出每只股票的最大 datetime 值，
        因为不能假设"最后一条记录"就是"时间上最新的数据"。
        特别是当数据可能乱序或部分更新时。
    """
    print(f"正在检查已存在的合并文件：{dest_filepath}")
    stock_max_dates = {}

    # 检查文件是否存在
    if not os.path.exists(dest_filepath):
        return stock_max_dates

    try:
        f = tables.open_file(dest_filepath, 'r')

        # 遍历/data 组下的所有股票表
        try:
            data_group = f.get_node('/data')
            for table in data_group:
                if isinstance(table, tables.Table):
                    stock_name = table._v_name

                    # 数据已按 datetime 排序，直接读取最后一条记录获取最大 datetime
                    if table.nrows > 0:
                        last_row = table[table.nrows - 1]
                        stock_max_dates[stock_name] = last_row['datetime']
        except Exception as e:
            print(f"  警告：无法访问/data 组：{e}")

        f.close()

        if stock_max_dates:
            print(f"  检测到已存在文件，包含 {len(stock_max_dates)} 只股票")

    except Exception as e:
        print(f"  读取已有文件时出错：{e}")

    return stock_max_dates


def find_year_files(src_dir, market_prefix, data_type):
    """查找指定目录下某市场和数据类型的按年份拆分文件

    参数:
        src_dir: 源目录
        market_prefix: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型（如 day, 5min, trans 等）

    返回:
        list: 匹配的文件路径列表，按年份排序
    """
    pattern = re.compile(rf'^{market_prefix}_{data_type}_\d{{4}}\.h5$')
    year_files = []

    for filename in os.listdir(src_dir):
        if pattern.match(filename):
            year_files.append(os.path.join(src_dir, filename))

    # 按文件名排序（即按年份排序）
    year_files.sort()

    return year_files


def find_year_files_recursive(base_dir, market, data_type):
    """递归查找指定市场和数据类型的年份文件（按年份从小到大排序）

    参数:
        base_dir: 基础目录
        market: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型（如 day, 5min, trans 等）

    返回:
        list: 匹配的文件路径列表，按年份从小到大排序

    重要说明:
        必须按年份从小到大排序，因为：
        1. 合并时需要按时间顺序追加数据
        2. 增量合并依赖 datetime 比较，必须先处理旧年份
        3. 保证 K 线数据的时序正确性
    """
    pattern = re.compile(rf'^{market}_{data_type}_\d{{4}}\.h5$')
    year_files = []

    # 遍历目录树
    for root, dirs, files in os.walk(base_dir):
        for filename in files:
            if pattern.match(filename):
                year_files.append(os.path.join(root, filename))

    # 关键：按文件名排序（即按年份 YYYY 排序，确保从小到大）
    # 例如：sh_day_2020.h5 < sh_day_2021.h5 < sh_day_2022.h5
    year_files.sort()

    print(f"  ✓ 找到 {len(year_files)} 个年份文件（已按年份排序）")
    if len(year_files) > 0:
        print(f"    - 最早年份：{os.path.basename(year_files[0])}")
        print(f"    - 最新年份：{os.path.basename(year_files[-1])}")

    return year_files


def scan_split_directory(src_dir, data_types):
    """扫描 split 工具的拆分目录结构，识别所有可用的市场和数据类型组合

    参数:
        src_dir: 基础目录
        data_types: 要处理的数据类型列表

    返回:
        dict: {(market, data_type): [year_files]}
    """
    result = {}

    print(f"\n正在扫描拆分目录：{src_dir}")
    print(f"数据类型：{', '.join(data_types)}")

    # 首先检查是否是扁平结构（所有年份文件直接在 src_dir 下）
    flat_files_found = False
    for data_type in data_types:
        pattern = re.compile(rf'^[a-z]{{2,4}}_{data_type}_\d{{4}}\.h5$')
        for filename in os.listdir(src_dir):
            if os.path.isfile(os.path.join(src_dir, filename)) and pattern.match(filename):
                flat_files_found = True
                break
        if flat_files_found:
            break

    # 如果是扁平结构，先处理扁平部分
    if flat_files_found:
        print(f"检测到扁平结构：所有年份文件直接在源目录下")
        for data_type in data_types:
            pattern = re.compile(rf'^([a-z]{{2,4}})_{data_type}_\d{{4}}\.h5$')
            markets_in_type = set()

            for filename in os.listdir(src_dir):
                match = pattern.match(filename)
                if match:
                    market = match.group(1)
                    markets_in_type.add(market)

            for market in sorted(markets_in_type):
                key = (market, data_type)
                year_files = find_year_files_recursive(src_dir, market, data_type)
                if year_files:
                    result[key] = year_files
                    print(f"  ✓ {market}_{data_type}: 找到 {len(year_files)} 个年份文件")

    # 继续扫描目录树结构（即使有扁平结构也要扫描）
    print(f"继续扫描目录树结构...")

    # 尝试扫描市场目录
    for item in os.listdir(src_dir):
        item_path = os.path.join(src_dir, item)
        if not os.path.isdir(item_path):
            continue

        # item 可能是市场目录（如 sh/, sz/）
        market = item

        # 在市场目录下查找数据类型目录
        for data_type in data_types:
            type_dir = os.path.join(item_path, data_type)
            if os.path.isdir(type_dir):
                key = (market, data_type)
                year_files = find_year_files_recursive(type_dir, market, data_type)
                if year_files:
                    if key not in result:
                        result[key] = year_files
                        print(f"  ✓ {market}/{data_type}: 找到 {len(year_files)} 个年份文件")
                    else:
                        # 合并文件列表（避免重复）
                        existing_paths = set(result[key])
                        new_count = 0
                        for f in year_files:
                            if f not in existing_paths:
                                result[key].append(f)
                                new_count += 1
                        if new_count > 0:
                            print(f"  ✓ {market}/{data_type}: 新增 {new_count} 个年份文件（共 {len(result[key])} 个）")

            # 也检查直接在市场目录下的年份文件
            year_files = find_year_files_recursive(item_path, market, data_type)
            if year_files:
                key = (market, data_type)
                if key not in result:
                    result[key] = year_files
                    print(f"  ✓ {market}/{data_type}: 找到 {len(year_files)} 个年份文件")
                else:
                    # 合并文件列表（避免重复）
                    existing_paths = set(result[key])
                    new_count = 0
                    for f in year_files:
                        if f not in existing_paths:
                            result[key].append(f)
                            new_count += 1
                    if new_count > 0:
                        print(f"  ✓ {market}/{data_type}: 新增 {new_count} 个年份文件（共 {len(result[key])} 个）")

    if not result:
        print(f"\n⚠ 未找到任何匹配的年份文件！")
        print(f"请确认目录结构是以下之一：")
        print(f"  扁平结构：{src_dir}/sh_day_2020.h5, {src_dir}/sh_day_2021.h5, ...")
        print(f"  目录结构：{src_dir}/sh/day/sh_day_2020.h5, {src_dir}/sh/day/sh_day_2021.h5, ...")

    return result


def merge_files_by_market(data_dir, dest_dir, market, data_type, table_desc):
    """
    合并特定市场和数据类型的所有年份文件（支持增量合并）

    参数:
        data_dir: 数据目录
        dest_dir: 目标目录
        market: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型
        table_desc: 表结构描述类
    """
    print(f"\n开始合并市场 {market} 的 {data_type} 数据...")

    # 查找所有年份文件
    year_files = find_year_files_recursive(data_dir, market, data_type)

    if not year_files:
        print(f"  未找到 {market}_{data_type}_*.h5 文件")
        return False

    print(f"  找到 {len(year_files)} 个年份文件")

    # 构建目标文件名
    dest_filename = f"{market}_{data_type}.h5"
    dest_filepath = os.path.join(dest_dir, dest_filename)

    print(f"  目标文件：{dest_filename}")

    # 获取已存在的股票信息（增量合并）
    existing_stocks = get_existing_stocks_in_merged_file(dest_filepath)

    if existing_stocks:
        print(f"  将进行增量合并，只处理新增或更新的股票")
    else:
        print(f"  创建新文件进行完整合并")

    # 创建或打开目标文件
    mode = "a" if os.path.exists(dest_filepath) else "w"
    dest_hdf5 = tables.open_file(dest_filepath, mode=mode,
                                 filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))

    try:
        # 获取或创建 data 组
        if os.path.exists(dest_filepath):
            try:
                dest_group = dest_hdf5.get_node("/data")
            except Exception:
                dest_group = dest_hdf5.create_group("/", "data", title="Stock Data")
        else:
            dest_group = dest_hdf5.create_group("/", "data", title="Stock Data")

        # 统计信息
        total_stocks = 0
        total_records = 0
        stocks_merged = set()
        stocks_appended = 0
        stocks_created = 0

        # 进度条
        progress = ProgressBar("    处理年份", 60)

        # 逐个处理年份文件
        for i, year_file in enumerate(year_files):
            progress.update(i + 1, len(year_files),
                            sub_description=f"{os.path.basename(year_file)}")

            try:
                src_hdf5 = tables.open_file(year_file, mode='r')

                # 遍历/data 组下的所有股票表
                try:
                    data_group = src_hdf5.get_node('/data')

                    for src_table in data_group:
                        if isinstance(src_table, tables.Table):
                            stock_name = src_table._v_name

                            # 获取该股票在已存在文件中的最后日期
                            existing_last_date = existing_stocks.get(stock_name, 0)
                            new_records_count = 0
                            skipped_records_count = 0

                            # 如果该股票表已存在，则追加新增数据；否则创建新表
                            if stock_name in stocks_merged:
                                # 表已在本次合并中创建或追加过
                                dest_table = dest_hdf5.get_node(dest_group, stock_name)

                                # 追加记录（如果有新数据）
                                for row in src_table:
                                    datetime_val = row['datetime']

                                    # 增量检查：跳过已有的数据
                                    if existing_last_date > 0 and datetime_val <= existing_last_date:
                                        skipped_records_count += 1
                                        continue

                                    dest_row = dest_table.row
                                    for col_name in dest_table.colnames:
                                        try:
                                            dest_row[col_name] = row[col_name]
                                        except KeyError:
                                            pass
                                    dest_row.append()
                                    new_records_count += 1

                                dest_table.flush()
                                total_records += new_records_count

                            else:
                                # 第一次遇到该股票
                                if stock_name in existing_stocks:
                                    # 股票表已存在，需要追加
                                    dest_table = dest_hdf5.get_node(dest_group, stock_name)

                                    # 追加记录（跳过已有数据）
                                    for row in src_table:
                                        datetime_val = row['datetime']

                                        # 增量检查：跳过已有的数据
                                        if existing_last_date > 0 and datetime_val <= existing_last_date:
                                            skipped_records_count += 1
                                            continue

                                        dest_row = dest_table.row
                                        for col_name in dest_table.colnames:
                                            try:
                                                dest_row[col_name] = row[col_name]
                                            except KeyError:
                                                pass
                                        dest_row.append()
                                        new_records_count += 1

                                    dest_table.flush()
                                    total_records += new_records_count
                                    stocks_appended += 1

                                else:
                                    # 创建新表
                                    dest_table = dest_hdf5.create_table(dest_group, stock_name, table_desc,
                                                                        title=stock_name)

                                    # 复制记录
                                    for row in src_table:
                                        dest_row = dest_table.row
                                        for col_name in dest_table.colnames:
                                            try:
                                                dest_row[col_name] = row[col_name]
                                            except KeyError:
                                                pass
                                        dest_row.append()

                                    dest_table.flush()
                                    total_stocks += 1
                                    total_records += src_table.nrows
                                    stocks_created += 1
                                    new_records_count = src_table.nrows

                            stocks_merged.add(stock_name)

                            # 打印该股票的处理情况
                            if existing_last_date > 0 and (skipped_records_count > 0 or new_records_count > 0):
                                print(f"      ✓ {stock_name}: 新增 {new_records_count:,} 条，跳过 {skipped_records_count:,} 条")
                            elif stock_name in existing_stocks:
                                print(f"      ✓ {stock_name}: 追加 {new_records_count:,} 条记录")
                            else:
                                print(f"      ✓ {stock_name}: 创建 {new_records_count:,} 条记录")

                except Exception as e:
                    print(f"      警告：无法访问/data 组：{e}")

                src_hdf5.close()

            except Exception as e:
                print(f"    读取文件 {os.path.basename(year_file)} 时出错：{e}")
                continue

        # 完成
        progress.finish(message=f"\n    ✓ 合并完成：{len(stocks_merged)} 只股票，{total_records:,} 条记录")
        print(f"  ✓ {market}_{data_type} 合并完成 -> {dest_filename}")
        print(f"    - 新建股票表：{stocks_created} 只")
        print(f"    - 追加数据：{stocks_appended} 只")

        return True

    except Exception as e:
        print(f"  写入目标文件时出错：{e}")
        import traceback
        traceback.print_exc()
        dest_hdf5.close()
        # 如果出错，删除不完整的目标文件
        if os.path.exists(dest_filepath):
            os.remove(dest_filepath)
        return False

    finally:
        try:
            dest_hdf5.close()
        except:
            pass


def merge_files_by_market_with_progress(data_dir, dest_dir, market, data_type, table_desc, stop_flag=None, progress_callback=None, market_index=0, total_markets=1):
    """
    合并特定市场和数据类型的所有年份文件（支持进度更新和可中断）

    参数:
        data_dir: 数据目录
        dest_dir: 目标目录
        market: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型
        table_desc: 表结构描述类
        stop_flag: 停止标志检查函数
        progress_callback: 进度回调函数，接收 (current, total, message) 参数
        market_index: 当前市场索引（用于计算总体进度）
        total_markets: 总市场数量

    返回:
        bool: 是否成功完成
    """
    print(f"\n开始合并市场 {market} 的 {data_type} 数据...")

    # 查找所有年份文件（已按年份排序）
    year_files = find_year_files_recursive(data_dir, market, data_type)

    if not year_files:
        print(f"  未找到 {market}_{data_type}_*.h5 文件")
        return False

    # 构建目标文件名
    dest_filename = f"{market}_{data_type}.h5"
    dest_filepath = os.path.join(dest_dir, dest_filename)

    print(f"  目标文件：{dest_filename}")

    # 获取已存在的股票信息（增量合并）
    existing_stocks = get_existing_stocks_in_merged_file(dest_filepath)

    if existing_stocks:
        print(f"  将进行增量合并，只处理新增或更新的股票")
    else:
        print(f"  创建新文件进行完整合并")

    # 创建或打开目标文件
    mode = "a" if os.path.exists(dest_filepath) else "w"
    dest_hdf5 = tables.open_file(dest_filepath, mode=mode,
                                 filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))

    try:
        # 获取或创建 data 组
        if os.path.exists(dest_filepath):
            try:
                dest_group = dest_hdf5.get_node("/data")
            except Exception:
                dest_group = dest_hdf5.create_group("/", "data", title="Stock Data")
        else:
            dest_group = dest_hdf5.create_group("/", "data", title="Stock Data")

        # 统计信息
        total_stocks = 0
        total_records = 0
        stocks_merged = set()
        stocks_appended = 0
        stocks_created = 0

        # 进度条
        progress = ProgressBar("    处理年份", 60)

        # 逐个处理年份文件
        for i, year_file in enumerate(year_files):
            # 检查停止标志
            if stop_flag and stop_flag():
                print(f"\n  用户取消处理年份文件：{os.path.basename(year_file)}")
                return False

            progress.update(i + 1, len(year_files),
                            sub_description=f"{os.path.basename(year_file)}")

            # 调用进度回调，更新 GUI 进度
            if progress_callback:
                # 计算总体进度：市场进度 + 年份进度
                market_progress = market_index / total_markets * 100
                year_progress = (i + 1) / len(year_files) / total_markets * 100
                overall_progress = int(market_progress + year_progress)

                progress_callback(overall_progress, 100,
                                  f"处理年份 {i+1}/{len(year_files)} - {os.path.basename(year_file)}")

            try:
                src_hdf5 = tables.open_file(year_file, mode='r')

                # 遍历/data 组下的所有股票表
                try:
                    data_group = src_hdf5.get_node('/data')

                    stock_count = 0
                    for src_table in data_group:
                        if isinstance(src_table, tables.Table):
                            stock_name = src_table._v_name

                            # 每处理 100 只股票，让出一点时间给 GUI
                            stock_count += 1
                            if stock_count % 100 == 0:
                                # 短暂休眠，让 GUI 有机会处理事件
                                import time
                                time.sleep(0.001)  # 1ms

                                # 再次检查停止标志
                                if stop_flag and stop_flag():
                                    print(f"\n  用户取消处理，当前股票：{stock_name}")
                                    return False

                            # 获取该股票在已存在文件中的最后日期
                            existing_last_date = existing_stocks.get(stock_name, 0)
                            new_records_count = 0
                            skipped_records_count = 0

                            # 如果该股票表已存在，则追加新增数据；否则创建新表
                            if stock_name in stocks_merged:
                                # 表已在本次合并中创建或追加过
                                dest_table = dest_hdf5.get_node(dest_group, stock_name)

                                # 追加记录（如果有新数据）
                                for row in src_table:
                                    datetime_val = row['datetime']

                                    # 增量检查：跳过已有的数据
                                    if existing_last_date > 0 and datetime_val <= existing_last_date:
                                        skipped_records_count += 1
                                        continue

                                    dest_row = dest_table.row
                                    for col_name in dest_table.colnames:
                                        try:
                                            dest_row[col_name] = row[col_name]
                                        except KeyError:
                                            pass
                                    dest_row.append()
                                    new_records_count += 1

                                dest_table.flush()
                                total_records += new_records_count

                            else:
                                # 第一次遇到该股票
                                if stock_name in existing_stocks:
                                    # 股票表已存在，需要追加
                                    dest_table = dest_hdf5.get_node(dest_group, stock_name)

                                    # 追加记录（跳过已有数据）
                                    for row in src_table:
                                        datetime_val = row['datetime']

                                        # 增量检查：跳过已有的数据
                                        if existing_last_date > 0 and datetime_val <= existing_last_date:
                                            skipped_records_count += 1
                                            continue

                                        dest_row = dest_table.row
                                        for col_name in dest_table.colnames:
                                            try:
                                                dest_row[col_name] = row[col_name]
                                            except KeyError:
                                                pass
                                        dest_row.append()
                                        new_records_count += 1

                                    dest_table.flush()
                                    total_records += new_records_count
                                    stocks_appended += 1

                                else:
                                    # 创建新表
                                    dest_table = dest_hdf5.create_table(dest_group, stock_name, table_desc,
                                                                        title=stock_name)

                                    # 复制记录
                                    for row in src_table:
                                        dest_row = dest_table.row
                                        for col_name in dest_table.colnames:
                                            try:
                                                dest_row[col_name] = row[col_name]
                                            except KeyError:
                                                pass
                                        dest_row.append()

                                    dest_table.flush()
                                    total_stocks += 1
                                    total_records += src_table.nrows
                                    stocks_created += 1
                                    new_records_count = src_table.nrows

                            stocks_merged.add(stock_name)

                            # 打印该股票的处理情况
                            if existing_last_date > 0 and (skipped_records_count > 0 or new_records_count > 0):
                                print(f"      ✓ {stock_name}: 新增 {new_records_count:,} 条，跳过 {skipped_records_count:,} 条")
                            elif stock_name in existing_stocks:
                                print(f"      ✓ {stock_name}: 追加 {new_records_count:,} 条记录")
                            else:
                                print(f"      ✓ {stock_name}: 创建 {new_records_count:,} 条记录")

                except Exception as e:
                    print(f"      警告：无法访问/data 组：{e}")

                src_hdf5.close()

            except Exception as e:
                print(f"    读取文件 {os.path.basename(year_file)} 时出错：{e}")
                continue

        # 完成
        progress.finish(message=f"\n    ✓ 合并完成：{len(stocks_merged)} 只股票，{total_records:,} 条记录")
        print(f"  ✓ {market}_{data_type} 合并完成 -> {dest_filename}")
        print(f"    - 新建股票表：{stocks_created} 只")
        print(f"    - 追加数据：{stocks_appended} 只")

        return True

    except Exception as e:
        print(f"  写入目标文件时出错：{e}")
        import traceback
        traceback.print_exc()
        dest_hdf5.close()
        # 如果出错，删除不完整的目标文件
        if os.path.exists(dest_filepath):
            os.remove(dest_filepath)
        return False

    finally:
        try:
            dest_hdf5.close()
        except:
            pass


def merge_all_data(src_dir, dest_dir, data_types):
    """
    合并所有选定数据类型的数据

    参数:
        src_dir: 源目录
        dest_dir: 目标目录
        data_types: 要合并的数据类型列表
    """
    print(f"\n{'='*80}")
    print(f"开始批量合并数据")
    print(f"源目录：{src_dir}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{', '.join(data_types)}")

    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)

    # 查找所有市场
    markets = set()
    for data_type in data_types:
        pattern = re.compile(rf'^[a-z]{{2,4}}_{data_type}_\d{{4}}\.h5$')
        for filename in os.listdir(src_dir):
            match = pattern.match(filename)
            if match:
                market = filename.split('_')[0]
                markets.add(market)

    if not markets:
        print("\n未找到任何匹配的数据文件！")
        return False

    print(f"\n发现市场：{', '.join(sorted(markets))}")

    # 为每个市场创建子目录
    success_count = 0
    total_tasks = len(markets) * len(data_types)
    task_count = 0

    for market in sorted(markets):
        # 创建市场子目录
        market_dest_dir = os.path.join(dest_dir, market)
        os.makedirs(market_dest_dir, exist_ok=True)

        for data_type in data_types:
            task_count += 1
            print(f"\n[{task_count}/{total_tasks}] 处理 {market} 市场的 {data_type} 数据")

            try:
                table_desc = get_table_desc(data_type)

                if merge_files_by_market(src_dir, market_dest_dir, market, data_type, table_desc):
                    success_count += 1
            except Exception as e:
                print(f"处理 {market}_{data_type} 时出错：{e}")
                continue

    print(f"\n{'='*80}")
    print(f"批量合并完成！成功：{success_count}/{total_tasks}")

    return success_count > 0


def merge_all_data_from_split(src_dir, dest_dir, data_types, selected_markets=None, stop_flag=None, progress_callback=None):
    """
    从 split 工具的拆分目录合并数据

    参数:
        src_dir: split 工具的拆分目录（可以是扁平或目录树结构）
        dest_dir: 目标目录（合并后的文件保存位置）
        data_types: 要合并的数据类型列表
        selected_markets: 选定的市场列表，如 ['sh', 'sz', 'bj']，None 表示全部市场
        stop_flag: 停止标志检查函数，返回 True 表示应该停止
        progress_callback: 进度回调函数，接收 (current, total, message) 参数

    返回:
        bool: 是否成功完成
    """
    print(f"\n{'='*80}")
    print(f"开始从 split 拆分目录合并数据")
    print(f"源目录：{src_dir}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{', '.join(data_types)}")
    if selected_markets:
        print(f"选定市场：{', '.join(selected_markets)}")
    else:
        print(f"选定市场：全部市场")

    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)

    # 扫描拆分目录
    merge_tasks = scan_split_directory(src_dir, data_types)

    if not merge_tasks:
        print("\n没有需要合并的数据！")
        return False

    # 过滤市场
    if selected_markets:
        original_tasks = merge_tasks
        merge_tasks = {}
        for (market, data_type), year_files in original_tasks.items():
            if market.lower() in [m.lower() for m in selected_markets]:
                merge_tasks[(market, data_type)] = year_files

        if not merge_tasks:
            print(f"\n未找到选定市场 ({', '.join(selected_markets)}) 的数据！")
            return False

        print(f"\n过滤后剩余：{len(merge_tasks)} 个合并任务（已按市场过滤）")

    total_tasks = len(merge_tasks)
    print(f"\n总计：{total_tasks} 个合并任务")

    # 执行合并
    success_count = 0
    task_index = 0

    # 将 merge_tasks 转换为列表以便获取索引
    tasks_list = list(merge_tasks.items())
    total_tasks = len(tasks_list)

    print(f"\n总计：{total_tasks} 个合并任务")

    for market_index, ((market, data_type), year_files) in enumerate(tasks_list):
        task_index += 1

        # 检查停止标志
        if stop_flag and stop_flag():
            print(f"\n用户取消处理，已完成 {task_index-1}/{total_tasks} 个任务")
            break

        # 更新进度（使用市场索引计算总体进度）
        progress = int(market_index / total_tasks * 100)
        if progress_callback:
            progress_callback(progress, total_tasks, f"准备处理：{market_index+1}/{total_tasks} - {market}_{data_type}")

        print(f"\n[{task_index}/{total_tasks}] 合并 {market} 市场的 {data_type} 数据")

        try:
            table_desc = get_table_desc(data_type)

            # 使用增强版的合并函数，支持增量合并、停止检查和进度回调
            if merge_files_by_market_with_progress(src_dir, dest_dir, market, data_type, table_desc,
                                                   stop_flag=stop_flag,
                                                   progress_callback=progress_callback,
                                                   market_index=market_index,
                                                   total_markets=total_tasks):
                success_count += 1

        except Exception as e:
            print(f"  处理 {market}_{data_type} 时出错：{e}")
            import traceback
            traceback.print_exc()
            continue

    # 最终进度更新
    if progress_callback:
        progress_callback(100, total_tasks, "完成")

    print(f"\n{'='*80}")
    completed_msg = f"批量合并完成！成功：{success_count}/{total_tasks}"
    print(completed_msg)

    return success_count > 0


class MergeApp:
    """数据文件合并 GUI 应用"""

    def __init__(self, root):
        self.root = root
        self.root.title("Hikyuu 数据文件按年份合并工具")
        self.root.geometry("800x600")

        # 变量
        self.src_dir = tk.StringVar()
        self.dest_dir = tk.StringVar()

        # 市场选择变量
        self.market_sh = tk.BooleanVar(value=True)  # 上证
        self.market_sz = tk.BooleanVar(value=True)  # 深证
        self.market_bj = tk.BooleanVar(value=True)  # 北证

        self.process_day = tk.BooleanVar(value=True)
        self.process_1min = tk.BooleanVar(value=False)
        self.process_5min = tk.BooleanVar(value=False)
        self.process_timeline = tk.BooleanVar(value=False)
        self.process_trans = tk.BooleanVar(value=False)

        self.log_queue = queue.Queue()
        self.is_processing = False

        self._create_widgets()
        self._start_log_processor()

    def _create_widgets(self):
        """创建界面组件"""
        # 主框架
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # 源目录选择
        src_frame = ttk.LabelFrame(main_frame, text="源数据目录（split 拆分后的目录）", padding="5")
        src_frame.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Entry(src_frame, textvariable=self.src_dir, width=60).grid(row=0, column=0, padx=5)
        ttk.Button(src_frame, text="浏览...", command=self._browse_src).grid(row=0, column=1)

        # 提示标签
        hint_label = ttk.Label(src_frame,
                               text="提示：可选择 split 生成的根目录或包含年份文件的子目录",
                               font=('Arial', 8),
                               foreground='gray')
        hint_label.grid(row=1, column=0, columnspan=2, sticky=tk.W, pady=2)

        # 目标目录选择
        dest_frame = ttk.LabelFrame(main_frame, text="目标目录（合并后的文件保存位置）", padding="5")
        dest_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Entry(dest_frame, textvariable=self.dest_dir, width=60).grid(row=0, column=0, padx=5)
        ttk.Button(dest_frame, text="浏览...", command=self._browse_dest).grid(row=0, column=1)

        # 市场选择
        market_frame = ttk.LabelFrame(main_frame, text="选择市场", padding="5")
        market_frame.grid(row=2, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Checkbutton(market_frame, text="上证 (SH)", variable=self.market_sh).grid(row=0, column=0, padx=15)
        ttk.Checkbutton(market_frame, text="深证 (SZ)", variable=self.market_sz).grid(row=0, column=1, padx=15)
        ttk.Checkbutton(market_frame, text="北证 (BJ)", variable=self.market_bj).grid(row=0, column=2, padx=15)

        # 市场全选/全不选按钮
        market_select_frame = ttk.Frame(market_frame)
        market_select_frame.grid(row=1, column=0, columnspan=3, pady=5)

        ttk.Button(market_select_frame, text="市场全选", command=self._select_all_markets).pack(side=tk.LEFT, padx=5)
        ttk.Button(market_select_frame, text="市场全不选", command=self._select_none_markets).pack(side=tk.LEFT, padx=5)

        # 数据类型选择
        type_frame = ttk.LabelFrame(main_frame, text="要合并的数据类型", padding="5")
        type_frame.grid(row=3, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Checkbutton(type_frame, text="日线 (day)", variable=self.process_day).grid(row=0, column=0, padx=10)
        ttk.Checkbutton(type_frame, text="1 分钟线 (1min)", variable=self.process_1min).grid(row=0, column=1, padx=10)
        ttk.Checkbutton(type_frame, text="5 分钟线 (5min)", variable=self.process_5min).grid(row=0, column=2, padx=10)
        ttk.Checkbutton(type_frame, text="分时 (time)", variable=self.process_timeline).grid(row=0, column=3, padx=10)
        ttk.Checkbutton(type_frame, text="分笔 (trans)", variable=self.process_trans).grid(row=0, column=4, padx=10)

        # 全选/全不选按钮
        select_frame = ttk.Frame(type_frame)
        select_frame.grid(row=1, column=0, columnspan=5, pady=5)

        ttk.Button(select_frame, text="数据类型全选", command=self._select_all_types).pack(side=tk.LEFT, padx=5)
        ttk.Button(select_frame, text="数据类型全不选", command=self._select_none_types).pack(side=tk.LEFT, padx=5)

        # 开始按钮
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=4, column=0, columnspan=3, pady=10)

        self.start_button = ttk.Button(button_frame, text="开始合并", command=self._start_merge, width=20)
        self.start_button.pack(side=tk.LEFT, padx=5)

        self.stop_button = ttk.Button(button_frame, text="停止", command=self._stop_merge, width=20, state=tk.DISABLED)
        self.stop_button.pack(side=tk.LEFT, padx=5)

        # 进度条
        progress_frame = ttk.LabelFrame(main_frame, text="进度", padding="5")
        progress_frame.grid(row=5, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(progress_frame, variable=self.progress_var, maximum=100, mode='determinate')
        self.progress_bar.grid(row=0, column=0, sticky=(tk.W, tk.E))

        self.status_label = ttk.Label(progress_frame, text="就绪", font=('Arial', 9))
        self.status_label.grid(row=1, column=0, sticky=tk.W, pady=5)

        # 详细信息标签（显示当前处理的文件、股票等信息）
        self.detail_label = ttk.Label(progress_frame, text="", font=('Arial', 8), foreground='gray')
        self.detail_label.grid(row=2, column=0, sticky=tk.W, pady=2)

        # 日志窗口
        log_frame = ttk.LabelFrame(main_frame, text="处理日志", padding="5")
        log_frame.grid(row=6, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)

        self.log_text = tk.Text(log_frame, height=15, width=80, state=tk.DISABLED)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        scrollbar = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.log_text.configure(yscrollcommand=scrollbar.set)

        # 配置网格权重
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.columnconfigure(2, weight=1)
        main_frame.rowconfigure(6, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
        progress_frame.columnconfigure(0, weight=1)

    def _browse_src(self):
        """浏览源目录"""
        directory = filedialog.askdirectory(title="选择源数据目录")
        if directory:
            self.src_dir.set(directory)
            self._log(f"源目录：{directory}")

    def _browse_dest(self):
        """浏览目标目录"""
        directory = filedialog.askdirectory(title="选择目标目录")
        if directory:
            self.dest_dir.set(directory)
            self._log(f"目标目录：{directory}")

    def _select_all(self):
        """全选"""
        self.process_day.set(True)
        self.process_1min.set(True)
        self.process_5min.set(True)
        self.process_timeline.set(True)
        self.process_trans.set(True)

    def _select_none(self):
        """全不选"""
        self.process_day.set(False)
        self.process_1min.set(False)
        self.process_5min.set(False)
        self.process_timeline.set(False)
        self.process_trans.set(False)

    def _select_all_markets(self):
        """市场全选"""
        self.market_sh.set(True)
        self.market_sz.set(True)
        self.market_bj.set(True)

    def _select_none_markets(self):
        """市场全不选"""
        self.market_sh.set(False)
        self.market_sz.set(False)
        self.market_bj.set(False)

    def _select_all_types(self):
        """数据类型全选"""
        self.process_day.set(True)
        self.process_1min.set(True)
        self.process_5min.set(True)
        self.process_timeline.set(True)
        self.process_trans.set(True)

    def _select_none_types(self):
        """数据类型全不选"""
        self.process_day.set(False)
        self.process_1min.set(False)
        self.process_5min.set(False)
        self.process_timeline.set(False)
        self.process_trans.set(False)

    def _log(self, message):
        """添加日志"""
        self.log_queue.put(message)

    def _start_log_processor(self):
        """启动日志处理器（优化版，提高响应性）"""
        def process_logs():
            try:
                messages = []
                # 批量处理日志消息，减少队列操作次数
                while True:
                    try:
                        message = self.log_queue.get_nowait()
                        messages.append(message)
                    except queue.Empty:
                        break

                # 批量更新 GUI
                if messages:
                    self.log_text.configure(state=tk.NORMAL)
                    for message in messages:
                        self.log_text.insert(tk.END, message + "\n")
                    self.log_text.see(tk.END)
                    self.log_text.configure(state=tk.DISABLED)

            except Exception as e:
                # 记录错误但不影响 GUI
                pass

            # 缩短检查间隔，提高响应性
            self.root.after(50, process_logs)

        self.root.after(50, process_logs)

    def _update_progress(self, value, status="", detail=""):
        """更新进度（确保在主线程执行）"""
        # 使用 after 确保在主线程更新
        self.root.after(0, lambda: self._do_update_progress(value, status, detail))

    def _do_update_progress(self, value, status="", detail=""):
        """实际执行进度更新（内部方法）"""
        try:
            self.progress_var.set(value)
            if status:
                self.status_label.config(text=status)
            if detail:
                self.detail_label.config(text=detail)
        except Exception:
            # 忽略 GUI 更新错误，避免影响处理流程
            pass

    def _validate_selection(self):
        """验证选择"""
        if not self.src_dir.get():
            messagebox.showerror("错误", "请选择源数据目录！")
            return False

        if not self.dest_dir.get():
            messagebox.showerror("错误", "请选择目标目录！")
            return False

        if not os.path.isdir(self.src_dir.get()):
            messagebox.showerror("错误", "源目录不存在！")
            return False

        # 检查是否至少选择了一种数据类型
        if not any([self.process_day.get(), self.process_1min.get(),
                   self.process_5min.get(), self.process_timeline.get(),
                   self.process_trans.get()]):
            messagebox.showerror("错误", "请至少选择一种数据类型！")
            return False

        # 检查是否至少选择了一个市场
        if not any([self.market_sh.get(), self.market_sz.get(), self.market_bj.get()]):
            messagebox.showerror("错误", "请至少选择一个市场！")
            return False

        return True

    def _process_merge(self, data_types, selected_markets):
        """执行合并操作（支持日志重定向和可中断处理）"""
        # 创建日志重定向器
        redirector = GUILogRedirector(self._log)
        redirector.start()

        try:
            self._log("=" * 60)
            self._log("开始合并数据...")
            self._update_progress(0, "正在扫描目录结构...")

            src_path = str(self.src_dir.get())
            dest_path = str(self.dest_dir.get())

            # 创建进度回调函数
            def progress_callback(current, total, message):
                """定期更新进度（每 5% 更新一次，避免过于频繁）"""
                # 计算进度百分比
                progress = int(current / total * 100)
                # 只在进度变化超过 5% 时更新，减少 GUI 刷新频率
                if not hasattr(progress_callback, 'last_progress') or abs(progress - progress_callback.last_progress) >= 5:
                    progress_callback.last_progress = progress
                    self._update_progress(progress, f"处理中：{current}/{total}", message)

                # 检查停止标志
                if not self.is_processing:
                    raise InterruptedError("用户取消处理")

            progress_callback.last_progress = 0

            # 使用新的智能合并函数，传入停止标志和进度回调
            success = merge_all_data_from_split(src_path, dest_dir=dest_path,
                                                data_types=data_types,
                                                selected_markets=selected_markets,
                                                stop_flag=lambda: not self.is_processing,
                                                progress_callback=progress_callback)

            if self.is_processing:  # 正常完成
                if success:
                    self._update_progress(100, "合并完成")
                    self._log("=" * 60)
                    self._log("数据合并完成！")
                    # 在主线程中显示对话框
                    self.root.after(0, lambda: messagebox.showinfo("完成", "数据合并完成！\n请查看目标目录。"))
                else:
                    self._update_progress(100, "无数据可合并")
                    self._log("=" * 60)
                    self._log("未找到需要合并的数据文件！")
                    self.root.after(0, lambda: messagebox.showwarning(
                        "提示", "未找到需要合并的数据文件！\n\n请确认：\n1. 源目录选择正确（split 拆分后的目录）\n2. 数据类型勾选正确\n3. 年份文件命名格式为：{market}_{type}_{YYYY}.h5"))
            else:  # 用户取消
                self._update_progress(0, "已取消")
                self._log("=" * 60)
                self._log("处理已取消！")
                self.root.after(0, lambda: messagebox.showinfo("取消", "处理已取消！"))

        except InterruptedError:
            # 用户主动取消
            self._update_progress(0, "已取消")
            self._log("=" * 60)
            self._log("处理已取消！")
            self.root.after(0, lambda: messagebox.showinfo("取消", "处理已取消！"))

        except Exception as e:
            self._log(f"\n发生错误：{str(e)}")
            self._update_progress(0, "错误")
            import traceback
            self._log(traceback.format_exc())
            self.root.after(0, lambda: messagebox.showerror("错误", f"合并过程中发生错误：\n{str(e)}"))

        finally:
            # 停止日志重定向
            redirector.stop()

            # 恢复按钮状态
            self.is_processing = False
            self.root.after(0, lambda: self.start_button.config(state=tk.NORMAL))
            self.root.after(0, lambda: self.stop_button.config(state=tk.DISABLED))

    def _start_merge(self):
        """开始合并"""
        if not self._validate_selection():
            return

        if self.is_processing:
            messagebox.showwarning("警告", "正在处理中，请稍候...")
            return

        self.is_processing = True
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)

        # 收集选定的数据类型
        data_types = []
        if self.process_day.get():
            data_types.append('day')
        if self.process_1min.get():
            data_types.append('1min')
        if self.process_5min.get():
            data_types.append('5min')
        if self.process_timeline.get():
            data_types.append('time')
        if self.process_trans.get():
            data_types.append('trans')

        # 收集选定的市场
        selected_markets = []
        if self.market_sh.get():
            selected_markets.append('sh')
        if self.market_sz.get():
            selected_markets.append('sz')
        if self.market_bj.get():
            selected_markets.append('bj')

        # 在新线程中执行
        thread = threading.Thread(target=self._process_merge, args=(data_types, selected_markets), daemon=True)
        thread.start()

    def _stop_merge(self):
        """停止合并"""
        if messagebox.askyesno("确认", "确定要停止当前处理吗？"):
            self.is_processing = False
            self._log("\n正在停止...")


def main():
    """主函数"""
    root = tk.Tk()
    app = MergeApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
