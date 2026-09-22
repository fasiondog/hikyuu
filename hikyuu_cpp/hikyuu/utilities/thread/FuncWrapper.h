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
 * The wrapper of the functions and function objects implements the move semantics, so that the
 * thread pool can support different types of the tasks
 */
class FuncWrapper {
public:
    FuncWrapper() = default;
    FuncWrapper(const FuncWrapper&) = delete;
    FuncWrapper(FuncWrapper&) = delete;
    FuncWrapper& operator=(const FuncWrapper&) = delete;

    /** Move constructor, it implements the wrapping of the tasks such as the functions and the
     *  function objects */
    template <typename F>
    // cppcheck-suppress noExplicitConstructor ; the explicit modifier cannot be added here, the
    // conversion copy is needed
    FuncWrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}

    /** Execute the wrapped task */
    void operator()() {
        if (impl) {
            impl->call();
        }
    }

    /** Move constructor */
    FuncWrapper(FuncWrapper&& other) : impl(std::move(other.impl)) {}

    /** Move copy function */
    FuncWrapper& operator=(FuncWrapper&& other) {
        impl = std::move(other.impl);
        return *this;
    }

    /** Whether it is an empty task, used by the thread pool to judge whether to terminate the run
     *  after all the tasks are finished */
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
        // cppcheck-suppress noExplicitConstructor ; the explicit modifier cannot be added here, the
        // conversion copy is needed
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
