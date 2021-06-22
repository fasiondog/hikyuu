# -*- coding: utf-8 -*-

import hashlib
from .restful import check_res, session_get, session_post, session_delete, session_put

from data import SessionModel

class TradeService:
    @staticmethod
    def query_td_accounts(session: SessionModel):
        pass