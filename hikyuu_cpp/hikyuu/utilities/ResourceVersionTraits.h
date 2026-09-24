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
 * @brief Detection utility of the resource version interface traits
 * @details It provides a compile-time type check, used to verify whether the resource type supports
 *          the version management interfaces
 * @ingroup Utilities
 */
namespace detail {

/**
 * @brief Detect whether the resource type has a getVersion() method
 * @tparam T the resource type to be detected
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
 * @brief Detect whether the resource type has a setVersion(int) method
 * @tparam T the resource type to be detected
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
 * @brief A convenient alias of the getVersion detection
 * @tparam T the resource type to be detected
 */
template <typename T>
static constexpr bool has_resource_getVersion_v = has_resource_getVersion<T>::value;

/**
 * @brief A convenient alias of the setVersion detection
 * @tparam T the resource type to be detected
 */
template <typename T>
static constexpr bool has_resource_setVersion_v = has_resource_setVersion<T>::value;

}  // namespace detail

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_VERSION_TRAITS_H */
