/*
 * _MoneyManager.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/moneymanager/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class MoneyManagerWrap : public MoneyManagerBase, public wrapper<MoneyManagerBase> {
public:
    MoneyManagerWrap() : MoneyManagerBase() {}
    MoneyManagerWrap(const string& name): MoneyManagerBase(name) {}

    void buyNotify(const TradeRecord& record) {
        if (override buyNotify = this->get_override("buyNotify")) {
            buyNotify(record);
            return;
        }
        MoneyManagerBase::buyNotify(record);
    }

    void default_buyNotify(const TradeRecord& record) {
        MoneyManagerBase::buyNotify(record);
    }

    void sellNotify(const TradeRecord& record) {
        if (override sellNotify = this->get_override("sellNotify")) {
            sellNotify(record);
            return;
        }
        MoneyManagerBase::sellNotify(record);
    }

    void default_sellNotify(const TradeRecord& record) {
        MoneyManagerBase::sellNotify(record);
    }

    size_t _getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        if (override _getSellNumber = this->get_override("_getSellNumber")) {
            return _getSellNumber(datetime, stock, price, risk, from);
        }
        return MoneyManagerBase::_getSellNumber(datetime, stock, price, risk, from);
    }

    size_t default_getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        return MoneyManagerBase::_getSellNumber(datetime, stock, price, risk, from);
    }

    size_t _getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        return this->get_override("_getBuyNumber")(datetime, stock, price, risk, from);
    }

    size_t _getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        if (override _getSellShortNumber = this->get_override("_getSellShortNumber")) {
            return _getSellShortNumber(datetime, stock, price, risk, from);
        }
        return MoneyManagerBase::_getSellShortNumber(datetime, stock, price, risk, from);
    }

    size_t default_getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        return MoneyManagerBase::_getSellShortNumber(datetime, stock, price, risk, from);
    }

    size_t getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        if (override _getBuyShortNumber = this->get_override("_getBuyShortNumber")) {
            return _getBuyShortNumber(datetime, stock, price, risk, from);
        }
        return MoneyManagerBase::_getBuyShortNumber(datetime, stock, price, risk, from);
    }

    size_t default_getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
        return MoneyManagerBase::_getBuyShortNumber(datetime, stock, price, risk, from);
    }

    void _reset() {
        if (override func = get_override("_reset")) {
            func();
        } else {
            MoneyManagerBase::_reset();
        }
    }

    void default_reset()  {
        this->MoneyManagerBase::_reset();
    }

    MoneyManagerPtr _clone() {
        return this->get_override("_clone")();
    }
};


string (MoneyManagerBase::*mm_get_name)() const = &MoneyManagerBase::name;
void (MoneyManagerBase::*mm_set_name)(const string&) = &MoneyManagerBase::name;


void export_MoneyManager() {
    class_<MoneyManagerWrap, boost::noncopyable>("MoneyManagerBase", init<>())
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", mm_get_name, mm_set_name)
            .def("getParam", &MoneyManagerBase::getParam<boost::any>)
            .def("setParam", &MoneyManagerBase::setParam<object>)
            .def("setTM", &MoneyManagerBase::setTM)
            .def("getTM", &MoneyManagerBase::getTM)
            .def("setQuery", &MoneyManagerBase::setQuery)
            .def("getQuery", &MoneyManagerBase::getQuery)
            .def("reset", &MoneyManagerBase::reset)
            .def("clone", &MoneyManagerBase::clone)
            .def("buyNotify", &MoneyManagerBase::buyNotify,
                              &MoneyManagerWrap::default_buyNotify)
            .def("sellNotify", &MoneyManagerBase::sellNotify,
                               &MoneyManagerWrap::default_sellNotify)

            .def("getBuyNumber", &MoneyManagerBase::getBuyNumber)
            .def("getSellNumber", &MoneyManagerBase::getSellNumber)
            .def("getSellShortNumber", &MoneyManagerBase::getSellShortNumber)
            .def("getBuyShortNumber", &MoneyManagerBase::getBuyShortNumber)

            .def("_getBuyNumber", pure_virtual(&MoneyManagerBase::_getBuyNumber))
            .def("_getSellNumber", &MoneyManagerBase::_getSellNumber,
                                  &MoneyManagerWrap::default_getSellNumber)
            .def("_getSellShortNumber", &MoneyManagerBase::_getSellShortNumber,
                                  &MoneyManagerWrap::default_getSellShortNumber)
            .def("_getBuyShortNumber", &MoneyManagerBase::_getBuyShortNumber,
                                  &MoneyManagerWrap::default_getBuyShortNumber)
            .def("_reset", &MoneyManagerBase::_reset, &MoneyManagerWrap::default_reset)
            .def("_clone", pure_virtual(&MoneyManagerBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<MoneyManagerBase>())
#endif
            ;

    register_ptr_to_python<MoneyManagerPtr>();

    def("MM_Nothing", MM_Nothing);
    def("MM_FixedRisk", MM_FixedRisk, (arg("risk")=1000.00));
    def("MM_FixedCapital", MM_FixedCapital, (arg("capital") = 10000.00));
    def("MM_FixedCount", MM_FixedCount, (arg("n")=100));
    def("MM_FixedPercent", MM_FixedPercent, (arg("p")=0.03));
    def("MM_FixedUnits", MM_FixedUnits, (arg("n") = 33));
    def("MM_WilliamsFixedRisk", MM_WilliamsFixedRisk, (arg("p") = 0.1, arg("max_loss")=1000.0));
}



