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
#pragma warning(push)
#pragma warning(disable : 4521)
#endif

namespace hku {

/**
 * 函数及函数对象等包装器实现移动语义，以便线程池支持不同类型的任务
 */
class FuncWrapper {
public:
    FuncWrapper() = default;
    FuncWrapper(const FuncWrapper&) = delete;
    FuncWrapper(FuncWrapper&) = delete;
    FuncWrapper& operator=(const FuncWrapper&) = delete;

    /** 移动构造函数，实现对函数及函数对象等任务包装 */
    template <typename F>
    // cppcheck-suppress noExplicitConstructor ; 此处不能添加 explicit 修饰，需要使用转换复制
    FuncWrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}

    /** 执行被包装的任务 */
    void operator()() {
        if (impl) {
            impl->call();
        }
    }

    /** 移动构造函数 */
    FuncWrapper(FuncWrapper&& other) : impl(std::move(other.impl)) {}

    /** 移动复制函数 */
    FuncWrapper& operator=(FuncWrapper&& other) {
        impl = std::move(other.impl);
        return *this;
    }

    /** 是否是空任务，用于线程池判断是否在所有任务完成后终止运行 */
    bool isNullTask() const {
        return impl ? false : true;
    }

private:
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;

    template <typename F>
    struct impl_type : impl_base {
        F f;
        // cppcheck-suppress noExplicitConstructor ; 此处不能添加 explicit 修饰，需要使用转换复制
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() override {
            f();
        }
    };
};

} /* namespace hku */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* HIKYUU_UTILITIES_THREAD_FUNCWRAPPER_H */
