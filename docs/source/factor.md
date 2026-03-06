# 因子管理

hikyuu提供了完善的因子管理体系，包括单个因子 Factor 和因子集合 FactorSet 的管理。因子是量化分析的基础构建块，通常由技术指标或其他计算公式构成。

## 最佳实践

1. **命名规范**: 因子名称应具有描述性且不区分大小写
2. **因子集组织**: 将相关的因子组织到同一个FactorSet中便于管理
3. **数据验证**: 使用 `check=True` 参数验证股票列表是否属于指定板块
4. **因子更新**: 每日行情数据下载完成后，应及时调用 `update_all_factors_values()` 更新所有存储的因子值，确保因子数据与行情数据同步。该方法未集成到 HikyuuTdx 和 importdata 中，需要自行手工调用，原因是有时需要进行数据检查，确认数据无误后再进行因子值保存。
5. **因子值保存**: 对于高频聚合的因子值或高频因子值，建议设置 `need_save_value=True` 并保存到数据库。由于 Hikyuu的超高计算速度，普通的日频因子值(如MA5)，通常不建议保存到数据库，因为从存储中读取因子值速度更更慢，**这和其他的量化框架依赖因子存储来提升速度的习惯有所不同**。建议自行根据需要测试决定。通常直接保存原始因子值，不需要进行截面、标准化等处理，可以由 MF 完成。对需要截面值的因子，通常需要指定对应的证券集，Factor和FactorSet直接指定。

<div class="admonition warning">
<p class="admonition-title">⚠️ 重要提醒：因子值精度与范围限制</p>
<p>因子值在数据库中使用 int32 类型存储，保留4位小数精度。具体存储方式为：实际值 × 10000 后转换为整数存储。</p>
<p><strong>数值范围限制：</strong></p>
<ul>
<li>int32 取值范围：-2,147,483,648 到 2,147,483,647</li>
<li>对应因子值范围：-214,748.3648 到 214,748.3647</li>
</ul>
<p><strong>特别注意事项：</strong></p>
<ul>
<li><strong>市值类因子</strong>：如总市值、流通市值等通常数值较大，容易超出存储范围</li>
<li><strong>解决方案</strong>：对于大数值因子，建议存储单位调整后的值（如亿元、万股等）</li>
<li><strong>示例</strong>：总市值存储为 亿元单位，流通股本存储为 万股单位</li>
</ul>
<p>超出范围的因子值将导致存储失败或数据截断，请务必在设计因子时考虑数值范围问题。</p>
</div>

6. **特殊因子值保存**: 对于不通过指标计算的特殊因子值（如PRICELIST或Indicator()），可以使用 `save_special_values_to_db()` 方法直接保存预计算的因子值
7. **捐赠用户功能使用**: ⚠️ 因子相关的数据库存储和读取操作均为捐赠用户功能，数据库引擎仅支持ClickHouse。使用前请确认已获得相应权限。包括但不限于：`save_to_db()`、`remove_from_db()`、`save_values()`、`get_all_values()`、`get_values()` 等涉及数据库的操作方法。

## 注意事项

- 因子名称和K线类型的组合是唯一的标识符
- FactorSet 添加同名因子时会覆盖原有的因子
- 因子集中的因子保持添加顺序
- 因子计算结果的日期对齐需要参考证券进行同步
- ⚠️ 所有涉及数据库的操作方法均为捐赠用户功能，仅支持ClickHouse数据库引擎

## Factor类

Factor 类用于表示单个因子，包含了因子的基本信息、计算公式和相关属性。

### Factor构造函数

```
# 默认构造函数
Factor()

# 只指定名称和K线类型（用于从数据库加载已有因子）
Factor(name, ktype=KQuery.DAY)

# 创建新的因子对象
Factor(name, formula, ktype=KQuery.DAY, brief="", details="", need_save_value=False, start_date=Datetime.min(), block=Block())
```

**参数说明:**

- `name` (str): 因子名称
- `formula` (Indicator): 计算公式指标，一旦创建不可更改
- `ktype` (KQuery.KType): K线类型，默认为日线
- `brief` (str): 简要描述，默认为空
- `details` (str): 详细描述，默认为空
- `need_save_value` (bool): 是否需要持久化保存因子值数据，默认为False。设置为True时，将因子指定股票集合从start_date开始的计算值保存到数据库中。由于Hikyuu的计算速度远快于数据库存储，通常日频因子计算的各证券值不建议保存到数据库，需要保存到数据库的通常为高频聚合到日频的因子值或高频因子值

<div class="admonition warning">
<p class="admonition-title">⚠️ 数值范围警告</p>
<p>因子值存储使用int32类型，保留4位小数精度。有效数值范围为 -214,748.3648 到 214,748.3647。</p>
<p>对于市值类因子等大数值因子，请务必进行单位调整（如存储亿元而非元）以避免超出存储范围。</p>
</div>

- `start_date` (Datetime): 开始日期，数据存储时的起始日期，默认为最小日期
- `block` (Block): 板块信息，证券集合，如果为空则为全部，默认为空

**注意:** 因子名称不区分大小写，以 `name + ktype` 作为唯一标识

### Factor属性

| 属性                | 类型         | 描述                     |
| ------------------- | ------------ | ------------------------ |
| `name`            | str          | 因子名称                 |
| `ktype`           | KQuery.KType | 因子频率类型             |
| `create_at`       | Datetime     | 创建日期                 |
| `update_at`       | Datetime     | 更改日期                 |
| `formula`         | Indicator    | 因子计算公式             |
| `start_date`      | Datetime     | 数据存储起始日期         |
| `block`           | Block        | 证券集合                 |
| `brief`           | str          | 基础说明                 |
| `details`         | str          | 详细说明                 |
| `need_save_value` | bool         | 是否持久化保存因子值数据 |

### 方法

#### 数据库操作 ⚠️ 捐赠用户功能

<div class="admonition note">
<p class="admonition-title">注意</p>
<p>以下数据库操作方法均为捐赠用户功能，仅支持ClickHouse数据库引擎。</p>
</div>

```
# 保存因子元数据到数据库 ⚠️ 捐赠用户功能
factor.save_to_db()

# 从数据库中删除因子及其数据 ⚠️ 捐赠用户功能
factor.remove_from_db()

# 特殊因子保存值到数据库 ⚠️ 捐赠用户功能
factor.save_special_values_to_db(stock, dates, values, replace=False)
```

#### 数据获取 ⚠️ 捐赠用户功能

<div class="admonition note">
<p class="admonition-title">注意</p>
<p>以下数据获取方法，如因子值本身存储在数据库中，则需 捐赠用户 权限，仅支持 ClickHouse 数据库引擎。</p>
</div>

```
# 获取单个证券的计算结果 ⚠️ 捐赠用户功能
result = factor.get_value(stock, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())

# 获取所有计算结果 ⚠️ 捐赠用户功能
results = factor.get_all_values(query, align=False, fill_null=False, tovalue=False, align_dates=DatetimeList())

# 获取指定证券列表的计算结果 ⚠️ 捐赠用户功能
results = factor.get_values(stocks, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())

# 保存计算结果到数据库 ⚠️ 捐赠用户功能
factor.save_values(stocks, query, replace=False)
```

**参数说明:**

- `stock` (Stock): 证券对象（仅 get_value 方法）
- `query` (Query): 查询参数
- `align` (bool): 是否对齐日期，默认 False
- `fill_null` (bool): 是否填充空值，默认 False
- `tovalue` (bool): 是否转换为数值，默认 False
- `check` (bool): 是否检查股票属于自身指定的 block，默认 False（仅 get_value 和 get_values 方法）
- `align_dates` (DatetimeList): 对齐日期列表，默认为空
- `stocks` (sequence): 证券列表
- `replace` (bool): 是否替换已有数据，默认 False

**返回值:**

- `get_value`: 返回单个 Indicator 对象，表示指定证券的计算结果
- `get_all_values`: 返回 Indicator 列表，包含所有证券的计算结果
- `get_values`: 返回 Indicator 列表，按股票顺序排列的计算结果

#### 特殊因子值保存方法 ⚠️ 捐赠用户功能

对于某些特殊因子，其值不是通过指标公式计算得出，而是预先准备好的数据，可以使用特殊因子值保存方法。该方法支持两种输入格式：

##### 重载版本1：保存Indicator对象

```
# 保存Indicator对象的结果数据 ⚠️ 捐赠用户功能
factor.save_special_values_to_db(stock, indicator, replace=False)
```

**参数说明:**

- `stock` (Stock): 证券对象
- `indicator` (Indicator): 已计算好的指标对象（必须已绑定K线数据）
- `replace` (bool): 是否替换已有数据，默认False

##### 重载版本2：保存预计算数据

```
# 保存预计算的日期-值对数据 ⚠️ 捐赠用户功能
factor.save_special_values_to_db(stock, dates, values, replace=False)
```

**参数说明:**

- `stock` (Stock): 证券对象
- `dates` (DatetimeList): 因子值对应的日期列表
- `values` (PriceList): 因子值列表
- `replace` (bool): 是否替换已有数据，默认False

<div class="admonition warning">
<p class="admonition-title">⚠️ 数值处理重要提醒</p>
<p><strong>因子值范围限制：</strong></p>
<ul>
<li>存储类型：int32（4字节整数）</li>
<li>精度：保留4位小数</li>
<li>存储方式：实际值 × 10000 转换为整数存储</li>
<li>有效范围：-214,748.3648 到 214,748.3647</li>
</ul>

<p><strong>大数值因子处理示例：</strong></p>

```python
# ❌ 错误示例：市值因子直接存储（容易超出范围）
market_value_yuan = PriceList([200000000000, 150000000000, 300000000000])  # 元为单位
factor.save_special_values_to_db(stock, dates, market_value_yuan)  # 可能超出范围

# ✅ 正确示例：调整单位后存储
market_value_yi_yuan = PriceList([2000, 1500, 3000])  # 亿元为单位
factor.save_special_values_to_db(stock, dates, market_value_yi_yuan)

# ❌ 错误示例：股本因子直接存储
total_shares = PriceList([10000000000, 8000000000, 12000000000])  # 股为单位
factor.save_special_values_to_db(stock, dates, total_shares)  # 可能超出范围

# ✅ 正确示例：调整单位后存储  
total_shares_wan_gu = PriceList([100000, 80000, 120000])  # 万股为单位
factor.save_special_values_to_db(stock, dates, total_shares_wan_gu)
```

<p><strong>常见需要单位调整的因子类型：</strong></p>
<ul>
<li>市值类：总市值、流通市值（建议存储单位：亿元）</li>
<li>股本类：总股本、流通股本（建议存储单位：万股）</li>
<li>成交额：日成交金额（建议存储单位：万元）</li>
<li>其他大数值指标：根据实际情况调整单位</li>
</ul>
</div>

**使用场景:**

- 保存复合指标计算结果
- 保存外部导入的财务数据
- 保存机器学习模型预测结果
- 保存人工标注的特殊因子值
- 保存从其他数据源获取的因子数据

**注意事项:**

- 当使用Indicator对象时，该对象必须已经绑定了具体的K线数据（即有有效的context）
- 当使用日期-值对时，日期列表和值列表长度必须相等
- Indicator对象可以通过 `.getDateList()`和 `.getResult(0)`方法提取所需数据

### Factor使用示例

```
from hikyuu import *

# 创建因子对象
special_factor = Factor("SPECIAL_FACTOR", PRICELIST(), KQuery.DAY, "特殊因子", "预计算因子值")
special_factor.save_to_db()

stock = sm['sh600000']

# 方式1：保存Indicator对象
k_data = stock.getKData(Query(Datetime(20240101), Datetime(20240110)))
complex_indicator = MA(CLOSE(), 10)(k_data) + RSI(CLOSE(), 14)(k_data) * 0.1
special_factor.save_special_values_to_db(stock, complex_indicator)

# 方式2：保存预计算的日期-值对
dates = DatetimeList([Datetime(20240101), Datetime(20240102), Datetime(20240103)])
values = PriceList([1.2, 1.5, 1.3])
special_factor.save_special_values_to_db(stock, dates, values)

# 从Indicator提取数据保存示例
extracted_dates = complex_indicator.getDateList()
extracted_values = complex_indicator.getResult(0)
special_factor.save_special_values_to_db(stock, extracted_dates, extracted_values)

# 验证保存结果
query_test = Query(Datetime(20240101), Datetime(20240110))
saved_values = special_factor.get_values([stock], query_test)
print(f"保存的因子值: {saved_values[0].getResult(0)}")
```

## FactorSet类

`FactorSet` 类用于管理一组相关的因子，提供批量操作和集合管理功能。

### FactorSet构造函数

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

### FactorSet属性

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

### FactorSet使用示例

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

## 全局因子管理函数 ⚠️ 捐赠用户功能

<div class="admonition warning">
<p class="admonition-title">⚠️ 重要提醒</p>
<p>以下所有全局因子管理函数均为捐赠用户功能，数据库引擎仅支持ClickHouse。</p>
</div>

除了Factor和FactorSet类的方法外，hikyuu还提供了一系列全局函数用于因子的数据库管理和批量操作。

### 因子数据库操作 ⚠️ 捐赠用户功能

#### get_factor(name[, ktype=KQuery.DAY]) ⚠️ 捐赠用户功能

获取指定名称和类型的因子元数据

```python
from hikyuu import *

# 获取日线因子 ⚠️ 捐赠用户功能
factor = get_factor("MA5")
# 获取周线因子 ⚠️ 捐赠用户功能
weekly_factor = get_factor("MA5", KQuery.WEEK)
```

**参数说明:**

- `name` (str): 因子名称
- `ktype` (KQuery.KType): K线类型，默认为日线

**返回值:** 因子对象，如果不存在则返回空因子

#### save_factor(factor) ⚠️ 捐赠用户功能

保存因子到数据库

```python
# 创建并保存因子 ⚠️ 捐赠用户功能
ma5 = MA(CLOSE(), 5)
ma5.name = "MA5"
ma5_factor = Factor("MA5", ma5)
save_factor(ma5_factor)
```

**参数说明:**

- `factor` (Factor): 要保存的因子对象

**注意:** 以 `name + ktype` 作为唯一标识

#### remove_factor(name, ktype) ⚠️ 捐赠用户功能

从数据库中删除因子

```python
# 删除指定因子 ⚠️ 捐赠用户功能
remove_factor("MA5", KQuery.DAY)
```

**参数说明:**

- `name` (str): 因子名称
- `ktype` (KQuery.KType): K线类型

**注意:** 以 `name + ktype` 作为唯一标识

#### get_all_factors() ⚠️ 捐赠用户功能

获取所有因子元数据

```python
# 获取所有因子 ⚠️ 捐赠用户功能
all_factors = get_all_factors()
print(f"共有 {len(all_factors)} 个因子")

# 遍历所有因子
for factor in all_factors:
    print(f"- {factor.name} ({factor.ktype})")
```

**返回值:** 所有因子对象列表

#### update_all_factors_values([ktype=KQuery.DAY]) ⚠️ 捐赠用户功能

更新所有因子值（增量更新）

```python
# 更新所有日线因子值 ⚠️ 捐赠用户功能
update_all_factors_values()

# 更新所有周线因子值 ⚠️ 捐赠用户功能
update_all_factors_values(KQuery.WEEK)
```

**参数说明:**

- `ktype` (KQuery.KType): K线类型，默认为日线

**使用场景:** 每日行情数据下载完成后，可以调用此函数更新所有存储的因子值。该操作为增量更新，只计算新增数据部分，提高更新效率。

### 因子集数据库操作 ⚠️ 捐赠用户功能

#### save_factorset(set) ⚠️ 捐赠用户功能

保存因子集到数据库

```python
# 创建并保存因子集 ⚠️ 捐赠用户功能
indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
factor_set = FactorSet(indicators)
factor_set.name = "均线因子集"
save_factorset(factor_set)
```

**参数说明:**

- `set` (FactorSet): 要保存的因子集对象

**注意:** 以 `name + ktype` 作为唯一标识

#### get_factorset(name[, ktype=KQuery.DAY]) ⚠️ 捐赠用户功能

获取因子集

```python
# 获取因子集 ⚠️ 捐赠用户功能
factor_set = get_factorset("均线因子集")
# 获取周线因子集 ⚠️ 捐赠用户功能
weekly_set = get_factorset("均线因子集", KQuery.WEEK)
```

**参数说明:**

- `name` (str): 因子集名称
- `ktype` (KQuery.KType): K线类型，默认为日线

**返回值:** 因子集对象，如果不存在则返回空因子集

#### remove_factorset(name, ktype) ⚠️ 捐赠用户功能

从数据库中删除因子集

```python
# 删除指定因子集 ⚠️ 捐赠用户功能
remove_factorset("均线因子集", KQuery.DAY)
```

**参数说明:**

- `name` (str): 因子集名称
- `ktype` (KQuery.KType): K线类型

**注意:** 以 `name + ktype` 作为唯一标识

#### get_all_factorsets() ⚠️ 捐赠用户功能

获取所有因子集

```python
# 获取所有因子集 ⚠️ 捐赠用户功能
all_sets = get_all_factorsets()
print(f"共有 {len(all_sets)} 个因子集")

# 遍历所有因子集
for factor_set in all_sets:
    print(f"- {factor_set.name} ({factor_set.ktype}): {len(factor_set)}个因子")
```

**返回值:** 所有因子集对象列表

### 全局因子管理使用示例

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
# 也可以不保存，保存到数据库的优势：后续可以直接: ma5 = Factor('MA5) , 会自动从数据库加载获取因子定义
# 默认创建的因子 need_save_value=False，如需同时保存因子值，需将 need_save_value=True，保存时将自动计算所有因子值并存储
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

## 常见问题 FAQ

### Q: 为什么我的因子值保存失败了？

**A:** 最常见的原因是因子值超出了int32的存储范围。请检查：
- 因子值是否在 -214,748.3648 到 214,748.3647 范围内
- 市值类因子是否进行了单位调整（建议存储亿元而非元）
- 大数值因子是否适当缩放

### Q: 如何处理市值类因子的存储问题？

**A:** 市值类因子通常数值很大，建议的处理方式：

```python
# 总市值因子示例
def create_market_value_factor():
    # ❌ 不推荐：直接使用元为单位
    # market_value_in_yuan = get_market_value_in_yuan()  # 数值可能达到千亿级别
    
    # ✅ 推荐：使用亿元为单位
    market_value_in_yi_yuan = get_market_value_in_yuan() / 100000000  # 转换为亿元
    mv_factor = Factor("MARKET_VALUE", PRICELIST(), brief="总市值(亿元)")
    mv_factor.save_special_values_to_db(stock, dates, market_value_in_yi_yuan)
    return mv_factor
```

### Q: 因子值的精度是如何保证的？

**A:** 系统使用int32存储，通过乘以10000的方式保留4位小数精度：
- 存储前：实际值 × 10000
- 存储后：转换为int32整数
- 读取时：int32值 ÷ 10000.0 转换回浮点数

这种方式既保证了精度，又节省了存储空间。

### Q: 如何验证因子值是否在有效范围内？

**A:** 可以使用以下代码进行验证：

```python
def validate_factor_values(values):
    """验证因子值是否在有效范围内"""
    min_val, max_val = -214748.3648, 214748.3647
    
    invalid_count = 0
    for val in values:
        if val < min_val or val > max_val:
            invalid_count += 1
            print(f"警告：发现超出范围的值 {val}")
    
    if invalid_count > 0:
        print(f"共发现 {invalid_count} 个超出范围的因子值")
        return False
    return True

# 使用示例
values = get_my_factor_values()
if not validate_factor_values(values):
    print("请调整因子值范围后再保存")
```

### Q: 对于高频因子值有什么特殊考虑吗？

**A:** 高频因子值通常波动较小，但仍需注意：
- 确保缩放后的值仍在有效范围内
- 考虑是否真的需要保存到数据库（Hikyuu计算速度很快）
- 如果保存，建议使用合适的单位以充分利用精度
