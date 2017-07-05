#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20170704, Added by fasiondog
#===============================================================================

import smtplib
from email.mime.text import MIMEText
from email.header import Header
    
class MailOrderBroker:
    def __init__(self, host, sender, pwd, receivers):
        """
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
        
    
    def buy(self, code, price, num):
        action = "买入：{}，价格：{}，数量：{} ".format(code, price, num)
        title = "【Hkyuu提醒】买入 {}".format(code)
        self._sendmail(title, action)
    
    def sell(self, code, price, num):
        title = "【Hkyuu提醒】卖出 {}".format(code)
        action = "卖出：{}，价格：{}，数量：{} ".format(code, price, num)
        self._sendmail(title, action)
        
   