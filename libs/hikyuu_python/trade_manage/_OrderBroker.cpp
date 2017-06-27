/*
 * _OrderBroker.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/OrderBrokerBase.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class OrderBrokerWrap : public OrderBrokerBase, public wrapper<OrderBrokerBase> {
public:
    OrderBrokerWrap(): OrderBrokerBase() {}

    void _buy(const string& code, price_t price, int num) {
        this->get_override("_buy")(code, price, num);
    }

    void _sell(const string& code, price_t price, int num) {
        this->get_override("_sell")(code, price, num);
    }
};


void export_OrderBroker() {
    class_<OrderBrokerWrap, boost::noncopyable>("OrderBrokerBase", init<>())
            .def("buy", &OrderBrokerBase::buy)
            .def("sell", &OrderBrokerBase::sell)
            .def("_buy", pure_virtual(&OrderBrokerBase::_buy))
            .def("_sell", pure_virtual(&OrderBrokerBase::_sell))
            ;

    register_ptr_to_python<OrderBrokerPtr>();
}


