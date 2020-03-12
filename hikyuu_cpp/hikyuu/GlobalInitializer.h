/*
 * GlobalInitializer.h
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-01
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_GLOBAL_INITIALIZER_H
#define HKU_GLOBAL_INITIALIZER_H

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

class HKU_API GlobalInitializer {
public:
    GlobalInitializer() {
        if (m_count++ == 0) {
            init();
        }
    }

    ~GlobalInitializer() {
        if (--m_count == 0) {
            clean();
        }
    }

private:
    void init();
    void clean();

private:
    static int m_count;
};

static GlobalInitializer s_global_initializer;

} /* namespace hku */

#endif /* HKU_GLOBAL_INITIALIZER_H */