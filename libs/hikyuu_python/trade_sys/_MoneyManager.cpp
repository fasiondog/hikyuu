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

    size_t getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        if (override getSellNumber = this->get_override("getSellNumber")) {
            return getSellNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::getSellNumber(datetime, stock, price, risk);
    }

    size_t default_getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::getSellNumber(datetime, stock, price, risk);
    }

    size_t getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return this->get_override("getBuyNumber")(datetime, stock, price, risk);
    }

    size_t getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        if (override getSellShortNumber = this->get_override("getSellShortNumber")) {
            return getSellShortNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::getSellShortNumber(datetime, stock, price, risk);
    }

    size_t default_getSellShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::getSellShortNumber(datetime, stock, price, risk);
    }

    size_t getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        if (override getBuyShortNumber = this->get_override("getBuyShortNumber")) {
            return getBuyShortNumber(datetime, stock, price, risk);
        }
        return MoneyManagerBase::getBuyShortNumber(datetime, stock, price, risk);
    }

    size_t default_getBuyShortNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
        return MoneyManagerBase::getBuyShortNumber(datetime, stock, price, risk);
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
            .def("getBuyNumber", pure_virtual(&MoneyManagerBase::getBuyNumber))
            .def("getSellNumber", &MoneyManagerBase::getSellNumber,
                                  &MoneyManagerWrap::default_getSellNumber)
            .def("getSellShortNumber", &MoneyManagerBase::getSellShortNumber,
                                  &MoneyManagerWrap::default_getSellShortNumber)
            .def("getBuyShortNumber", &MoneyManagerBase::getBuyShortNumber,
                                  &MoneyManagerWrap::default_getBuyShortNumber)
            .def("_reset", pure_virtual(&MoneyManagerBase::_reset))
            .def("_clone", pure_virtual(&MoneyManagerBase::_clone))
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(name_init_pickle_suite<MoneyManagerBase>())
#endif
            ;

    register_ptr_to_python<MoneyManagerPtr>();

    def("FixedCount_MM", FixedCount_MM);
}



