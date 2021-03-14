/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "RestHandle.h"
#include "../yyjsonpp/yyjson_doc.h"

namespace hku {

class HelloHandle : public RestHandle {
public:
    HelloHandle(nng_aio* aio) : RestHandle(aio) {}

    virtual void run() override {
        std::string req = getReqData();
        try {
            yyjson::doc doc(req);
            auto x = doc.get_root();
            for (auto iter = x.begin(); iter != x.end(); ++iter) {
                CLS_INFO("{}", (*iter).get_int());
            }

            /*std::vector<int64_t> y = x.to_vector_int64_t();
            for (auto i : y) {
                CLS_INFO("{}", i);
            }*/

            setResData("hello");
        } catch (std::exception& e) {
            CLS_INFO("req: {}", req);
            throw e;
        }
    }
};

}  // namespace hku