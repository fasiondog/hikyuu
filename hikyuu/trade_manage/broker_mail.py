#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# ===============================================================================
# History
# 1. 20170704, Added by fasiondog
# ===============================================================================

import smtplib
from email.mime.text import MIMEText
from email.header import Header


class MailOrderBroker:
    """
    邮件订单代理
    """

    def __init__(self, host, sender, pwd, receivers):
        """
        邮件订单代理，执行买入/卖出操作时发送 Email

        :param str host: smtp服务器地址
        :param int port: smtp服务器端口
        :param str sender: 发件邮箱（既用户名）
        :param str pwd: 密码
        :param list receivers: 接受者邮箱列表
        """
        self._host = host
        self._pwd = pwd
        self._sender = sender
        self._receivers = receivers

    def _sendmail(self, title, msg):
        """发送邮件

        :param str title: 邮件标题
        :param str msg: 邮件内容
        """
        message = MIMEText(msg, 'plain', 'utf-8')
        message['From'] = self._sender
        to_mail = ""
        for r in self._receivers:
            to_mail = to_mail + "," + r
        message['To'] = to_mail
        message['Subject'] = Header(title, 'utf-8')
        smtpObj = smtplib.SMTP()
        smtpObj.connect(self._host, 25)
        smtpObj.login(self._sender, self._pwd)
        smtpObj.sendmail(self._sender, self._receivers, message.as_string())

    def buy(self, market, code, price, num, stoploss, goal_price, part_from):
        """执行买入操作，向指定的邮箱发送邮件，格式如下::

            邮件标题：【Hkyuu提醒】买入 证券代码
            邮件内容：买入：证券代码，价格：买入的价格，数量：买入数量

        :param str code: 证券代码
        :param float price: 买入价格
        :param int num: 买入数量
        """
        action = "买入：{}{}，价格：{}，数量：{} ".format(market, code, price, num)
        title = "【Hkyuu提醒】买入 {}".format(code)
        self._sendmail(title, action)

    def sell(self, market, code, price, num, stoploss, goal_price, part_from):
        """执行卖出操作，向指定的邮箱发送邮件，格式如下::

            邮件标题：【Hkyuu提醒】卖出 证券代码
            邮件内容：卖出：证券代码，价格：卖出的价格，数量：卖出数量

        :param str code: 证券代码
        :param float price: 卖出价格
        :param int num: 卖出数量
        """
        title = "【Hkyuu提醒】卖出 {}{}".format(market, code)
        action = "卖出：{}，价格：{}，数量：{} ".format(code, price, num)
        self._sendmail(title, action)
