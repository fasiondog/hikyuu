/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "IInBlock.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IInBlock)
#endif

namespace hku {

IInBlock::IInBlock() : IndicatorImp("INBLOCK", 1) {
    setParam<string>("category", "");
    setParam<string>("name", "");
}

IInBlock::~IInBlock() {}

IInBlock::IInBlock(const KData& kdata, const string& category, const string& name)
: IndicatorImp("INBLOCK", 1) {
    setParam<string>("category", category);
    setParam<string>("name", name);
    setParam<KData>("kdata", kdata);
    IInBlock::_calculate(Indicator());
}

void IInBlock::_calculate(const Indicator& data) {
    HKU_IF_RETURN(!isLeaf() && !data.empty(), void());

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    Block block = getBlock(getParam<string>("category"), getParam<string>("name"));
    value_t in = block.have(k.getStock()) ? 1.0 : 0.0;
    auto* dst = this->data();
    for (size_t i = 0; i < total; ++i) {
        dst[i] = in;
    }
}

Indicator HKU_API INBLOCK(const string& category, const string& name) {
    auto p = make_shared<IInBlock>();
    p->setParam<string>("category", category);
    p->setParam<string>("name", name);
    return Indicator(p);
}

Indicator HKU_API INBLOCK(const KData& k, const string& category, const string& name) {
    return Indicator(make_shared<IInBlock>(k, category, name));
}

} /* namespace hku */
