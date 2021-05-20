/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#include "DBCondition.h"

namespace hku {

DBCondition& DBCondition::operator&(const DBCondition& other) {
    if (this == &other) {
        return *this;
    }

    if (m_condition.empty()) {
        m_condition = other.m_condition;
    } else {
        m_condition += " and " + other.m_condition;
    }

    return *this;
}

DBCondition& DBCondition::operator|(const DBCondition& other) {
    if (this == &other) {
        return *this;
    }

    if (m_condition.empty()) {
        m_condition = other.m_condition;
    } else {
        m_condition += " or " + other.m_condition;
    }

    return *this;
}

}  // namespace hku