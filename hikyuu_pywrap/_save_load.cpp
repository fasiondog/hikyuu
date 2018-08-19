/*
 * _save_load.cpp
 *
 *  Created on: 2013-4-26
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <hikyuu/config.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/Parameter.h>
#include <hikyuu/trade_manage/TradeManager.h>
#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/indicator/Operand.h>
#include <hikyuu/trade_sys/all.h>

#include <hikyuu/serialization/all.h>


#if HKU_SUPPORT_SERIALIZATION

using namespace boost::python;
using namespace hku;

static map<size_t , string> g_support_class_dict;

void registerSupportClass() {
    g_support_class_dict[typeid(PriceList).hash_code()] = "PriceList";
    g_support_class_dict[typeid(Datetime).hash_code()] = "Datetime";
    g_support_class_dict[typeid(DatetimeList).hash_code()] = "DatetimeList";
    g_support_class_dict[typeid(KData).hash_code()] = "KData";
    g_support_class_dict[typeid(KQuery).hash_code()] = "KQuery";
    g_support_class_dict[typeid(KRecord).hash_code()] = "KRecord";
    g_support_class_dict[typeid(KRecordList).hash_code()] = "KRecordList";
    g_support_class_dict[typeid(MarketInfo).hash_code()] = "MarketInfo";
    g_support_class_dict[typeid(Stock).hash_code()] = "Stock";
    g_support_class_dict[typeid(Block).hash_code()] = "Block";
    g_support_class_dict[typeid(StockTypeInfo).hash_code()] = "StockTypeInfo";
    g_support_class_dict[typeid(StockWeight).hash_code()] = "StockWeight";
    g_support_class_dict[typeid(StockWeightList).hash_code()] = "StockWeightList";
    g_support_class_dict[typeid(Parameter).hash_code()] = "Parameter";
    g_support_class_dict[typeid(Indicator).hash_code()] = "Indicator";
    g_support_class_dict[typeid(Operand).hash_code()] = "Operand";

    g_support_class_dict[typeid(BorrowRecord).hash_code()] = "BorrowRecord";
    g_support_class_dict[typeid(CostRecord).hash_code()] = "CostRecord";
    g_support_class_dict[typeid(FundsRecord).hash_code()] = "FundsRecord";
    g_support_class_dict[typeid(PositionRecord).hash_code()] = "PositionRecord";
    g_support_class_dict[typeid(PositionRecordList).hash_code()] = "PositionRecordList";
    g_support_class_dict[typeid(TradeCostPtr).hash_code()] = "TradeCostPtr";
    g_support_class_dict[typeid(TradeRecord).hash_code()] = "TradeRecord";
    g_support_class_dict[typeid(TradeRecordList).hash_code()] = "TradeRecordList";
    g_support_class_dict[typeid(TradeManager).hash_code()] = "TradeManager";

    g_support_class_dict[typeid(TradeRequest).hash_code()] = "TradeRequest";
    g_support_class_dict[typeid(SystemPtr).hash_code()] = "SystemPtr";
    g_support_class_dict[typeid(SignalPtr).hash_code()] = "SignalBase";
    g_support_class_dict[typeid(StoplossPtr).hash_code()] = "StoplossBase";
    g_support_class_dict[typeid(MoneyManagerPtr).hash_code()] = "MoneyManagerBase";
    g_support_class_dict[typeid(ProfitGoalPtr).hash_code()] = "ProfitGoalBase";
    g_support_class_dict[typeid(SlippagePtr).hash_code()] = "SlippageBase";
    g_support_class_dict[typeid(ConditionPtr).hash_code()] = "ConditionBase";
    g_support_class_dict[typeid(EnvironmentPtr).hash_code()] = "EnvironmentBase";
}

string supportClassName(const boost::any& arg) {
    map<size_t , string>::const_iterator iter;
    iter = g_support_class_dict.find(arg.type().hash_code());
    if (iter != g_support_class_dict.end()) {
        return iter->second;
    }
    return "Unknown";
}

template <class T>
void xml_save(const T& arg, const string& filename) {
    try {
        std::ofstream ofs(filename);
        if (!ofs) {
            std::cout << "Can't open file(" << filename << ")!" << std::endl;
        }
        boost::archive::xml_oarchive oa(ofs);
        boost::any obj = arg;
        string name(supportClassName(obj));
        oa << boost::serialization::make_nvp("type", name);
        oa << BOOST_SERIALIZATION_NVP(arg);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch(...) {
        std::cout << "Unknow error! [xml_save]" << std::endl;
    }
}

template <class T>
void xml_load(T& arg, const string& filename) {
    try {
        std::ifstream ifs(filename);
        if (!ifs) {
            std::cout << "Can't open file(" << filename << ")!" << std::endl;
        }
        boost::archive::xml_iarchive ia(ifs);
        string name;
        ia >> boost::serialization::make_nvp("type", name);
        boost::any obj = arg;
        if (name == supportClassName(obj)) {
            ia >> BOOST_SERIALIZATION_NVP(arg);
        } else {
            std::cout << "Unsupport type! [xml_load]" << std::endl;
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch(...) {
        std::cout << "Unknow error! [xml_load]" << std::endl;
    }
}


void export_save_load() {
    docstring_options doc_options(false);

    //初始化注册支持的类型
    registerSupportClass();

    def("hku_save", xml_save<PriceList>);
    def("hku_load", xml_load<PriceList>);

    def("hku_save", xml_save<Datetime>);
    def("hku_load", xml_load<Datetime>);

    def("hku_save", xml_save<DatetimeList>);
    def("hku_load", xml_load<DatetimeList>);

    def("hku_save", xml_save<KData>);
    def("hku_load", xml_load<KData>);

    def("hku_save", xml_save<KQuery>);
    def("hku_load", xml_load<KQuery>);

    def("hku_save", xml_save<KRecord>);
    def("hku_load", xml_load<KRecord>);

    def("hku_save", xml_save<KRecordList>);
    def("hku_load", xml_load<KRecordList>);

    def("hku_save", xml_save<MarketInfo>);
    def("hku_load", xml_load<MarketInfo>);

    def("hku_save", xml_save<Stock>);
    def("hku_load", xml_load<Stock>);

    def("hku_save", xml_save<Block>);
    def("hku_load", xml_load<Block>);

    def("hku_save", xml_save<StockTypeInfo>);
    def("hku_load", xml_load<StockTypeInfo>);

    def("hku_save", xml_save<StockWeight>);
    def("hku_load", xml_load<StockWeight>);

    def("hku_save", xml_save<StockWeightList>);
    def("hku_load", xml_load<StockWeightList>);

    def("hku_save", xml_save<Parameter>);
    def("hku_load", xml_load<Parameter>);

    def("hku_save", xml_save<Indicator>);
    def("hku_load", xml_load<Indicator>);

    def("hku_save", xml_save<Operand>);
    def("hku_load", xml_load<Operand>);

    def("hku_save", xml_save<BorrowRecord>);
    def("hku_load", xml_load<BorrowRecord>);

    def("hku_save", xml_save<CostRecord>);
    def("hku_load", xml_load<CostRecord>);

    def("hku_save", xml_save<FundsRecord>);
    def("hku_load", xml_load<FundsRecord>);

    def("hku_save", xml_save<PositionRecord>);
    def("hku_load", xml_load<PositionRecord>);

    def("hku_save", xml_save<PositionRecordList>);
    def("hku_load", xml_load<PositionRecordList>);

    def("hku_save", xml_save<TradeCostPtr>);
    def("hku_load", xml_load<TradeCostPtr>);

    def("hku_save", xml_save<TradeRecord>);
    def("hku_load", xml_load<TradeRecord>);

    def("hku_save", xml_save<TradeRecordList>);
    def("hku_load", xml_load<TradeRecordList>);

    def("hku_save", xml_save<TradeManager>);
    def("hku_load", xml_load<TradeManager>);

    def("hku_save", xml_save<TradeRequest>);
    def("hku_load", xml_load<TradeRequest>);

    def("hku_save", xml_save<SystemPtr>);
    def("hku_load", xml_load<SystemPtr>);

    def("hku_save", xml_save<SignalPtr>);
    def("hku_load", xml_load<SignalPtr>);

    def("hku_save", xml_save<StoplossPtr>);
    def("hku_load", xml_load<StoplossPtr>);

    def("hku_save", xml_save<MoneyManagerPtr>);
    def("hku_load", xml_load<MoneyManagerPtr>);

    def("hku_save", xml_save<ProfitGoalPtr>);
    def("hku_load", xml_load<ProfitGoalPtr>);

    def("hku_save", xml_save<SlippagePtr>);
    def("hku_load", xml_load<SlippagePtr>);

    def("hku_save", xml_save<ConditionPtr>);
    def("hku_load", xml_load<ConditionPtr>);

    def("hku_save", xml_save<EnvironmentPtr>);
    def("hku_load", xml_load<EnvironmentPtr>);
}

#else /* HKU_SUPPORT_SERIALIZATION */

void export_save_load() {

}

#endif /* HKU_SUPPORT_SERIALIZATION */
