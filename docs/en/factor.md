# Factor Management

hikyuu provides a complete factor management system, including the management of the single factor Factor and the factor set FactorSet. A factor is the basic building block of the quantitative analysis, usually composed of the technical indicators or the other calculation formulas.

## Best Practices

1. **Naming convention**: the factor names should be descriptive and are not case-sensitive
2. **Factor set organization**: organize the related factors into the same FactorSet for easy management
3. **Data validation**: use the `check=True` parameter to verify whether the stock list belongs to the specified block
4. **Factor update**: after the daily market data download is completed, `update_all_factors_values()` should be called in time to update all the stored factor values, ensuring that the factor data is synchronized with the market data. This method is not integrated into HikyuuTdx and importdata, and needs to be called manually by yourself, because sometimes a data check is needed, and the factor values are saved only after confirming that the data is correct.
5. **Saving the factor values**: for the high-frequency aggregated factor values or the high-frequency factor values, it is recommended to set `need_save_value=True` and save them to the database. Because of Hikyuu's ultra-high calculation speed, the ordinary daily-frequency factor values (such as MA5) are usually not recommended to be saved to the database, because reading the factor values from the storage is slower, **which is different from the habit of the other quantitative frameworks relying on the factor storage to improve the speed**. It is recommended to test and decide by yourself according to your needs. Usually the original factor values are saved directly, without the cross-section, the standardization, etc.; these can be done by the MF. For the factors needing the cross-section values, the corresponding security set usually needs to be specified, which can be specified directly by Factor and FactorSet.

6. **Saving the special factor values**: for the special factor values not calculated through the indicators (such as PRICELIST or Indicator()), you can use the `save_special_values_to_db()` method to save the pre-calculated factor values directly
7. **Using the donating user features**: ⚠️ the database storage and reading operations related to the factors are all the donating user features, and the database engine only supports ClickHouse. Before using them, please confirm that you have obtained the corresponding permissions. Including but not limited to: `save_to_db()`, `remove_from_db()`, `save_values()`, `get_all_values()`, `get_values()` and the other database-related operation methods.

## Notes

- The combination of the factor name and the K-line type is the unique identifier
- When adding a factor with the same name to the FactorSet, the original factor will be overwritten
- The factors in the factor set keep the adding order
- The date alignment of the factor calculation results needs the reference security to synchronize
- ⚠️ All the database-related operation methods are the donating user features, supporting only the ClickHouse database engine
- **Block change handling**: when the block (the security set) of a Factor or a FactorSet changes, the system will not automatically update the saved factor values. If the stock set changes, you need to delete the original factor/factor set and recreate it yourself, or directly create a new factor/factor set. It is recommended to explicitly specify a fixed security set at the creation, avoiding the later changes.

## The Factor Class

The Factor class is used to represent a single factor, containing the basic information of the factor, the calculation formula and the related attributes.

### The Factor Constructor

```
# The default constructor
Factor()

# Only specify the name and the K-line type (used to load the existing factor from the database)
Factor(name, ktype=KQuery.DAY)

# Create a new factor object
Factor(name, formula, ktype=KQuery.DAY, brief="", details="", need_save_value=False, start_date=Datetime.min(), block=Block(), recover_type=KQuery.NO_RECOVER)
```

**Parameter descriptions:**

- `name` (str): the factor name
- `formula` (Indicator): the calculation formula indicator, which cannot be changed once created
- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line
- `brief` (str): the brief description, defaulting to empty
- `details` (str): the detailed description, defaulting to empty
- `need_save_value` (bool): whether the factor value data needs to be persisted, defaulting to False. When it is set to True, the calculated values of the specified stock set of the factor starting from start_date are saved to the database. Since Hikyuu's calculation speed is much faster than the database storage, the values of each security calculated by the ordinary daily-frequency factors are usually not recommended to be saved to the database; what usually needs to be saved to the database is the factor values aggregated from the high frequency to the daily frequency or the high-frequency factor values

- `start_date` (Datetime): the start date, the starting date when storing the data, defaulting to the minimum date
- `block` (Block): the block information, the security set; if it is empty, it is all, defaulting to empty
- `recover_type` (KQuery.RecoverType): the adjustment type, defaulting to NO_RECOVER

**Note:** the factor name is not case-sensitive, with `name + ktype` as the unique identifier

### The Factor Attributes

| Attribute                | Type         | Description                     |
| ------------------- | ------------ | ------------------------ |
| `name`            | str          | the factor name                 |
| `ktype`           | KQuery.KType | the factor frequency type             |
| `create_at`       | Datetime     | the creation date                 |
| `update_at`       | Datetime     | the modification date                 |
| `formula`         | Indicator    | the factor calculation formula             |
| `start_date`      | Datetime     | the start date of the data storage         |
| `block`           | Block        | the security set                 |
| `brief`           | str          | the basic description                 |
| `details`         | str          | the detailed description                 |
| `need_save_value` | bool         | whether to persist the factor value data |

### The Methods

#### The Common Methods

```
# Check whether the factor is empty
is_null = factor.is_null()
```

**Return value:**

- `bool`: if the factor is empty, return True; otherwise, return False

#### The Database Operations ⚠️ Donating user features

<div class="admonition note">
<p class="admonition-title">Note</p>
<p>The following database operation methods are all the donating user features, supporting only the ClickHouse database engine.</p>
</div>

```
# Save the factor metadata to the database ⚠️ Donating user feature
factor.save_to_db()

# Delete the factor and its data from the database ⚠️ Donating user feature
factor.remove_from_db()

# Save the special factor values to the database ⚠️ Donating user feature
factor.save_special_values_to_db(stock, dates, values, replace=False)
```

#### Getting the Data ⚠️ Donating user features

<div class="admonition note">
<p class="admonition-title">Note</p>
<p>For the following data getting methods, if the factor values themselves are stored in the database, the donating user permission is required, supporting only the ClickHouse database engine.</p>
</div>

```
# Get the calculation result of a single security ⚠️ Donating user feature
result = factor.get_value(stock, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())

# Get all the calculation results ⚠️ Donating user feature
results = factor.get_all_values(query, align=False, fill_null=False, tovalue=False, align_dates=DatetimeList())

# Get the calculation results of the specified security list ⚠️ Donating user feature
results = factor.get_values(stocks, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())

# Save the calculation results to the database ⚠️ Donating user feature
factor.save_values(stocks, query, replace=False)
```

**Parameter descriptions:**

- `stock` (Stock): the security object (only the get_value method)
- `query` (Query): the query parameters
- `align` (bool): whether to align the dates, defaulting to False
- `fill_null` (bool): whether to fill the empty values, defaulting to False
- `tovalue` (bool): whether to convert to the values, defaulting to False
- `check` (bool): whether to check that the stocks belong to the block specified by itself, defaulting to False (only the get_value and the get_values methods)
- `align_dates` (DatetimeList): the aligned date list, defaulting to empty
- `stocks` (sequence): the security list
- `replace` (bool): whether to replace the existing data, defaulting to False

**Return values:**

- `get_value`: return a single Indicator object, representing the calculation result of the specified security
- `get_all_values`: return a list of the Indicators, containing the calculation results of all the securities
- `get_values`: return a list of the Indicators, the calculation results arranged by the stock order

#### The Special Factor Value Saving Method ⚠️ Donating user features

For some special factors, whose values are not calculated through the indicator formulas but are pre-prepared data, the special factor value saving method can be used. This method supports two input formats:

##### Overloaded version 1: save the Indicator object

```
# Save the result data of the Indicator object ⚠️ Donating user feature
factor.save_special_values_to_db(stock, indicator, replace=False)
```

**Parameter descriptions:**

- `stock` (Stock): the security object
- `indicator` (Indicator): the already calculated indicator object (it must have been bound to the K-line data)
- `replace` (bool): whether to replace the existing data, defaulting to False

##### Overloaded version 2: save the pre-calculated data

```
# Save the pre-calculated date-value pair data ⚠️ Donating user feature
factor.save_special_values_to_db(stock, dates, values, replace=False)
```

**Parameter descriptions:**

- `stock` (Stock): the security object
- `dates` (DatetimeList): the date list corresponding to the factor values
- `values` (PriceList): the factor value list
- `replace` (bool): whether to replace the existing data, defaulting to False

**Usage scenarios:**

- Save the composite indicator calculation results
- Save the externally imported finance data
- Save the machine learning model prediction results
- Save the manually annotated special factor values
- Save the factor data obtained from the other data sources

**Notes:**

- When using the Indicator object, the object must have been bound to the specific K-line data (i.e. having a valid context)
- When using the date-value pairs, the date list and the value list must have the equal length
- The Indicator object can extract the needed data through the `.getDateList()` and the `.getResult(0)` methods

### The Factor Usage Example

```
from hikyuu import *

# Create the factor object
special_factor = Factor("SPECIAL_FACTOR", PRICELIST(), KQuery.DAY, "特殊因子", "预计算因子值")
special_factor.save_to_db()

stock = sm['sh600000']

# Way 1: save the Indicator object
k_data = stock.getKData(Query(Datetime(20240101), Datetime(20240110)))
complex_indicator = MA(CLOSE(), 10)(k_data) + RSI(CLOSE(), 14)(k_data) * 0.1
special_factor.save_special_values_to_db(stock, complex_indicator)

# Way 2: save the pre-calculated date-value pairs
dates = DatetimeList([Datetime(20240101), Datetime(20240102), Datetime(20240103)])
values = PriceList([1.2, 1.5, 1.3])
special_factor.save_special_values_to_db(stock, dates, values)

# An example of extracting the data from the Indicator to save
extracted_dates = complex_indicator.getDateList()
extracted_values = complex_indicator.getResult(0)
special_factor.save_special_values_to_db(stock, extracted_dates, extracted_values)

# Verify the saving result
query_test = Query(Datetime(20240101), Datetime(20240110))
saved_values = special_factor.get_values([stock], query_test)
print(f"保存的因子值: {saved_values[0].getResult(0)}")
```

## The FactorSet Class

The `FactorSet` class is used to manage a group of the related factors, providing the batch operations and the set management functions.

### The FactorSet Constructor

```
# The default constructor
FactorSet()

# Create the factor set with the specified name and type
FactorSet(name, ktype=KQuery.DAY, block=Block())

# Create the factor set with an indicator list
FactorSet(inds, ktype=KQuery.DAY)

# Create the factor set with a factor list
FactorSet(factors[, ktype=KQuery.DAY[, block=Block(), name='']])
```

**Parameter descriptions:**

- `name` (str): the factor set name
- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line
- `block` (Block): the block information, the security set, defaulting to empty
- `inds` (list): the indicator list
- `factors` (list): the factor list

**Note:** when creating with an indicator list or a factor list, the indicators/factors with the same name will be overwritten, and finally the last indicator/factor with the same name is kept

### The FactorSet Attributes

| Attribute      | Type         | Description             |
| --------- | ------------ | ---------------- |
| `name`  | str          | the factor set name       |
| `ktype` | KQuery.KType | the factor set frequency type   |
| `block` | Block        | the block corresponding to the factor set |

### The Basic Methods

```
# Check whether it is empty
is_empty = factor_set.empty()

# Clear the factor set
factor_set.clear()

# Check whether it is a null value
is_null = factor_set.is_null()

# Get the number of the factors
count = len(factor_set)
```

### The Factor Management Methods

```
# Add the factors
factor_set.add(factor)           # add a single Factor object
factor_set.add(indicator)        # add an Indicator object
factor_set.add(indicator_list)   # add an Indicator list
factor_set.add(factor_list)      # add a Factor list

# Check whether the specified factor exists
exists = factor_set.has_factor("MA5")

# Get the specified factor
factor = factor_set.get_factor("MA5")

# Remove the factor
factor_set.remove_factor("MA5")

# Get all the factors
factors = factor_set.get_factors()
```

### The Data Getting Methods

```
# Get the calculation results of all the factors
all_results = factor_set.get_all_values(query, align=False, fill_null=False, tovalue=False, align_dates=DatetimeList())

# Get the calculation results of the specified security list
results = factor_set.get_values(stocks, query, align=False, fill_null=False, tovalue=False, check=False, align_dates=DatetimeList())
```

**Parameter descriptions:**

- `query` (Query): the query parameters
- `align` (bool): whether to align the dates, defaulting to False
- `fill_null` (bool): whether to fill the empty values, defaulting to False
- `tovalue` (bool): whether to convert to the values, defaulting to False
- `check` (bool): whether to check that the stock list belongs to the block specified by itself, defaulting to False
- `align_dates` (DatetimeList): the aligned date list, defaulting to empty
- `stocks` (sequence): the security list

### The Iterator Support

```
# Traverse all the factors
for factor in factor_set:
    print(f"因子名称: {factor.name}")

# Access by the index
first_factor = factor_set[0]
named_factor = factor_set["MA5"]
```

### The FactorSet Usage Example

```
from hikyuu import *

# Create the technical indicator factors
ma5 = MA(CLOSE(), 5)
ma10 = MA(CLOSE(), 10)
rsi = RSI(CLOSE(), 14)

# Set the factor names
ma5.name = "MA5"
ma10.name = "MA10" 
rsi.name = "RSI"

# Create the factor set
factor_set = FactorSet("技术指标因子集", KQuery.DAY)

# Add the factors
factor_set.add(ma5)
factor_set.add(ma10)
factor_set.add(rsi)

# Or create it directly from the indicator list
indicators = [ma5, ma10, rsi]
factor_set2 = FactorSet(indicators, KQuery.DAY)

print(f"因子集包含 {len(factor_set)} 个因子")

# Traverse the factors
for factor in factor_set:
    print(f"- {factor.name}: {factor.brief}")

# Get the calculation results
stocks = blocka.get_stock_list()[:10]  # get the first 10 A-shares
query = Query(Datetime(20240101), Datetime(20241231))
results = factor_set.get_values(stocks, query)

# Check the specific factor
if factor_set.has_factor("MA5"):
    ma5_factor = factor_set.get_factor("MA5")
    print(f"找到MA5因子: {ma5_factor.details}")
```

## Combining with the Multi-factor Composition

The factors and the factor sets can be used together with the multi-factor composition algorithms:

```
from hikyuu import *

# Way 1: use an Indicator list
indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
weights = [0.6, 0.4]
mf1 = MF_Weight(indicators, stocks, weights, query)

# Way 2: use a FactorSet
factor_set = FactorSet(indicators)
mf2 = MF_Weight(factor_set, stocks, weights, query)

# Compose with the equal weights
mf_equal = MF_EqualWeight(factor_set, stocks, query)
```

## The Global Factor Management Functions ⚠️ Donating user features

<div class="admonition warning">
<p class="admonition-title">⚠️ Important reminder</p>
<p>All the following global factor management functions are the donating user features, and the database engine only supports ClickHouse.</p>
</div>

Besides the methods of the Factor and the FactorSet classes, hikyuu also provides a series of the global functions for the database management and the batch operations of the factors.

### The Factor Database Operations ⚠️ Donating user features

#### has_factor(name[, ktype=KQuery.DAY]) ⚠️ Donating user feature

Check whether the factor with the specified name and type exists in the database

```python
from hikyuu import *

# Check whether the daily-line factor exists ⚠️ Donating user feature
exists = has_factor("MA5")
# Check whether the weekly-line factor exists ⚠️ Donating user feature
weekly_exists = has_factor("MA5", KQuery.WEEK)
```

**Parameter descriptions:**

- `name` (str): the factor name
- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line

**Return value:** `bool` - if the factor exists, return True; otherwise, return False

#### get_factor(name[, ktype=KQuery.DAY]) ⚠️ Donating user feature

Get the factor metadata with the specified name and type

```python
from hikyuu import *

# Get the daily-line factor ⚠️ Donating user feature
factor = get_factor("MA5")
# Get the weekly-line factor ⚠️ Donating user feature
weekly_factor = get_factor("MA5", KQuery.WEEK)
```

**Parameter descriptions:**

- `name` (str): the factor name
- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line

**Return value:** the factor object; if it does not exist, return an empty factor

#### save_factor(factor[, update_before=True]) ⚠️ Donating user feature

Save the factor to the database

```python
# Create and save the factor (by default it will check and update the existing factor) ⚠️ Donating user feature
ma5 = MA(CLOSE(), 5)
ma5.name = "MA5"
ma5_factor = Factor("MA5", ma5)
save_factor(ma5_factor)
```

**Parameter descriptions:**

- `factor` (Factor): the factor object to save
- `update_before` (bool): whether to check and update the existing factor before saving, defaulting to True. Note: it usually must be true, otherwise it will cause the data errors, unless it is certain that all the factor values have been updated

**Note:** with `name + ktype` as the unique identifier

#### remove_factor(name, ktype) ⚠️ Donating user feature

Delete the factor from the database

```python
# Delete the specified factor ⚠️ Donating user feature
remove_factor("MA5", KQuery.DAY)
```

**Parameter descriptions:**

- `name` (str): the factor name
- `ktype` (KQuery.KType): the K-line type

**Note:** with `name + ktype` as the unique identifier

#### get_all_factors() ⚠️ Donating user feature

Get all the factor metadata

```python
# Get all the factors ⚠️ Donating user feature
all_factors = get_all_factors()
print(f"共有 {len(all_factors)} 个因子")

# Traverse all the factors
for factor in all_factors:
    print(f"- {factor.name} ({factor.ktype})")
```

**Return value:** the list of all the factor objects

#### update_all_factors_values([ktype=KQuery.DAY]) ⚠️ Donating user feature

Update all the factor values (an incremental update)

```python
# Update all the daily-line factor values ⚠️ Donating user feature
update_all_factors_values()

# Update all the weekly-line factor values ⚠️ Donating user feature
update_all_factors_values(KQuery.WEEK)
```

**Parameter descriptions:**

- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line

**Usage scenario:** after the daily market data download is completed, this function can be called to update all the stored factor values. This operation is an incremental update, calculating only the newly added data part, improving the update efficiency.

#### is_valid_factor_name(name)

Validate whether the factor name is legal

```python
from hikyuu import *

# Validate whether the factor name is legal
is_valid = is_valid_factor_name("MA5")
print(f"MA5 是否为合法因子名称: {is_valid}")
```

**Parameter descriptions:**

- `name` (str): the factor name

**Return value:** `bool` - if the name is legal, return True; otherwise, return False

**Note:** this function is mainly used for the ClickHouse database driver, and it always returns True under the non-ClickHouse drivers

### The Factor Set Database Operations ⚠️ Donating user features

#### save_factorset(set) ⚠️ Donating user feature

Save the factor set to the database

```python
# Create and save the factor set ⚠️ Donating user feature
indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
factor_set = FactorSet(indicators)
factor_set.name = "均线因子集"
save_factorset(factor_set)
```

**Parameter descriptions:**

- `set` (FactorSet): the factor set object to save

**Note:** with `name + ktype` as the unique identifier

#### get_factorset(name[, ktype=KQuery.DAY]) ⚠️ Donating user feature

Get the factor set

```python
# Get the factor set ⚠️ Donating user feature
factor_set = get_factorset("均线因子集")
# Get the weekly-line factor set ⚠️ Donating user feature
weekly_set = get_factorset("均线因子集", KQuery.WEEK)
```

**Parameter descriptions:**

- `name` (str): the factor set name
- `ktype` (KQuery.KType): the K-line type, defaulting to the daily line

**Return value:** the factor set object; if it does not exist, return an empty factor set

#### remove_factorset(name, ktype) ⚠️ Donating user feature

Delete the factor set from the database

```python
# Delete the specified factor set ⚠️ Donating user feature
remove_factorset("均线因子集", KQuery.DAY)
```

**Parameter descriptions:**

- `name` (str): the factor set name
- `ktype` (KQuery.KType): the K-line type

**Note:** with `name + ktype` as the unique identifier

#### get_all_factorsets() ⚠️ Donating user feature

Get all the factor sets

```python
# Get all the factor sets ⚠️ Donating user feature
all_sets = get_all_factorsets()
print(f"共有 {len(all_sets)} 个因子集")

# Traverse all the factor sets
for factor_set in all_sets:
    print(f"- {factor_set.name} ({factor_set.ktype}): {len(factor_set)}个因子")
```

**Return value:** the list of all the factor set objects

### The Global Factor Management Usage Example

```
from hikyuu import *

# 1. Create and manage the factors
ma5 = MA(CLOSE(), 5)
ma5.name = "MA5"
ma5_factor = Factor("MA5", ma5, brief="5日均线")

rsi = RSI(CLOSE(), 14)
rsi.name = "RSI"
rsi_factor = Factor("RSI", rsi, brief="14日相对强弱指数")

# Save to the database
# It can also be not saved; the advantage of saving to the database: later you can directly: ma5 = Factor('MA5), which will automatically load and get the factor definition from the database
# The factor created by default has need_save_value=False; if you also need to save the factor values, you need to set need_save_value=True; when saving, all the factor values will be calculated and stored automatically
save_factor(ma5_factor)
save_factor(rsi_factor)

# 2. Create the factor set
tech_indicators = [ma5, rsi]
factor_set = FactorSet(tech_indicators)
factor_set.name = "技术指标集"
save_factorset(factor_set)

# 3. Query and use
# Get all the factors
all_factors = get_all_factors()
print(f"数据库中共有 {len(all_factors)} 个因子")

# Get the specific factor set
loaded_set = get_factorset("技术指标集")
if not loaded_set.is_null():
    print(f"加载因子集: {loaded_set.name}, 包含 {len(loaded_set)} 个因子")
  
    # Use the factor set to calculate
    stocks = blocka.get_stock_list()[:5]
    query = Query(Datetime(20240101), Datetime(20241231))
    results = loaded_set.get_values(stocks, query)
    print("计算完成")

# 4. Clean up the unneeded factors
# remove_factor("MA5", KQuery.DAY)
# remove_factorset("技术指标集", KQuery.DAY)
```

## FAQ

### Q: How is the precision of the factor values guaranteed?

**A:** The system uses float64 to store, supporting the double-precision floating-point numbers, which can precisely represent the numerical range needed by most of the financial calculations.

### Q: Are there any special considerations for the high-frequency factor values?

**A:** The suggestions for handling the high-frequency factor values:
- Consider whether it is really necessary to save them to the database (Hikyuu's calculation speed is very fast)
- Test and decide the storage strategy according to the actual needs

### Q: When I have modified the security set (block) of the factor, how are the previously saved factor values updated?

**A:** When the block (the security set) of a Factor or a FactorSet changes, the system will not automatically update the saved factor values. If you have modified the stock set, you have two ways to handle it:

1. **Delete and recreate**: delete the original factor/factor set, and then recreate and save it
   ```python
   # Delete the old factor
   remove_factor("MY_FACTOR", KQuery.DAY)
   
   # Create and save the new factor
   new_factor = Factor("MY_FACTOR", formula, block=new_block)
   save_factor(new_factor)
   ```

2. **Create a new factor**: create a new factor/factor set with a different name
   ```python
   # Create the new factor with a version identifier
   updated_factor = Factor("MY_FACTOR_V2", formula, block=new_block)
   save_factor(updated_factor)
   ```

We recommend explicitly specifying a fixed security set when creating the factor, avoiding the later changes as much as possible, which can reduce the unnecessary maintenance work.
