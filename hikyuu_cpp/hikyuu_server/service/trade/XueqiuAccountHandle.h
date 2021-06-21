/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#pragma once

#include "db/db.h"
#include "service/RestHandle.h"
#include "service/user/model/UserModel.h"
#include "bean/TradeDbBean.h"
#include "model/TradeAccountModel.h"

namespace hku {

class QueryXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(QueryXueqiuAccountHandle)
    virtual void run() override {
        auto xq_accounts = TradeDbBean::queryAllXueqiuAccount();
        json jarray;
        for (auto& account : xq_accounts) {
            json j;
            j["td_id"] = account.getTdId();
            j["name"] = account.getName();
            j["cookies"] = account.getCookies();
            j["portfolio_code"] = account.getPortfolioCode();
            j["portfolio_market"] = account.getPortfolioMarket();
            jarray.push_back(j);
        }
        res["data"] = jarray;
    }
};

class AddXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(AddXueqiuAccountHandle)
    virtual void run() override {
        check_missing_param({"name", "cookies", "portfolio_code", "portfolio_market"});

        auto con = TradeDbBean::getConnect();
        try {
            TransAction trans(con);
            TradeAccountModel td;
            td.setName(req["name"].get<std::string>());
            td.setType(XueqiuAccountModel::ACCOUNT_TYPE);
            con->save(td, false);
            APP_CHECK(td.id() != 0, "Insert TradeAccountModel error!");

            XueqiuAccountModel xq;
            xq.setTdId(td.id());
            xq.setCookies(req["cookies"].get<std::string>());
            xq.setPortfolioCode(req["portfolio_code"].get<std::string>());
            xq.setPortfolioMarket(req["portfolio_market"].get<std::string>());
            con->save(xq, false);
        } catch (...) {
            con->rollback();
            throw;
        }
    }
};

class RemoveXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(RemoveXueqiuAccountHandle)
    virtual void run() override {
        check_missing_param("td_id");
        int64_t td_id = req["td_id"].get<int64_t>();
        auto con = TradeDbBean::getConnect();
        try {
            TransAction trans(con);
            con->exec(fmt::format("delete from {} where td_id={}",
                                  XueqiuAccountModel::getTableName(), td_id));
            con->exec(fmt::format("delete from {} where td_id={}",
                                  TradeAccountModel::getTableName(), td_id));
        } catch (...) {
            con->rollback();
            throw;
        }
    }
};

class ModifyXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(ModifyXueqiuAccountHandle)
    virtual void run() override {
        check_missing_param("td_id");
        int64_t td_id = req["td_id"].get<int64_t>();
        auto con = TradeDbBean::getConnect();
        try {
            TransAction trans(con);
            if (req.contains("name")) {
                con->exec(fmt::format(R"(update {} set name="{}" where td_id={})",
                                      TradeAccountModel::getTableName(), td_id));
            }

            std::ostringstream buf;
            buf << "update " << XueqiuAccountModel::getTableName() << " set ";
            bool need_modify = false;
            if (req.contains("cookies")) {
                buf << "cookies=\"" << req["cookies"].get<std::string>() << "\" ";
                need_modify = true;
            }
            if (req.contains("portfolio_code")) {
                if (need_modify) {
                    buf << ", ";
                }
                buf << "portfolio_code=\"" << req["portfolio_code"].get<std::string>() << "\" ";
                need_modify = true;
            }
            if (req.contains("portfolio_market")) {
                if (need_modify) {
                    buf << ", ";
                }
                buf << "portfolio_market=\"" << req["portfolio_market"].get<std::string>() << "\" ";
                need_modify = true;
            }

            if (need_modify) {
                buf << "where td_id=" << td_id;
                con->exec(buf.str());
            }

        } catch (...) {
            con->rollback();
            throw;
        }
    }
};

}  // namespace hku