/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/utilities/db_connect/DBConnect.h>
#include "db/db.h"

namespace hku {

class TradeDbBean;

class XueqiuAccountModel {
    TABLE_BIND4(XueqiuAccountModel, td_account_xq, td_id, cookies, portfolio_code, portfolio_market)
    friend class TradeDbBean;

public:
    static constexpr const char* ACCOUNT_TYPE = "xq";

    int64_t getTdId() const {
        return td_id;
    }

    void setTdId(int64_t id) {
        td_id = id;
    }

    const std::string& getCookies() const {
        return cookies;
    }

    void setCookies(const std::string& cookies) {
        this->cookies = cookies;
    }

    const std::string& getPortfolioCode() const {
        return portfolio_code;
    }

    void setPortfolioCode(const std::string& code) {
        portfolio_code = code;
    }

    const std::string& getPortfolioMarket() const {
        return portfolio_market;
    }

    void setPortfolioMarket(const std::string& market) {
        portfolio_market = market;
    }

    const std::string& getName() const {
        return m_name;
    }

    void setName(const std::string& name) {
        m_name = name;
    }

private:
    //------------------------------
    // 表内字段
    //------------------------------
    int64_t td_id;                 // 交易账户id
    std::string cookies;           // 雪球 cookies，登陆后获取，获取方式见
                                   // https://smalltool.github.io/2016/08/02/cookie/
    std::string portfolio_code;    // 组合代码(例:ZH818559)
    std::string portfolio_market;  // 交易市场(例:us 或者 cn 或者 hk)

    //------------------------------
    // 非表内字段
    //------------------------------
    int64_t m_userid;
    std::string m_name;
};

}  // namespace hku