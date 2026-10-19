/*
 * test_IndicatorImp_serialization.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-19
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cstring>
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/EXP.h>
#include <hikyuu/indicator/crt/LOG.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/POW.h>
#include <hikyuu/indicator/crt/SLOPE.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace hku;

/**
 * @defgroup test_indicator_serialization test_indicator_serialization
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

namespace {

using value_t = Indicator::value_t;

template <typename OArchive, typename IArchive>
Indicator roundTrip(const Indicator& src) {
    std::ostringstream os;
    {
        OArchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(src);
    }

    Indicator dst;
    std::istringstream is(os.str());
    {
        IArchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(dst);
    }
    return dst;
}

/** Two NaNs match whatever their payload is, everything else must match bitwise (the sign of
 *  zero and the exact infinity are part of the value) */
bool sameBits(value_t a, value_t b) {
    if (std::isnan(a) || std::isnan(b)) {
        return std::isnan(a) && std::isnan(b);
    }
    return std::memcmp(&a, &b, sizeof(value_t)) == 0;
}

void checkRoundTrip(const Indicator& src, const Indicator& dst) {
    REQUIRE_EQ(dst.size(), src.size());
    REQUIRE_EQ(dst.getResultNumber(), src.getResultNumber());
    REQUIRE_EQ(dst.discard(), src.discard());
    for (size_t r = 0; r < src.getResultNumber(); ++r) {
        for (size_t i = 0; i < src.size(); ++i) {
            INFO("result " << r << ", pos " << i);
            CHECK_UNARY(sameBits(src.get(i, r), dst.get(i, r)));
        }
    }
}

// The legacy string format kept 6 decimal places, so every value below the threshold was
// corrupted on a round trip; they are the regression values of this suite
// @param includeSubnormal the smallest subnormal cannot be read back from a text archive once it
//        is embedded in a vector<double> parameter (libc++ reports ERANGE for a subnormal
//        conversion, which makes the boost text archive fail), so the text cases build it through
//        a computation instead of a parameter
PriceList specialValues(bool includeSubnormal = true) {
    PriceList px;
    px.push_back(1e-7);
    px.push_back(1e-12);
    px.push_back(2.5e-6);           // the legacy format rounded it to 3e-6
    px.push_back(1.234567890123);   // 16 significant digits
    px.push_back(-0.0);             // the sign bit must survive
    if (includeSubnormal) {
        px.push_back(5e-324);  // the smallest subnormal
    }
    px.push_back(std::numeric_limits<value_t>::max());
    px.push_back(0.0);
    px.push_back(-1.0);
    return px;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_indicator_serialization_value_precision") {
    const value_t inf = std::numeric_limits<value_t>::infinity();
    PriceList px = specialValues();
    px.push_back(Null<value_t>());  // NaN
    px.push_back(inf);
    px.push_back(-inf);

    Indicator x = PRICELIST(px);
    REQUIRE_EQ(x.size(), px.size());

    /** @arg the binary archive (the one python pickle uses) keeps every bit */
    checkRoundTrip(x, roundTrip<boost::archive::binary_oarchive, boost::archive::binary_iarchive>(
                        x));
}

/** @par Test points */
TEST_CASE("test_indicator_serialization_value_precision_text") {
    // A text archive cannot parse "nan" or "inf" back, so the bit patterns are archived as
    // unsigned integers; the values below reach the buffers through computations whose parameters
    // stay representable (a non-finite or a subnormal value inside a PRICELIST parameter would hit
    // the pre-existing boost limitation of vector<double> in text archives, which is out of the
    // scope of the buffer format)
    Indicator finite = PRICELIST(specialValues(false));
    Indicator with_subnormal = POW(PRICELIST(PriceList{2.0}), -1074);  // 2^-1074, the smallest
                                                                       // subnormal
    Indicator with_nan = LOG(PRICELIST(PriceList{-1.0, 2.0, -3.0}));       // NaN, ln2, NaN
    Indicator with_inf = EXP(PRICELIST(PriceList{1000.0, 1.0}));           // +Inf, e
    Indicator with_ninf = LOG(PRICELIST(PriceList{0.0, 1.0}));             // -Inf, 0

    for (const Indicator* x : {&finite, &with_subnormal, &with_nan, &with_inf, &with_ninf}) {
        /** @arg the text archive keeps every bit of the result buffers, NaN and Inf included */
        checkRoundTrip(*x, roundTrip<boost::archive::text_oarchive,
                                      boost::archive::text_iarchive>(*x));
    }
}

/** @par Test points */
TEST_CASE("test_indicator_serialization_calculated_values") {
    PriceList px;
    for (size_t i = 0; i < 30; ++i) {
        px.push_back(10.0 + i * 0.5);
    }

    Indicator ma = MA(PRICELIST(px), 5);
    Indicator slope = SLOPE(PRICELIST(px), 4);  // three results

    /** @arg a calculated indicator round trips bitwise, its NaN discard prefix included */
    checkRoundTrip(ma, roundTrip<boost::archive::binary_oarchive, boost::archive::binary_iarchive>(
                         ma));
    checkRoundTrip(ma, roundTrip<boost::archive::text_oarchive, boost::archive::text_iarchive>(ma));

    /** @arg every result set of a multi-result indicator round trips bitwise */
    checkRoundTrip(slope, roundTrip<boost::archive::binary_oarchive, boost::archive::binary_iarchive>(
                            slope));
    checkRoundTrip(slope,
                   roundTrip<boost::archive::text_oarchive, boost::archive::text_iarchive>(slope));
}

/** @par Test points */
TEST_CASE("test_indicator_serialization_nan_in_series") {
    // NaN and Inf inside the series (not only in the discard prefix)
    PriceList px;
    for (size_t i = 0; i < 30; ++i) {
        px.push_back(10.0 + i * 0.5);
    }
    px[10] = Null<value_t>();
    px[20] = std::numeric_limits<value_t>::infinity();

    Indicator ma = MA(PRICELIST(px), 5);
    Indicator slope = SLOPE(PRICELIST(px), 4);

    /** @arg the binary archive keeps every bit of a series holding NaN and Inf */
    checkRoundTrip(ma, roundTrip<boost::archive::binary_oarchive, boost::archive::binary_iarchive>(
                         ma));
    checkRoundTrip(slope, roundTrip<boost::archive::binary_oarchive, boost::archive::binary_iarchive>(
                            slope));
}

/** @par Test points */
TEST_CASE("test_indicator_serialization_volume") {
    // The legacy format stored every value as a string plus one nvp node per value, which cost
    // about 40 bytes per value; the raw bit pattern costs 8 bytes per value
    PriceList px;
    for (size_t i = 0; i < 2500; ++i) {
        px.push_back(double(i));
    }
    Indicator x = PRICELIST(px);

    std::ostringstream os;
    {
        boost::archive::binary_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(x);
    }
    const size_t archive_size = os.str().size();
    const size_t raw_size = 2500 * sizeof(value_t);

    /** @arg the archive stays close to the raw data size instead of the legacy string size */
    CHECK_LT(archive_size, raw_size * 3);
    HKU_INFO("serialized size: {} bytes (raw data: {} bytes)", archive_size, raw_size);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
