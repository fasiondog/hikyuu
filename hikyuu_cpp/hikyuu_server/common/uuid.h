/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-12
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace hku {

inline std::string UUID() {
    static auto rgen = boost::uuids::random_generator();
    return boost::uuids::to_string(rgen());
}

}  // namespace hku