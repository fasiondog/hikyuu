.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

交易成本算法
============

内建交易成本算法
----------------

零交易成本算法
^^^^^^^^^^^^^^

.. py:function:: TC_Zero()

    :return: :py:class:`TradeCostBase` 子类实例


沪深A股交易成本算法
^^^^^^^^^^^^^^^^^^^

* TC_FixedA 为滚动默认成本模型，始终对应最新年份的费率；各年份模型（TC_FixedAPre2015 / 2015 / 2017）保持冻结，用于历史回测。
* 2022年4月29日起过户费降为成交金额的千分之0.01（沪深京双向收取）；2023年8月28日起卖出印花税降为千分之0.5。

.. py:function:: TC_FixedA([commission=0.0018, lowest_commission=5.0, stamptax=0.0005, transferfee=0.00001])

    当前最新A股交易成本算法（滚动默认，始终对应当前费率）：过户费按成交金额的千分之0.01全市场双向收取，卖出印花税千分之0.5

    :param float commission: 佣金比例
    :param float lowest_commission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例

.. py:function:: TC_FixedA2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    2015年8月1日及之后的A股交易成本算法，上证过户费改为成交金额的千分之0.02

    :param float commission: 佣金比例
    :param float lowest_commission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例

.. py:function:: TC_FixedA2017([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    2017年1月1日及之后的A股交易成本算法，深证过户费单列，沪深均按成交金额的千分之0.02双向收取

    :param float commission: 佣金比例
    :param float lowest_commission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :return: :py:class:`TradeCostBase` 子类实例

.. py:function:: TC_FixedAPre2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.001, lowest_transferfee=1.0])

    2015年8月1日之前的A股交易成本算法：上证过户费按交易数量千分之一（每1000股1元），不足1元按1元计；深证不收过户费

    :param float commission: 佣金比例
    :param float lowest_commission: 最低佣金值
    :param float stamptax: 印花税
    :param float transferfee: 过户费
    :param float lowest_transferfee: 最低过户费
    :return: :py:class:`TradeCostBase` 子类实例


融资融券利息成本算法
^^^^^^^^^^^^^^^^^^^^

买卖手续费与利息是两个口径：A股 / ETF / 通用模型只回答买卖费用，**融资融券的年化利率由专门的包装模型
回答**，因此可以把任意一个成本模型包进去而不改动它。

.. py:function:: TC_MarginInterest([trade_cost=TC_Zero(), borrow_cash_rate=0.0, borrow_stock_rates="", default_borrow_stock_rate=0.0])

    包装另一个交易成本算法，附加融资利息与融券费用的年化利率：买卖费用全部转给被包装的算法回答，本算法
    只多回答两个利率。利率由账户结算计提，按距上次计提经过的自然日数除以账户参数 ``annual_days`` 折算金额，计入负债本身
    而不从现金支付，产生 ``BUSINESS_INTEREST`` 记录，见 :doc:`margin`

    两个利率默认均为 0.0（不计提），因此只包装而不配置利率时，账簿与不包装时逐位一致。

    :param TradeCost trade_cost: 回答买卖费用的成本算法，默认 TC_Zero
    :param float borrow_cash_rate: 融资年化利率，[0, 1)，0.0 表示不计提利息
    :param str borrow_stock_rates: 融券年化费率表，``"key:rate"`` 列表，如 ``"12:0.10, SHF:0.12"``；纯数字键
        为证券类型，其他键为市场代码
    :param float default_borrow_stock_rate: 未被任何键命中标的的融券年化费率，[0, 1)，0.0 表示不计提
    :return: :py:class:`TradeCostBase` 子类实例

.. code-block:: python

    # 当前A股买卖费用 + 年化 6% 融资利率、按市场配置的融券费率
    cost = TC_MarginInterest(TC_FixedA(), borrow_cash_rate=0.06, borrow_stock_rates="SH:0.10, SZ:0.12")
    tm = crtTM(cost_func=cost, margin=MG_ByType("1:0.5"))


ETF交易成本算法
^^^^^^^^^^^^^^^

计算规则如下:: python

    买入：佣金（最低5元）
    卖出：佣金（最低5元）
    
    无印花税和过户费

.. py:function:: TC_FixedETF([commission=0.0001, lowest_commission=5.0])

    ETF交易成本算法，买卖双向收取佣金，无印花税和过户费

    :param float commission: 佣金比例，默认万分之1
    :param float lowest_commission: 最低佣金值，默认5元/笔
    :return: :py:class:`TradeCostBase` 子类实例


港股交易成本算法
^^^^^^^^^^^^^^^^

适用于港股。港股印花税、交易费与征费均为**买卖双边**收取（与 A 股印花税单边不同）：

* 买入 / 卖出：max(佣金比例 × 成交金额, 最低佣金) + 印花税比例 × 成交金额
  + 交易费及征费比例 × 成交金额 + 每笔固定费用

.. py:function:: TC_HK([commission=0.00025, lowest_commission=3.0, stamptax=0.001, tradefee=0.0000555, per_trade=0.0])

    港股交易成本算法，买卖双边对称收取

    :param float commission: 佣金比例（双边，按成交金额），默认万分之2.5
    :param float lowest_commission: 最低佣金值（双边，按笔），默认 3 港元
    :param float stamptax: 印花税率（双边收取），2023 年 11 月 17 日起为 0.1%
    :param float tradefee: 交易费及征费合计比例（双边）：联交所交易费 0.0027% +
        证监会征费 0.0027% + 财汇局征费 0.00015%，默认 0.0000555；计入
        CostRecord.transferfee
    :param float per_trade: 每笔固定费用（双边，如经纪商代收的结算费），默认 0；
        计入 CostRecord.others
    :return: :py:class:`TradeCostBase` 子类实例


通用固定费率交易成本算法
^^^^^^^^^^^^^^^^^^^^^^^^

适用于期货、外汇、数字货币等无专用成本模型的市场。计算规则：

* 买入：max(佣金比例 × 成交金额, 最低佣金) + per_lot × 数量 + 其他固定费用
* 卖出：同上，另加印花税率 × 成交金额

成交金额 = 价格 × 数量 × 合约乘数，合约乘数取自证券自身的 unit（股票为 1，期货为合约规模），
因此同一套公式同时适用于股票与期货；per_lot 与其他固定费用计入 CostRecord.others。

.. py:function:: TC_Fixed([commission=0.0, lowest_commission=0.0, stamptax=0.0, per_lot=0.0, others=0.0])

    通用固定费率交易成本算法，覆盖期货、外汇、数字货币等无专用成本模型的市场

    :param float commission: 佣金比例（按成交金额）
    :param float lowest_commission: 最低佣金值，按 ``max(佣金比例 × 成交金额, 最低佣金)``
        生效——即使佣金比例 × 成交金额为 0 也照收，如需免除请置 0
    :param float stamptax: 印花税率，仅卖出收取
    :param float per_lot: 按成交数量（股/张/币量）收取的固定费用，非按手——即
        ``per_lot × number``；如需按手收费请将费率除以每手数量
    :param float others: 其他固定费用，如交割费、资金费
    :return: :py:class:`TradeCostBase` 子类实例


自定义交易成本算法
------------------

自定义交易成本算法接口：

* :py:meth:`TradeCostBase.get_buy_cost` - 【必须】获取买入成本
* :py:meth:`TradeCostBase.get_sell_cost` - 【必须】获取卖出成本
* :py:meth:`TradeCostBase.get_borrow_cash_cost` - 【可选】获取借入资金成本（融资）
* :py:meth:`TradeCostBase.get_return_cash_cost` - 【可选】获取归还借入资金成本
* :py:meth:`TradeCostBase.get_borrow_stock_cost` - 【可选】获取借入股票成本（融券做空）
* :py:meth:`TradeCostBase.get_return_stock_cost` - 【可选】获取归还借入股票成本
* :py:meth:`TradeCostBase._clone` - 【必须】子类克隆接口

.. note::

    以上名称就是 Python 侧需要实现的覆写名，账户 C++ 代码按这些名字查找到子类实现后回调。旧版本绑定层是按 C++ 方法名
    （``getBuyCost`` 一类）查找的，与该清单不一致，按清单写的覆写会被静默忽略；现已统一到 snake_case 名称上，以 C++
    名定义覆写的旧代码需改名。



交易算法成本基类
----------------

.. py:class:: TradeCostBase(name)

    交易成本算法基类
    
    .. py:attribute:: name 名称
        
    .. py:method:: get_param(self, name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: set_param(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: clone(self)
    
        克隆操作

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        【重载接口】获取买入成本
        
        :param Datetime datetime: 买入时刻
        :param Stock stock: 买入对象
        :param float price: 买入价格
        :param int num: 买入数量
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        【重载接口】获取卖出成本
        
        :param Datetime datetime: 卖出时刻
        :param Stock stock: 卖出对象
        :param float price: 卖出价格
        :param int num: 卖出数量
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_borrow_cash_cost(self, datetime, cash)
    
        【重载接口】获取借入资金的成本
        
        :param Datetime datetime: 借入时刻
        :param float cash: 借入的现金额
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_return_cash_cost(self, borrow_datetime, return_datetime, cash)
    
        【重载接口】获取归还借入资金的成本
        
        :param Datetime borrow_datetime: 借入时刻
        :param Datetime return_datetime: 归还时刻
        :param float cash: 归还的现金额
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_borrow_stock_cost(self, datetime, stock, price, num)
    
        【重载接口】获取借入股票的成本
        
        :param Datetime datetime: 借入时刻
        :param Stock stock: 借入对象
        :param float price: 借入价格
        :param int num: 借入数量
        :return: 交易成本记录
        :rtype: CostRecord
    
    .. py:method:: get_return_stock_cost(self, borrow_datetime, return_datetime, stock, price, num)
    
        【重载接口】获取归还借入股票的成本
        
        :param Datetime borrow_datetime: 借入时刻
        :param Datetime return_datetime: 归还时刻
        :param Stock stock: 归还对象
        :param float price: 归还价格
        :param int num: 归还数量
        :return: 交易成本记录
        :rtype: CostRecord

    .. py:method:: get_borrow_cash_rate(self, datetime)

        【重载接口】融资借入资金的年化利率，默认 0.0（不计提）

        利率由账户结算计提，按距上次计提经过的自然日数除以账户参数 ``annual_days`` 折算金额，计入负债本身而
        不从现金支付，产生 ``BUSINESS_INTEREST`` 记录，见 :doc:`margin`；内建实现见
        :py:func:`TC_MarginInterest`
        
        :param Datetime datetime: 结算时刻
        :return: 年化小数利率，如 0.06 表示年化 6%
        :rtype: float

    .. py:method:: get_borrow_stock_rate(self, datetime, stock)

        【重载接口】融券借入标的的年化费率，默认 0.0（不计提）

        按结算日借入数量的市值计提，同样计入负债并以现金结算；内建实现见 :py:func:`TC_MarginInterest`

        :param Datetime datetime: 结算时刻
        :param Stock stock: 借入对象
        :return: 年化小数费率，如 0.10 表示年化 10%
        :rtype: float

    .. py:method:: _clone(self)
    
        【重载接口】子类克隆接口