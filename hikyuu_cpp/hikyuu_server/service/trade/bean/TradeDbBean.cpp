/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#include "TradeDbBean.h"

namespace hku {

std::vector<XueqiuAccountModel> TradeDbBean::queryAllXueqiuAccount() {
    std::vector<XueqiuAccountModel> result;
    auto con = TradeDbBean::getConnect();
    std::string sql(fmt::format(
      R"(select a.id, a.td_id, a.cookies, a.portfolio_code, a.portfolio_market, b.name, "
                  "b.userid from "
                  "td_account_xq a, td_account b where a.td_id=b.td_id and b.td_id="{}")",
      XueqiuAccountModel::ACCOUNT_TYPE));
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

}  // namespace hku