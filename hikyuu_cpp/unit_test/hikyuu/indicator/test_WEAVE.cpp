#include "../test_config.h"

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_weave test_indicator_serialization
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_WAEVE_export") {
    KData k = getKData("sh000001", KQuery(-20));
    auto c = k.close();
    auto h = k.high();
    auto o = k.open();
    auto l = k.low();
    auto v = k.vol();
    auto a = k.amo();

    auto ret = WEAVE(o, h, l, c);
    CHECK_EQ(ret.name(), "WEAVE");
    CHECK_EQ(ret.size(), k.size());
    CHECK_EQ(ret.getResultNumber(), 4);
    CHECK_EQ(ret.discard(), 0);
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_EQ(ret.get(i, 0), o[i]);
        CHECK_EQ(ret.get(i, 1), h[i]);
        CHECK_EQ(ret.get(i, 2), l[i]);
        CHECK_EQ(ret.get(i, 3), c[i]);
    }

    ret = WEAVE(o, c, h, l, v, a);
    CHECK_EQ(ret.getResultNumber(), 6);
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_EQ(ret.get(i, 0), o[i]);
        CHECK_EQ(ret.get(i, 1), c[i]);
        CHECK_EQ(ret.get(i, 2), h[i]);
        CHECK_EQ(ret.get(i, 3), l[i]);
        CHECK_EQ(ret.get(i, 4), v[i]);
        CHECK_EQ(ret.get(i, 5), a[i]);
    }

    /** @arg 超出参数个数 */
    CHECK_THROWS(WEAVE(c, c, c, c, c, c, c));
}

#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点 */
TEST_CASE("test_WAEVE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/WEAVE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    Indicator w1 = WEAVE(c, o);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(w1);
    }

    Indicator w2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(w2);
    }

    CHECK_EQ(w1.size(), w2.size());
    CHECK_EQ(w1.discard(), w2.discard());
    CHECK_EQ(w1.getResultNumber(), 2);
    CHECK_EQ(w2.getResultNumber(), 2);
    size_t total = w1.size();
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(w1[i], doctest::Approx(w2[i]).epsilon(0.00001));
    }
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(w1.get(i, 1), doctest::Approx(w2.get(i, 1)).epsilon(0.00001));
    }
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
