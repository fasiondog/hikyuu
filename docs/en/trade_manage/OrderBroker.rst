.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Order Broker
============

Multiple order broker instances can be registered to the TradeManager through :py:meth:`TradeManager.regBroker`. These order brokers can perform the additional buy/sell actions; e.g. the mail order broker can send an email when the TradeManager issues the buy/sell instructions.

By default, when the TradeManager executes the buy/sell operations, it calls the order broker to execute the broker's buy/sell actions, but there will be a problem in the live trading operation. Because the system needs to backtrack the historical data to get the latest signal when calculating the signal generator, the TradeManager will execute the buy/sell operations at the historical moments; **at this time, if the order broker itself does not control the moment of issuing the buy/sell instructions, it will cause the broker to send the wrong instructions**. Therefore, it is necessary to specify that only after a certain moment are the buy/sell operations of the order broker allowed to be executed. The TradeManager attribute :py:attr:`TradeManager.brokeLastDatetime` is used to specify that moment.


The Order Broker Wrapper in Python
----------------------------------

Since implementing a custom order broker by inheriting from :py:class:`OrderBrokerBase` requires implementing the two interface methods _buy and _sell, and among the many Python packages, some packages have already implemented the live trading functions, e.g. the built-in `扯线木偶 (puppet) <https://github.com/Raytone-D/puppet>`_ from "睿瞳深邃" (thanks to "睿瞳深邃" for sharing). The trading classes in these packages have generally already implemented the buy and sell methods; if the order broker class is implemented by inheriting from OrderBrokerBase, the code looks verbose and is inconvenient to use. Therefore, in Python, the :py:class:`OrderBrokerWrap` class and the :py:func:`crtOB` function are implemented, which can quickly wrap a class with the buy and sell methods to generate an order broker. The code example is as follows::

    # Create a simulated trading account for the backtest, with an initial capital of 300,000
    my_tm = crtTM(init_cash = 300000)

    # Register the live trading order broker
    ob = crtOB(TestOrderBroker())
    my_tm.reg_broker(ob) # TestOrderBroker is a test order broker object, which only prints
    #my_tm.reg_broker(crtOB(MailOrderBroker("smtp.sina.com", "yourmail@sina.com", "yourpwd", "receivermail@XXX.yy)))

    # Modify the last timestamp of the order broker as needed; only when it is greater than this timestamp will the order broker actually issue the order instructions
    my_tm.broke_last_datetime=Datetime(201706010000)

    # Create the signal generator (with the 5-day EMA as the fast line and the 10-day EMA of the 5-day EMA itself as the slow line; buy when the fast line crosses the slow line upward, and sell otherwise)
    my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

    # Fixedly buy 1000 shares each time
    my_mm = MM_FixedCount(1000)

    # Create the trading system and run it
    sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
    sys.run(sm['sz000001'], Query(-150))
        

.. py:class:: OrderBrokerWrap

    Used to wrap the order broker wrapper class in python, so that the proxy class in python does not need to inherit from OrderBrokerBase; it only needs to contain the implementations of the buy, sell and get_asset_info methods. Such python proxy classes need to be wrapped with crtOB before they can be called by c++.

    .. py:method:: __init__(self, broker, name)

        :param broker: the python broker instance
        :param str name: the name

    .. py:method:: _buy(self, market, code, price, num, stoploss, goal_price, part_from)
    
        Wrap the buy method of the python object
        
        :param str market: the security market "SH" | "SZ"
        :param str code: the security code
        :param float price: the buy price
        :param float num: the buy quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source

    .. py:method:: _sell(self, market, code, price, num, stoploss, goal_price, part_from)
    
        Wrap the sell method of the python object
        
        :param str market: the security market
        :param str code: the security code
        :param float price: the sell price
        :param float num: the sell quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source

    .. py:method:: _get_asset_info(self)

        For the details, see :py:meth:`OrderBrokerBase._get_asset_info`
                        

.. py:function:: crtOB(broker[, name="NO_NAME"]) 

    Quickly generate the order broker wrapper object
    

Built-in Order Broker Classes
-----------------------------    
    
.. py:class:: TestOrderBroker

    Used for testing; prints when executing the buy/sell operations, e.g.: "Buy: SH000001, price: 10.0, number: 1000, ..."

.. py:class:: MailOrderBroker

    The mail order broker, which sends an Email when executing the buy/sell operations, e.g.::
        
        my_tm.regBroker(crtOB(MailOrderBroker("smtp.sina.com", "yourmail@sina.com", "yourpwd", "receivermail@XXX.yy)))
    
    .. py:method:: __init__(self, host, sender, pwd, receivers)

        The initialization constructor
        
        :param str host: the smtp server address
        :param int port: the smtp server port
        :param str sender: the sender mailbox (i.e. the user name)
        :param str pwd: the password
        :param list receivers: the list of the receiver mailboxes

    .. py:method:: buy(self, market, code, price, num)
    
        Execute the buy operation and send an email to the specified mailbox, in the following format:
        
            The email title: [Hkyuu Notice] Buy <stock code>
            The email content: Buy: <stock code>, price: <buy price>, number: <buy quantity>
        
        :param str market: the security market
        :param str code: the security code
        :param float price: the buy price
        :param float num: the buy quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source
        
        
    .. py:method:: sell(self, market, code, price, num)
    
        Execute the sell operation and send an email to the specified mailbox, in the following format:
        
            The email title: [Hkyuu Notice] Sell <stock code>
            The email content: Sell: <stock code>, price: <sell price>, number: <sell quantity>
    
        :param str market: the security market
        :param str code: the security code
        :param float price: the sell price
        :param float num: the sell quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source       


Order Broker Base Class
-----------------------

It is not necessary to use OrderBrokerBase in Python to implement a custom order broker. As long as the Python object contains the buy and sell methods, whose method parameter rules are the same as the _buy and _sell methods in :py:class:`OrderBrokerWrap`, see the previous section to quickly create an order broker instance.

The custom order broker interfaces:

* :py:meth:`OrderBrokerBase._buy` - [Required] Execute the actual buy operation
* :py:meth:`OrderBrokerBase._sell` - [Required] Execute the actual sell operation
* :py:meth:`OrderBrokerBase._get_asset_info` - [Optional] Return the current asset information; if sys/pf needs to be used in Strategy, this interface needs to be implemented


.. py:class:: OrderBrokerBase

    The order broker base class, implementing the actual order operations and the programmatic orders
    
    .. py:attribute:: name The broker name
    
    .. py:method:: __init__(self[, name='NO_NAME'])
    
        Initialize the order broker base class
        
        :param str name: the broker name
        
    .. py:method:: buy(self, market, code, price, num, stoploss, goal_price, part_from)

        Execute the buy operation
    
        :param str market: the security market
        :param str code: the security code
        :param float price: the buy price
        :param float num: the buy quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source
        :return: the execution moment of the buy operation
        :rtype: Datetime
        
    .. py:method:: sell(self, market, code, price, num, stoploss, goal_price, part_from)
    
        Execute the sell operation
    
        :param str market: the security market
        :param str code: the security code
        :param float price: the sell price
        :param float num: the sell quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source        
        :return: the execution moment of the sell operation
        :rtype: Datetime

    .. py:method:: _buy(self, market, code, price, num, stoploss, goal_price, part_from)

        [Override hook] Execute the actual buy operation
    
        :param str code: the security code
        :param float price: the buy price
        :param float num: the buy quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source        
        
    .. py:method:: _sell(self, market, code, price, num, stoploss, goal_price, part_from)
    
        [Override hook] Execute the actual sell operation
    
        :param str market: the security market    
        :param str code: the security code
        :param float price: the sell price
        :param float num: the sell quantity
        :param float stoploss: the planned stop-loss price
        :param float goal_price: the planned profit target price
        :param SystemPart part_from: the signal source

    .. py:method:: _get_asset_info(self)

        [Subclass interface] Get the current asset information; the subclass needs to return a json string conforming to the following specification::

            {
                "datetime": "2001-01-01 18:00:00.12345",
                "cash": 0.0,
                "positions": [
                    {"market": "SZ", "code": "000001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
                    "cost_price": 0.0},
                    {"market": "SH", "code": "600001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
                    "cost_price": 0.0},
                ]
            }    

        :return: return the current asset information as a string (in json format)
        :rtype: str    
