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
    Email order broker
    """

    def __init__(self, host, sender, pwd, receivers):
        """
        Email order broker, sends an email when executing the buy/sell operations

        :param str host: the smtp server address
        :param int port: the smtp server port
        :param str sender: the sender mailbox (i.e. the user name)
        :param str pwd: the password
        :param list receivers: the list of the receiver mailboxes
        """
        self._host = host
        self._pwd = pwd
        self._sender = sender
        self._receivers = receivers

    def _sendmail(self, title, msg):
        """Send an email

        :param str title: the email title
        :param str msg: the email content
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

    def buy(self, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        """Execute the buy operation, send an email to the specified mailbox, in the following format::

            Email title: [Hkyuu Notice] Buy the stock code
            Email content: Buy: the stock code, price: the buy price, number: the buy number

        :param str code: the stock code
        :param float price: the buy price
        :param int num: the buy number
        """
        action = "Buy: {}{}, price: {}, number: {} ".format(market, code, price, num)
        title = "[Hkyuu Notice] Buy {}".format(code)
        self._sendmail(title, action)

    def sell(self, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        """Execute the sell operation, send an email to the specified mailbox, in the following format::

            Email title: [Hkyuu Notice] Sell the stock code
            Email content: Sell: the stock code, price: the sell price, number: the sell number

        :param str code: the stock code
        :param float price: the sell price
        :param int num: the sell number
        """
        title = "[Hkyuu Notice] Sell {}{}".format(market, code)
        action = "Sell: {}, price: {}, number: {} ".format(code, price, num)
        self._sendmail(title, action)
