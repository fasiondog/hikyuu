/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "RestHandle.h"
#include "../yyjsonpp/yyjson_doc.h"
#include "../yyjsonpp/yyjson_mut_doc.h"

namespace hku {

class HelloHandle : public RestHandle {
public:
    HelloHandle(nng_aio* aio) : RestHandle(aio) {}

    virtual void run() override {
        std::string req = getReqData();
        try {
            yyjson::doc doc(req);
            yyjson::mut_doc mut_doc(doc);
            auto x = mut_doc.root();
            CLS_INFO("{}", x);
            CLS_INFO("{} {}", x.type_desc(), x.size());
            size_t count = 0;
            for (auto iter = x.arr_begin(); iter != x.arr_end(); ++iter) {
                CLS_INFO("count: {}", count++);
                // CLS_INFO("{} {}", iter->first, iter->second);
                CLS_INFO("{}", *iter);
            }

            /*std::vector<int64_t> y = x.to_vector_int64_t();
            for (auto i : y) {
                CLS_INFO("{}", i);
            }*/

            /*yyjson::mut_doc mut_doc(doc);
            std::vector<bool> y{true, false, true};
            auto z = mut_doc.mut_arr(y);
            const char* strs[3] = {"Jan", "Feb", "Mar"};
            auto zz = mut_doc.mut_arr(strs, 3);*/

            setResData("hello");
        } catch (std::exception& e) {
            CLS_INFO("req: {}", req);
            throw e;
        }
    }
};

}  // namespace hku