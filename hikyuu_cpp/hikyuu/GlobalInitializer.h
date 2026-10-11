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
            try {
                init();
            } catch (...) {
                // The destructor never runs for an object whose construction threw, so the count
                // has to be rolled back: it would otherwise stay above zero forever and clean()
                // would never be reached
                --m_count;
                throw;
            }
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