/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-15
 *      Author: fasiondog
 */

#include "hikyuu/utilities/node/NodeClient.h"
#include "interface/plugins.h"
#include "dataserver.h"

namespace hku {

void HKU_API startDataServer(const std::string& addr, size_t work_num, bool save_tick,
                             bool buf_tick, const std::string& parquet_path) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataServerPluginInterface>(HKU_PLUGIN_DATASERVER);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_DATASERVER));
    plugin->start(addr, work_num, save_tick, buf_tick, parquet_path);
}

void HKU_API stopDataServer() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataServerPluginInterface>(HKU_PLUGIN_DATASERVER);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DATASERVER);
    plugin->stop();
}

void HKU_API getDataFromBufferServer(const std::string& addr, const StockList& stklist,
                                     const KQuery::KType& ktype) {
    // SPEND_TIME(getDataFromBufferServer);
    const auto& preload = StockManager::instance().getPreloadParameter();
    string low_ktype = ktype;
    to_lower(low_ktype);
    HKU_ERROR_IF_RETURN(!preload.tryGet<bool>(low_ktype, false), void(),
                        "The {} kdata is not preload! Can't update!", low_ktype);

    NodeClient client(addr);
    try {
        HKU_CHECK(client.dial(), "Failed dial server!");
        json req;
        req["cmd"] = "market";
        req["ktype"] = ktype;
        json code_list;
        json date_list;
        for (const auto& stk : stklist) {
            if (!stk.isNull()) {
                code_list.emplace_back(stk.market_code());
                auto k = stk.getKData(KQueryByIndex(-1, Null<int64_t>(), ktype));
                if (k.empty()) {
                    date_list.emplace_back(Datetime::min().str());
                } else {
                    date_list.emplace_back(k[k.size() - 1].datetime.str());
                }
            }
        }
        req["codes"] = std::move(code_list);
        req["dates"] = std::move(date_list);

        json res;
        client.post(req, res);
        HKU_ERROR_IF_RETURN(res["ret"] != NodeErrorCode::SUCCESS, void(),
                            "Recieved error: {}, msg: {}", res["ret"].get<int>(),
                            res["msg"].get<string>());

        const auto& jdata = res["data"];
        // HKU_INFO("{}", to_string(jdata));
        for (auto iter = jdata.cbegin(); iter != jdata.cend(); ++iter) {
            const auto& r = *iter;
            try {
                string market_code = r["code"].get<string>();
                Stock stk = getStock(market_code);
                if (stk.isNull()) {
                    continue;
                }

                const auto& jklist = r["data"];
                for (auto kiter = jklist.cbegin(); kiter != jklist.cend(); ++kiter) {
                    const auto& k = *kiter;
                    KRecord kr(Datetime(k[0].get<string>()), k[1], k[2], k[3], k[4], k[5], k[6]);
                    stk.realtimeUpdate(kr, ktype);
                }

            } catch (const std::exception& e) {
                HKU_ERROR("Failed decode json: {}! {}", to_string(r), e.what());
            }
        }

    } catch (const std::exception& e) {
        HKU_ERROR("Failed get data from buffer server! {}", e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
}

vector<SpotRecord> HKU_API getSpotFromBufferServer(const std::string& addr,
                                                   const std::string& market,
                                                   const std::string& code,
                                                   const Datetime& datetime) {
    vector<SpotRecord> ret;
    NodeClient client(addr);
    try {
        HKU_CHECK(client.dial(), "Failed dial server!");
        json req;
        req["cmd"] = "tick";
        req["market"] = market;
        req["code"] = code;
        req["datetime"] = datetime.str();

        json res;
        client.post(req, res);
        HKU_ERROR_IF_RETURN(res["ret"] != NodeErrorCode::SUCCESS, ret,
                            "Recieved error: {}, msg: {}", res["ret"].get<int>(),
                            res["msg"].get<string>());

        const auto& jdata = res["data"];
        // HKU_INFO("{}", to_string(jdata));
        for (auto iter = jdata.cbegin(); iter != jdata.cend(); ++iter) {
            const auto& r = *iter;
            try {
                SpotRecord spot;
                spot.market = market;
                spot.code = code;
                spot.datetime = Datetime(r[0].get<string>());
                spot.yesterday_close = r[1];
                spot.open = r[2];
                spot.high = r[3];
                spot.low = r[4];
                spot.close = r[5];
                spot.amount = r[6];
                spot.volume = r[7];

                const auto& bid = r[8];
                for (auto siter = bid.cbegin(); siter != bid.cend(); ++siter) {
                    spot.bid.push_back(*siter);
                }

                const auto& bid_amount = r[9];
                for (auto siter = bid_amount.cbegin(); siter != bid_amount.cend(); ++siter) {
                    spot.bid_amount.push_back(*siter);
                }

                const auto& ask = r[10];
                for (auto siter = ask.cbegin(); siter != ask.cend(); ++siter) {
                    spot.ask.push_back(*siter);
                }

                const auto& ask_amount = r[11];
                for (auto siter = ask_amount.cbegin(); siter != ask_amount.cend(); ++siter) {
                    spot.ask_amount.push_back(*siter);
                }

                ret.emplace_back(std::move(spot));

            } catch (const std::exception& e) {
                HKU_ERROR("Failed decode json: {}! {}", to_string(r), e.what());
            }
        }

    } catch (const std::exception& e) {
        HKU_ERROR("Failed get data from buffer server! {}", e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
    return ret;
}

}  // namespace hku