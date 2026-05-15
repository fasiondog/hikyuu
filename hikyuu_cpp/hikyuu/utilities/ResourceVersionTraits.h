/*
 * ResourceVersionTraits.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-05-13
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_RESOURCE_VERSION_TRAITS_H
#define HKU_UTILS_RESOURCE_VERSION_TRAITS_H

#include <type_traits>

namespace hku {

/**
 * @brief 资源版本接口特征检测工具
 * @details 提供编译期类型检查，用于验证资源类型是否支持版本管理接口
 * @ingroup Utilities
 */
namespace detail {

/**
 * @brief 检测资源类型是否具有 getVersion() 方法
 * @tparam T 待检测的资源类型
 */
template <typename T>
struct has_resource_getVersion {
    template <typename U>
    static auto test(int) -> decltype(std::declval<U>().getVersion(), std::true_type{});
    template <typename>
    static std::false_type test(...);
    static constexpr bool value = decltype(test<T>(0))::value;
};

/**
 * @brief 检测资源类型是否具有 setVersion(int) 方法
 * @tparam T 待检测的资源类型
 */
template <typename T>
struct has_resource_setVersion {
    template <typename U>
    static auto test(int)
      -> decltype(std::declval<U>().setVersion(std::declval<int>()), std::true_type{});
    template <typename>
    static std::false_type test(...);
    static constexpr bool value = decltype(test<T>(0))::value;
};

/**
 * @brief getVersion 检测的便捷别名
 * @tparam T 待检测的资源类型
 */
template <typename T>
static constexpr bool has_resource_getVersion_v = has_resource_getVersion<T>::value;

/**
 * @brief setVersion 检测的便捷别名
 * @tparam T 待检测的资源类型
 */
template <typename T>
static constexpr bool has_resource_setVersion_v = has_resource_setVersion<T>::value;

}  // namespace detail

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_VERSION_TRAITS_H */
