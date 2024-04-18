/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-16
 *      Author: fasiondog
 */

#include "../KDataDriver.h"

namespace hku {

/**
 * 一个特殊的 KDatadriver，不实际读取数据，用于增加外部临时Stock时使用
 */
class DoNothingKDataDriver : public KDataDriver {
public:
    DoNothingKDataDriver() : KDataDriver("DoNothing") {}
    virtual ~DoNothingKDataDriver() = default;

    virtual KDataDriverPtr _clone() override {
        return std::make_shared<DoNothingKDataDriver>();
    }

    virtual bool isIndexFirst() override {
        return true;
    }

    virtual bool canParallelLoad() override {
        return true;
    }
};

}  // namespace hku