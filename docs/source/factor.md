# 因子管理

hikyuu提供了完善的因子管理体系，包括单个因子 Factor 和因子集合 FactorSet 的管理。因子是量化分析的基础构建块，通常由技术指标或其他计算公式构成。

## 最佳实践

1. **命名规范**: 因子名称应具有描述性且不区分大小写
2. **因子公式**: 因子公式为 `Indicator` 对象且不能为 `PRICELIST`。通常不使用截面值，截面、标准化等由 MF 计算时处理。
3. **因子集组织**: 将相关的因子组织到同一个FactorSet中便于管理
4. **数据验证**: 使用 `check=True` 参数验证股票列表是否属于指定板块
5. **因子更新**: 每日行情数据下载完成后，应及时调用 `update_all_factors_values()` 更新所有存储的因子值，确保因子数据与行情数据同步
6. **持久化管理**: 对于高频聚合的因子值或高频因子值，建议设置 `need_persist=True` 并保存到数据库。由于 Hikyuu的超高计算速度，普通的日频因子值，通常不建议保存到数据库，因为从存储中读取因子值速度更慢。可以自行测试决定。
7. **VIP功能使用**: ⚠️ 因子相关的数据库存储和读取操作均为VIP功能，数据库引擎仅支持ClickHouse。使用前请确认已获得相应权限。包括但不限于：`save_to_db()`、`load_from_db()`、`remove_from_db()`、`save_values()`、`get_all_values()`、`get_values()` 等涉及数据库的操作方法。

## 注意事项

- 因子名称和K线类型的组合是唯一的标识符
- FactorSet 添加同名因子时会覆盖原有的因子
- 因子集中的因子保持添加顺序
- 因子计算结果的日期对齐需要参考证券进行同步
- ⚠️ 所有涉及数据库的操作方法均为VIP功能，仅支持ClickHouse数据库引擎

## Factor类

Factor 类用于表示单个因子，包含了因子的基本信息、计算公式和相关属性。

### 构造函数

```
# 默认构造函数
Factor()

# 只指定名称和K线类型（用于从数据库加载已有因子）
Factor(name, ktype=KQuery.DAY)

# 创建新的因子对象
Factor(name, formula, ktype=KQuery.DAY, brief="", details="", need_persist=False, start_date=Datetime.min(), block=Block())
```

**参数说明:**

- `name` (str): 因子名称
- `formula` (Indicator): 计算公式指标，一旦创建不可更改
- `ktype` (KQuery.KType): K线类型，默认为日线
- `brief` (str): 简要描述，默认为空
- `details` (str): 详细描述，默认为空
- `need_persist` (bool): 是否需要持久化，默认为False。设置为True时，将因子指定股票集合从start_date开始的计算值保存到数据库中。由于Hikyuu的计算速度远快于数据库存储，通常日频因子计算的各证券值不建议保存到数据库，需要保存到数据库的通常为高频聚合到日频的因子值或高频因子值
- `start_date` (Datetime): 开始日期，数据存储时的起始日期，默认为最小日期
- `block` (Block): 板块信息，证券集合，如果为空则为全部，默认为空

**注意:** 因子名称不区分大小写，以 `name + ktype` 作为唯一标识

### 属性

| 属性             | 类型         | 描述             |
| ---------------- | ------------ | ---------------- |
| `name`         | str          | 因子名称         |
| `ktype`        | KQuery.KType | 因子频率类型     |
| `create_at`    | Datetime     | 创建日期         |
| `update_at`    | Datetime     | 更改日期         |
| `formula`      | Indicator    | 因子计算公式     |
| `start_date`   | Datetime     | 数据存储起始日期 |
| `block`        | Block        | 证券集合         |
| `brief`        | str          | 基础说明         |
| `details`      | str          | 详细说明         |
| `need_persist` | bool         | 是否持久化       |

### 方法

#### 数据库操作 ⚠️ VIP功能

<div class="admonition note">
<p class="admonition-title">注意</p>
<p>以下数据库操作方法均为VIP功能，仅支持ClickHouse数据库引擎。</p>
</div>

```
# 保存因子元数据到数据库 ⚠️ VIP功能
factor.save_to_db()

# 从数据库中删除因子及其数据 ⚠️ VIP功能
factor.remove_from_db()

# 从数据库中加载因子元数据 ⚠️ VIP功能
factor.load_from_db()
```

#### 数据获取 ⚠️ VIP功能

<div class="admonition note">
<p class="admonition-title">注意</p>
<p>以下数据获取方法涉及数据库操作，均为VIP功能，仅支持ClickHouse数据库引擎。</p>
</div>

```
# 获取所有计算结果 ⚠️ VIP功能
results = factor.get_all_values(query, align=False, fill_null=False, tovalue=False, align_dates=DatetimeList())

# 获取指定证券列表的计算结果 ⚠️ VIP功能
results = factor.get_values(stocks, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())

# 保存计算结果到数据库 ⚠️ VIP功能
factor.save_values(stocks, query, replace=False)
```

**参数说明:**

- `query` (Query): 查询参数
- `align` (bool): 是否对齐日期，默认False
- `fill_null` (bool): 是否填充空值，默认False
- `tovalue` (bool): 是否转换为数值，默认False
- `check` (bool): 是否检查股票列表属于自身指定的block，默认False（仅get_values方法）
- `align_dates` (DatetimeList): 对齐日期列表，默认为空
- `stocks` (sequence): 证券列表
- `replace` (bool): 是否替换已有数据，默认False

### 使用示例

```
from hikyuu import *

# 创建简单的移动平均因子
ma5 = MA(CLOSE(), 5)
ma5_factor = Factor("MA5", ma5, KQuery.DAY, "5日均线因子", "简单移动平均线指标")

# 查看因子信息
print(f"因子名称: {ma5_factor.name}")
print(f"因子类型: {ma5_factor.ktype}")
print(f"简要说明: {ma5_factor.brief}")

# 保存到数据库
ma5_factor.save_to_db()

# 获取计算结果
stocks = [sm['sh600000'], sm['sz000001']]
query = Query(Datetime(20240101), Datetime(20241231))
results = ma5_factor.get_values(stocks, query)
```

## FactorSet类

`FactorSet` 类用于管理一组相关的因子，提供批量操作和集合管理功能。

### 构造函数

```
# 默认构造函数
FactorSet()

# 创建指定名称和类型的因子集
FactorSet(name, ktype=KQuery.DAY, block=Block())

# 使用指标列表创建因子集合
FactorSet(inds, ktype=KQuery.DAY)
```

**参数说明:**

- `name` (str): 因子集名称
- `ktype` (KQuery.KType): K线类型，默认为日线
- `block` (Block): 板块信息，证券集合，默认为空
- `inds` (list): 指标列表

**注意:** 使用指标列表创建时，同名的指标会被覆盖，最终保留最后一个同名指标

### 属性

| 属性      | 类型         | 描述             |
| --------- | ------------ | ---------------- |
| `name`  | str          | 因子集名称       |
| `ktype` | KQuery.KType | 因子集频率类型   |
| `block` | Block        | 因子集对应的板块 |

### 基本方法

```
# 检查是否为空
is_empty = factor_set.empty()

# 清空因子集
factor_set.clear()

# 检查是否为null值
is_null = factor_set.is_null()

# 获取因子数量
count = len(factor_set)
```

### 因子管理方法

```
# 添加因子
factor_set.add(factor)           # 添加单个Factor对象
factor_set.add(indicator)        # 添加Indicator对象
factor_set.add(indicator_list)   # 添加Indicator列表
factor_set.add(factor_list)      # 添加Factor列表

# 检查是否存在指定因子
exists = factor_set.has_factor("MA5")

# 获取指定因子
factor = factor_set.get_factor("MA5")

# 移除因子
factor_set.remove_factor("MA5")

# 获取所有因子
factors = factor_set.get_factors()
```

### 数据获取方法

```
# 获取所有因子的计算结果
all_results = factor_set.get_all_values(query, align=False, fill_null=False, tovalue=False, align_dates=DatetimeList())

# 获取指定证券列表的计算结果
results = factor_set.get_values(stocks, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())
```

**参数说明:**

- `query` (Query): 查询参数
- `align` (bool): 是否对齐日期，默认False
- `fill_null` (bool): 是否填充空值，默认False
- `tovalue` (bool): 是否转换为数值，默认False
- `check` (bool): 是否检查股票列表属于自身指定的block，默认False
- `align_dates` (DatetimeList): 对齐日期列表，默认为空
- `stocks` (sequence): 证券列表

### 迭代器支持

```
# 遍历所有因子
for factor in factor_set:
    print(f"因子名称: {factor.name}")

# 通过索引访问
first_factor = factor_set[0]
named_factor = factor_set["MA5"]
```

### 使用示例

```
from hikyuu import *

# 创建多个技术指标因子
ma5 = MA(CLOSE(), 5)
ma10 = MA(CLOSE(), 10)
rsi = RSI(CLOSE(), 14)

# 设置因子名称
ma5.name = "MA5"
ma10.name = "MA10" 
rsi.name = "RSI"

# 创建因子集
factor_set = FactorSet("技术指标因子集", KQuery.DAY)

# 添加因子
factor_set.add(ma5)
factor_set.add(ma10)
factor_set.add(rsi)

# 或者直接从指标列表创建
indicators = [ma5, ma10, rsi]
factor_set2 = FactorSet(indicators, KQuery.DAY)

print(f"因子集包含 {len(factor_set)} 个因子")

# 遍历因子
for factor in factor_set:
    print(f"- {factor.name}: {factor.brief}")

# 获取计算结果
stocks = blocka.get_stock_list()[:10]  # 获取前10只A股
query = Query(Datetime(20240101), Datetime(20241231))
results = factor_set.get_values(stocks, query)

# 检查特定因子
if factor_set.has_factor("MA5"):
    ma5_factor = factor_set.get_factor("MA5")
    print(f"找到MA5因子: {ma5_factor.details}")
```

## 与多因子合成的结合使用

因子和因子集可以与多因子合成算法配合使用：

```
from hikyuu import *

# 方法1: 使用Indicator列表
indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
weights = [0.6, 0.4]
mf1 = MF_Weight(indicators, stocks, weights, query)

# 方法2: 使用FactorSet
factor_set = FactorSet(indicators)
mf2 = MF_Weight(factor_set, stocks, weights, query)

# 等权重合成
mf_equal = MF_EqualWeight(factor_set, stocks, query)
```

## 全局因子管理函数 ⚠️ VIP功能

<div class="admonition warning">
<p class="admonition-title">⚠️ 重要提醒</p>
<p>以下所有全局因子管理函数均为VIP功能，数据库引擎仅支持ClickHouse。</p>
</div>

除了Factor和FactorSet类的方法外，hikyuu还提供了一系列全局函数用于因子的数据库管理和批量操作。

### 因子数据库操作 ⚠️ VIP功能

#### get_factor(name[, ktype=KQuery.DAY]) ⚠️ VIP功能

获取指定名称和类型的因子元数据

```python
from hikyuu import *

# 获取日线因子 ⚠️ VIP功能
factor = get_factor("MA5")
# 获取周线因子 ⚠️ VIP功能
weekly_factor = get_factor("MA5", KQuery.WEEK)
```

**参数说明:**

- `name` (str): 因子名称
- `ktype` (KQuery.KType): K线类型，默认为日线

**返回值:** 因子对象，如果不存在则返回空因子

#### save_factor(factor) ⚠️ VIP功能

保存因子到数据库

```python
# 创建并保存因子 ⚠️ VIP功能
ma5 = MA(CLOSE(), 5)
ma5.name = "MA5"
ma5_factor = Factor("MA5", ma5)
save_factor(ma5_factor)
```

**参数说明:**

- `factor` (Factor): 要保存的因子对象

**注意:** 以 `name + ktype` 作为唯一标识

#### remove_factor(name, ktype) ⚠️ VIP功能

从数据库中删除因子

```python
# 删除指定因子 ⚠️ VIP功能
remove_factor("MA5", KQuery.DAY)
```

**参数说明:**

- `name` (str): 因子名称
- `ktype` (KQuery.KType): K线类型

**注意:** 以 `name + ktype` 作为唯一标识

#### get_all_factors() ⚠️ VIP功能

获取所有因子元数据

```python
# 获取所有因子 ⚠️ VIP功能
all_factors = get_all_factors()
print(f"共有 {len(all_factors)} 个因子")

# 遍历所有因子
for factor in all_factors:
    print(f"- {factor.name} ({factor.ktype})")
```

**返回值:** 所有因子对象列表

#### update_all_factors_values([ktype=KQuery.DAY]) ⚠️ VIP功能

更新所有因子值（增量更新）

```python
# 更新所有日线因子值 ⚠️ VIP功能
update_all_factors_values()

# 更新所有周线因子值 ⚠️ VIP功能
update_all_factors_values(KQuery.WEEK)
```

**参数说明:**

- `ktype` (KQuery.KType): K线类型，默认为日线

**使用场景:** 每日行情数据下载完成后，可以调用此函数更新所有存储的因子值。该操作为增量更新，只计算新增数据部分，提高更新效率。

### 因子集数据库操作 ⚠️ VIP功能

#### save_factorset(set) ⚠️ VIP功能

保存因子集到数据库

```python
# 创建并保存因子集 ⚠️ VIP功能
indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
factor_set = FactorSet(indicators)
factor_set.name = "均线因子集"
save_factorset(factor_set)
```

**参数说明:**

- `set` (FactorSet): 要保存的因子集对象

**注意:** 以 `name + ktype` 作为唯一标识

#### get_factorset(name[, ktype=KQuery.DAY]) ⚠️ VIP功能

获取因子集

```python
# 获取因子集 ⚠️ VIP功能
factor_set = get_factorset("均线因子集")
# 获取周线因子集 ⚠️ VIP功能
weekly_set = get_factorset("均线因子集", KQuery.WEEK)
```

**参数说明:**

- `name` (str): 因子集名称
- `ktype` (KQuery.KType): K线类型，默认为日线

**返回值:** 因子集对象，如果不存在则返回空因子集

#### remove_factorset(name, ktype) ⚠️ VIP功能

从数据库中删除因子集

```python
# 删除指定因子集 ⚠️ VIP功能
remove_factorset("均线因子集", KQuery.DAY)
```

**参数说明:**

- `name` (str): 因子集名称
- `ktype` (KQuery.KType): K线类型

**注意:** 以 `name + ktype` 作为唯一标识

#### get_all_factorsets() ⚠️ VIP功能

获取所有因子集

```python
# 获取所有因子集 ⚠️ VIP功能
all_sets = get_all_factorsets()
print(f"共有 {len(all_sets)} 个因子集")

# 遍历所有因子集
for factor_set in all_sets:
    print(f"- {factor_set.name} ({factor_set.ktype}): {len(factor_set)}个因子")
```

**返回值:** 所有因子集对象列表

### 使用示例

```
from hikyuu import *

# 1. 创建并管理因子
ma5 = MA(CLOSE(), 5)
ma5.name = "MA5"
ma5_factor = Factor("MA5", ma5, brief="5日均线")

rsi = RSI(CLOSE(), 14)
rsi.name = "RSI"
rsi_factor = Factor("RSI", rsi, brief="14日相对强弱指数")

# 保存到数据库
save_factor(ma5_factor)
save_factor(rsi_factor)

# 2. 创建因子集
tech_indicators = [ma5, rsi]
factor_set = FactorSet(tech_indicators)
factor_set.name = "技术指标集"
save_factorset(factor_set)

# 3. 查询和使用
# 获取所有因子
all_factors = get_all_factors()
print(f"数据库中共有 {len(all_factors)} 个因子")

# 获取特定因子集
loaded_set = get_factorset("技术指标集")
if not loaded_set.is_null():
    print(f"加载因子集: {loaded_set.name}, 包含 {len(loaded_set)} 个因子")
  
    # 使用因子集进行计算
    stocks = blocka.get_stock_list()[:5]
    query = Query(Datetime(20240101), Datetime(20241231))
    results = loaded_set.get_values(stocks, query)
    print("计算完成")

# 4. 清理不需要的因子
# remove_factor("MA5", KQuery.DAY)
# remove_factorset("技术指标集", KQuery.DAY)
```
