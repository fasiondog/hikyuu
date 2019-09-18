/*
 * FuncWrapper.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_THREAD_FUNCWRAPPER_H
#define HIKYUU_UTILITIES_THREAD_FUNCWRAPPER_H

#include <memory>
#include <functional>

#ifdef _MSC_VER
#pragma  warning(push) 
#pragma  warning(disable: 4521)
#endif

namespace hku {

class FuncWrapper {
public:
    FuncWrapper() = default;
    FuncWrapper(const FuncWrapper&)=delete;
    FuncWrapper(FuncWrapper&)=delete;
    FuncWrapper& operator=(const FuncWrapper&)=delete;

    template<typename F>
    FuncWrapper(F&& f): impl(new impl_type<F>(std::move(f))) {}

    void operator()() { impl->call(); }

    FuncWrapper(FuncWrapper&& other): impl(std::move(other.impl)) {}

    FuncWrapper& operator=(FuncWrapper&& other) {
        impl=std::move(other.impl);
        return *this;
    }

    bool is_stop_task() {
        return impl ? false : true;
    }

private:    
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    
    template<typename F>
    struct impl_type: impl_base {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call() { f(); }
    };
};

} /* namespace hku */

#ifdef _MSC_VER
#pragma  warning(pop) 
#endif

#endif /* HIKYUU_UTILITIES_THREAD_FUNCWRAPPER_H */
