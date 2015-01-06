/*
 * AmaSigal.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "AmaSignal.h"
#include "../../../indicator/crt/AMA.h"
#include "../../../indicator/crt/KDATA.h"
#include "../../../indicator/crt/DIFF.h"
#include "../../../indicator/crt/STDEV.h"

namespace hku {

AmaSignal::AmaSignal(int n, int fast_n, int slow_n,
        int filter_n, double filter_p, const string& kpart)
: SignalBase("AMA") {
    addParam<int>("n", n);
    addParam<int>("fast_n", fast_n);
    addParam<int>("slow_n", slow_n);
    addParam<int>("filter_n", filter_n);
    addParam<double>("filter_p", filter_p);
    addParam<string>("kpart", kpart);
}

AmaSignal::~AmaSignal() {

}

void AmaSignal::_reset() {

}

SignalPtr AmaSignal::_clone() {
    int n = getParam<int>("n");
    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");
    int filter_n = getParam<int>("filter_n");
    double p = getParam<double>("filter_p");
    string kpart = getParam<string>("kpart");
    return SignalPtr(new AmaSignal(n, fast_n, slow_n, filter_n, p, kpart));
}

void AmaSignal::_calculate() {
    int n = getParam<int>("n");
    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");
    int filter_n = getParam<int>("filter_n");
    double filter_p = getParam<double>("filter_p");
    string kpart = getParam<string>("kpart");

    Indicator ama = AMA(KDATA_PART(m_kdata, kpart), n, fast_n, slow_n);
    Indicator dev = STDEV(DIFF(ama), filter_n);

    size_t start = dev.discard();
    if (start < 3) {
        return;
    }

    bool buy = true;
    size_t total = dev.size();
    for (size_t i = start; i < total; ++i) {
        double dama = ama[i] - ama[i-1];
        double dama2 = ama[i] - ama[i-2];
        double dama3 = ama[i] - ama[i-3];
        double sdama = dev[i] * filter_p;
        if (buy && dama >= 0 && (dama > sdama || dama2 > sdama || dama3 > sdama)) {
            _addBuySignal(m_kdata[i].datetime);
            buy = false;
        } else if (!buy && dama < 0 && (dama < sdama || dama2 < sdama || dama3 < sdama)) {
            _addSellSignal(m_kdata[i].datetime);
            buy = true;
        }
    }
}


SignalPtr HKU_API AMA_SG(int n, int fast_n, int slow_n,
        int filter_n, double filter_p, const string& kpart) {
    return SignalPtr(new AmaSignal(n, fast_n, slow_n, filter_n, filter_p, kpart));
}

} /* namespace hku */
