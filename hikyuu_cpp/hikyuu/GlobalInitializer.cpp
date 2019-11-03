/*
 * GlobalInitializer.cpp
 * 
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-01
 *      Author: fasiondog
 */

#include "Log.h"
#include "GlobalInitializer.h"

namespace hku {

int GlobalInitializer::m_count = 0;

void GlobalInitializer::init() {
    inner::g_hikyuu_logger = inner::init_logger();
}

void GlobalInitializer::clean() {

}

} /* namespace hku */