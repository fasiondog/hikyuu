# -*- coding: utf-8 -*-

import hashlib
from .restful import check_res, session_get, session_post, session_delete, session_put

from data import SessionModel


class TradeService:
    name = "trade"

    @staticmethod
    def query_xq_accounts(session: SessionModel):
        """ 查询所有雪球账户信息 """
        r = check_res(session_get(session, TradeService.name, "xqaccounts"))
        data = r["data"]
        return data if data is not None else []

    @staticmethod
    def add_xq_account(session: SessionModel, name, cookies, portfolio_code, portfolio_market):
        return check_res(
            session_post(
                session,
                TradeService.name,
                "xqaccount",
                json=dict(name=name, cookies=cookies, portfolio_code=portfolio_code, portfolio_market=portfolio_market)
            )
        )

    @staticmethod
    def remove_xq_account(session: SessionModel, td_id):
        """ 删除指定账户id的账户信息 """
        return check_res(session_delete(session, TradeService.name, "xqaccount", json=dict(td_id=td_id)))

    @staticmethod
    def modify_xq_account(session: SessionModel, account_info):
        return check_res(session_put(session, TradeService.name, "xqaccount", json=account_info))
