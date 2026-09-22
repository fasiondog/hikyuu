/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-16
 *      Author: fasiondog
 */

#include "../KDataDriver.h"

namespace hku {

/**
 * A special KDataDriver that does not actually read the data, used when adding an external
 * temporary Stock
 */
class DoNothingKDataDriver : public KDataDriver {
public:
    DoNothingKDataDriver() : KDataDriver("DoNothing") {}
    virtual ~DoNothingKDataDriver() override = default;

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