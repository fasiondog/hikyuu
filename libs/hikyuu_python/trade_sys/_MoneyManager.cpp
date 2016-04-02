/*
 * _MoneyManager.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/moneymanager/build_in.h>
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class MoneyManagerWrap : public MoneyManagerBase, public wrapper<MoneyManagerBase> {
public:
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
            price_t price, price_t risk) {
        if (override _getSellNumber = this->get_override("_getSellNumber")) {
            return _getSellNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::_getSellNumber(datetime, stock, price, risk);
    }

    size_t default_getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::_getSellNumber(datetime, stock, price, risk);
    }

    size_t _getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return this->get_override("_getBuyNumber")(datetime, stock, price, risk);
    }

    size_t _getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        if (override _getSellShortNumber = this->get_override("_getSellShortNumber")) {
            return _getSellShortNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::_getSellShortNumber(datetime, stock, price, risk);
    }

    size_t default_getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::_getSellShortNumber(datetime, stock, price, risk);
    }

    size_t getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        if (override _getBuyShortNumber = this->get_override("_getBuyShortNumber")) {
            return _getBuyShortNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::_getBuyShortNumber(datetime, stock, price, risk);
    }

    size_t default_getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::_getBuyShortNumber(datetime, stock, price, risk);
    }

    void _reset() {
        this->get_override("_reset")();
    }

    MoneyManagerPtr _clone() {
        return this->get_override("_clone")();
    }
};


void export_MoneyManager() {
    class_<MoneyManagerWrap, boost::noncopyable>("MoneyManagerBase", init<const string&>())
            .def(self_ns::str(self))
            .add_property("name",
                    make_function(&MoneyManagerBase::name,
                            return_value_policy<copy_const_reference>()))
            .add_property("params",
                    make_function(&MoneyManagerBase::getParameter,
                            return_internal_reference<>()))
            .def("setTM", &MoneyManagerBase::setTM)
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
            .def("_reset", pure_virtual(&MoneyManagerBase::_reset))
            .def("_clone", pure_virtual(&MoneyManagerBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<MoneyManagerBase>())
#endif
            ;

    register_ptr_to_python<MoneyManagerPtr>();

    def("FixedCount_MM", FixedCount_MM, (arg("n")=100));
    def("PercentRisk_MM", PercentRisk_MM, (arg("p")=0.03));
}



