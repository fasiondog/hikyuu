/*
 * test_util.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/arithmetic.h>
#include "hikyuu/utilities/Log.h"

using namespace hku;

/**
 * @defgroup test_hikyuu_arithmetic test_hikyuu_arithmetic
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par Test points */
TEST_CASE("test_round") {
    double x;

    x = 10.11;
    CHECK(roundEx(x) == 10.0);
    CHECK(roundDown(x) == 10.0);
    CHECK(roundUp(x) == 11.0);
    CHECK_EQ(roundEx(x, 1), doctest::Approx(10.1));
    CHECK(roundDown(x, 1) == 10.1);
    CHECK(roundUp(x, 1) == 10.2);

    x = 10.55;
    CHECK(roundEx(x) == 11);
    CHECK(roundDown(x) == 10);
    CHECK(roundUp(x) == 11.0);
    CHECK_EQ(roundEx(x, 1), doctest::Approx(10.6));
    CHECK(roundDown(x, 1) == 10.5);
    CHECK(roundUp(x, 1) == 10.6);

    x = -10.11;
    CHECK(roundEx(x) == -10);
    CHECK(roundDown(x) == -10);
    CHECK(roundUp(x) == -11.0);
    CHECK_EQ(roundEx(x, 1), doctest::Approx(-10.1));
    CHECK(roundDown(x, 1) == -10.1);
    CHECK(roundUp(x, 1) == -10.2);

    x = -10.55;
    CHECK(roundEx(x) == -11);
    CHECK(roundDown(x) == -10);
    CHECK(roundUp(x) == -11.0);
    CHECK_EQ(roundEx(x, 1), doctest::Approx(-10.6));
    CHECK(roundDown(x, 1) == -10.5);
    CHECK_EQ(roundUp(x, 1), doctest::Approx(-10.6));
}

/** @par Test points */
TEST_CASE("test_roundEx_large_ndigits") {
    // Regression: the epsilon used to be 1e-10 * factor, which reaches 1.0 at ndigits >=
    // 10 and swamps the 0.5 rounding threshold, so a value already within the requested precision
    // got a systematic upward bias. After capping, large ndigits are a no-op for such values.
    /** @arg ndigits beyond the value's own precision returns it unchanged (was 1.0000000001) */
    CHECK_EQ(roundEx(1.0, 10), 1.0);
    CHECK_EQ(roundEx(0.5, 10), 0.5);
    CHECK_EQ(roundEx(-1.0, 10), -1.0);
    CHECK_EQ(roundEx(123.456, 10), doctest::Approx(123.456));
    /** @arg very large ndigits stay correct (no threshold swamping) */
    CHECK_EQ(roundEx(1.0, 15), 1.0);

    /** @arg the representation-recovery nudge is preserved for small ndigits: 2.675 -> 2.68 */
    CHECK_EQ(roundEx(2.675, 2), doctest::Approx(2.68));
    /** @arg ordinary money-scale rounding is unchanged */
    CHECK_EQ(roundEx(10.11, 2), doctest::Approx(10.11));
    CHECK_EQ(roundEx(2.345, 2), doctest::Approx(2.35));
}

/** @par Test points */
TEST_CASE("test_roundEx_negative_ndigits") {
    // A negative ndigits rounds half away from zero to the left of the decimal point (to the
    // 10^|ndigits| place), matching roundUp / roundDown; it used to be rejected and returned as-is.
    /** @arg round to hundreds */
    CHECK_EQ(roundEx(1234.0, -2), 1200.0);
    CHECK_EQ(roundEx(1499.0, -2), 1500.0);
    CHECK_EQ(roundEx(1234.567, -2), 1200.0);
    /** @arg exactly half rounds away from zero (both signs) */
    CHECK_EQ(roundEx(1250.0, -2), 1300.0);
    CHECK_EQ(roundEx(-1250.0, -2), -1300.0);
    CHECK_EQ(roundEx(-1234.0, -2), -1200.0);
    /** @arg round to tens */
    CHECK_EQ(roundEx(15.0, -1), 20.0);
    CHECK_EQ(roundEx(-25.0, -1), -30.0);
    /** @arg zero is unchanged */
    CHECK_EQ(roundEx(0.0, -2), 0.0);
    /** @arg the positive path is unaffected by the negative branch */
    CHECK_EQ(roundEx(1234.0, 2), 1234.0);
    /** @arg the divide-branch tolerance is a few ULPs of the quotient, not scaled by factor:
     *  values far below the rounding boundary stay put (they used to be pushed past it) */
    CHECK_EQ(roundEx(49999.99, -5), 0.0);
    CHECK_EQ(roundEx(4999.999, -4), 0.0);
    CHECK_EQ(roundEx(24999999999.0, -10), 2e10);
    /** @arg the magnitude is clamped, so extreme ndigits (including INT_MIN) stay well-defined */
    CHECK_EQ(roundEx(1.0, 1000000), 1.0);
    CHECK_EQ(roundEx(1.0, std::numeric_limits<int>::min()), 0.0);
}

TEST_CASE("test_string_to_upper") {
    std::string x("abcd");
    to_upper(x);
    CHECK(x == "ABCD");

    std::string y("中abcdD");
    to_upper(y);
    CHECK(y == "中ABCDD");
}

TEST_CASE("test_string_to_lower") {
    std::string x("ABcD");
    to_lower(x);
    CHECK(x == "abcd");

    std::string y("中abCdD");
    to_lower(y);
    CHECK(y == "中abcdd");
}

TEST_CASE("test_byteToHexStr") {
    const char *x = "abcd";
    std::string hex = byteToHexStr(x, 4);
    CHECK_EQ(hex, "61626364");

    std::string y(x);
    hex = byteToHexStr(y);
    CHECK_EQ(hex, "61626364");

    CHECK_EQ("", byteToHexStr(""));
}

TEST_CASE("test_byteToHexStrForPrint") {
    const char *x = "abcd";
    std::string hex = byteToHexStrForPrint(x, 4);
    CHECK_EQ(hex, "0x61 0x62 0x63 0x64");

    CHECK_EQ("", byteToHexStrForPrint(""));
}

TEST_CASE("test_split_by_char") {
    std::string x("");
    auto splits = split(x, '.');
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    x = "100.1.";
    splits = split(x, '.');
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], "1");

    x = "..";
    splits = split(x, '.');
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "");
    CHECK_EQ(splits[1], "");
    CHECK_EQ(splits[2], "");
}

TEST_CASE("test_split_by_string") {
    std::string x("");

    // The split string is empty
    auto splits = split(x, "");
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    x = "123";
    splits = split(x, "");
    CHECK_EQ(splits.size(), 1);
    CHECK_EQ(splits[0], x);

    // The split string length is 1
    x = "100.1.";
    splits = split(x, ".");
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], "1");
    CHECK_EQ(splits[2], "");

    // The split string length is 2
    x = "100.1.234.1.56";
    splits = split(x, ".1");
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "100");
    CHECK_EQ(splits[1], ".234");
    CHECK_EQ(splits[2], ".56");

    x = "..";
    splits = split(x, ".");
    CHECK_EQ(splits.size(), 3);
    CHECK_EQ(splits[0], "");
    CHECK_EQ(splits[1], "");
    CHECK_EQ(splits[2], "");
}

/** @par Test points */
TEST_CASE("test_nearly_equal") {
    using limits = std::numeric_limits<double>;

    /** @arg Bitwise equal values match, incl. +0 == -0 */
    CHECK_UNARY(nearly_equal(1.0, 1.0));
    CHECK_UNARY(nearly_equal(0.0, -0.0));

    /** @arg NaN never matches, even itself */
    double nan = limits::quiet_NaN();
    CHECK_UNARY(!nearly_equal(nan, nan));
    CHECK_UNARY(!nearly_equal(nan, 1.0));

    /** @arg Infinities match only via == (same sign); opposite signs or inf-vs-largest-finite do
     *  not, otherwise the tolerance band would be inf */
    double inf = limits::infinity();
    CHECK_UNARY(nearly_equal(inf, inf));
    CHECK_UNARY(nearly_equal(-inf, -inf));
    CHECK_UNARY(!nearly_equal(inf, -inf));
    CHECK_UNARY(!nearly_equal(inf, limits::max()));

    /** @arg One ULP apart matches at price and large magnitudes (an absolute 1e-9 epsilon would
     *  miss the 1e20 case) */
    CHECK_UNARY(nearly_equal(100.0, std::nextafter(100.0, 101.0)));
    CHECK_UNARY(nearly_equal(1e20, std::nextafter(1e20, 1e21)));

    /** @arg A tiny non-zero value does not falsely match 0 (relative scale of 0 is 0) */
    CHECK_UNARY(!nearly_equal(0.0, 1e-20));

    /** @arg Clearly different values never match */
    CHECK_UNARY(!nearly_equal(1.0, 2.0));

    /** @arg Float version behaves the same */
    CHECK_UNARY(nearly_equal(100.0f, std::nextafter(100.0f, 101.0f)));
    CHECK_UNARY(!nearly_equal(1.0f, 2.0f));
}

/** @par Test points */
TEST_CASE("test_safe_math") {
    using limits = std::numeric_limits<double>;

    /** @arg A finite value passes through unchanged */
    CHECK_EQ(safe_math(1.5), 1.5);
    CHECK_EQ(safe_math(-3.0), -3.0);

    /** @arg NaN passes through as NaN (framework null already uses NaN) */
    CHECK_UNARY(std::isnan(safe_math(limits::quiet_NaN())));

    /** @arg +-Inf is folded to Null (NaN) so it does not leak downstream */
    CHECK_UNARY(std::isnan(safe_math(limits::infinity())));
    CHECK_UNARY(std::isnan(safe_math(-limits::infinity())));

    /** @arg Zero and -0 are finite and pass through (not folded to Null) */
    CHECK_EQ(safe_math(0.0), 0.0);
    CHECK_UNARY(!std::isnan(safe_math(-0.0)));
}

/** @par Test points */
TEST_CASE("test_truncToMultiple") {
    /** @arg ordinary truncation to the lot multiple (toward zero) */
    CHECK_EQ(truncToMultiple(333.0, 100.0), 300.0);
    CHECK_EQ(truncToMultiple(200.0, 100.0), 200.0);
    CHECK_EQ(truncToMultiple(99.0, 100.0), 0.0);
    CHECK_EQ(truncToMultiple(0.5, 100.0), 0.0);
    CHECK_EQ(truncToMultiple(-250.0, 100.0), -200.0);

    /** @arg a quotient beyond the int64 range returns NaN, so the caller rejects the order */
    CHECK_UNARY(std::isnan(truncToMultiple(1e30, 100.0)));
    CHECK_UNARY(std::isnan(truncToMultiple(std::numeric_limits<double>::infinity(), 100.0)));

    /** @arg a NaN input returns NaN */
    CHECK_UNARY(std::isnan(truncToMultiple(std::numeric_limits<double>::quiet_NaN(), 100.0)));
}

/** @} */
