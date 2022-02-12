/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#include "common/log.h"
#include "TradeDbBean.h"

namespace hku {

std::vector<XueqiuAccountModel> TradeDbBean::queryAllXueqiuAccount() {
    std::vector<XueqiuAccountModel> result;
    std::string sql(
      fmt::format("select a.id, a.td_id, a.cookies, a.portfolio_code, a.portfolio_market, b.name, "
                  "b.userid from "
                  "td_account_xq a, td_account b where a.td_id=b.td_id and b.type='{}'",
                  XueqiuAccountModel::ACCOUNT_TYPE));
    auto con = TradeDbBean::getConnect();
    SQLStatementPtr st = con->getStatement(sql);
    st->exec();
    while (st->moveNext()) {
        XueqiuAccountModel mo;
        st->getColumn(0, mo.m_id, mo.td_id, mo.cookies, mo.portfolio_code, mo.portfolio_market,
                      mo.m_name, mo.m_userid);
        result.push_back(mo);
    }

    return result;
}

bool TradeDbBean::queryXueqiuAccountsByTdId(int64_t id, XueqiuAccountModel& out_mo) {
    std::string sql(fmt::format(
      "select a.id, a.td_id, a.cookies, a.portfolio_code, a.portfolio_market, b.name, "
      "b.userid from "
      "td_account_xq a, td_account b where a.td_id=b.td_id and b.type='{}' and a.td_id={}",
      XueqiuAccountModel::ACCOUNT_TYPE, id));
    auto con = TradeDbBean::getConnect();
    SQLStatementPtr st = con->getStatement(sql);
    st->exec();
    if (st->moveNext()) {
        st->getColumn(0, out_mo.m_id, out_mo.td_id, out_mo.cookies, out_mo.portfolio_code,
                      out_mo.portfolio_market, out_mo.m_name, out_mo.m_userid);
        return true;
    }
    return false;
}

}  // namespace hku