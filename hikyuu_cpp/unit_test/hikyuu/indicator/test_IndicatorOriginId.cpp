/*
 * test_IndicatorOriginId.cpp
 *
 *  Created on: 2026-08-12
 *      Author: woleigegg
 *
 *  A dedicated test of origin_id (the construction origin identifier):
 *  it is unique at the construction, inherited by clone and renewed by the deserialization load.
 *  Note: the inheritance in CompiledFactorPlan::cloneNode (the same-line copy semantics) is a
 *  private static path with no public probe, covered by the code review; here the publicly
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_indicator_OriginId test_indicator_OriginId
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

static uint64_t origin_id_of(const Indicator& ind) {
    return ind.getImp()->originId();
}

/** @par Test point: independent constructions always differ (the basis for eradicating the formula
 * collision bugs) */
TEST_CASE("test_IndicatorOriginId_construct_unique") {
    Indicator c = CLOSE(), o = OPEN(), h = HIGH();

    // The same structure with different parameters: the whole representative family of the B1 bug
    CHECK_NE(origin_id_of(MA(c, 20)), origin_id_of(MA(c, 60)));
    CHECK_NE(origin_id_of(REF(c, 1)), origin_id_of(REF(c, 5)));
    CHECK_NE(origin_id_of(CVAL(1.0)), origin_id_of(CVAL(2.0)));
    CHECK_NE(origin_id_of(CVAL(1.0)), origin_id_of(CVAL(1.0)));  // Same param, different origin id

    // Different structures: a bracket ambiguity and the same shape with a different operator
    CHECK_NE(origin_id_of((c + o) * h), origin_id_of(c + o * h));
    CHECK_NE(origin_id_of(c + o), origin_id_of(c - o));

    // When the same leaf object is shared by two trees (a diamond) their root nodes stay
    // independent
    Indicator shared = c + o;
    CHECK_NE(origin_id_of(shared * h), origin_id_of(shared + h));
}

/** @par Test point: the clone inheritance (the premise of the cache hit in the batch paths) */
TEST_CASE("test_IndicatorOriginId_clone_inherit") {
    Indicator c = CLOSE(), o = OPEN(), h = HIGH();

    Indicator f = MA(c, 20) + CVAL(1.0);
    Indicator g = f.clone();
    CHECK_EQ(origin_id_of(f), origin_id_of(g));

    // The whole tree inherits node by node: the root, the subtrees and the data leaf of the OP node
    auto fp = f.getImp(), gp = g.getImp();
    CHECK_EQ(fp->getLeftNode()->originId(), gp->getLeftNode()->originId());    // The MA subtree
    CHECK_EQ(fp->getRightNode()->originId(), gp->getRightNode()->originId());  // The CVAL subtree
    auto ma_l = fp->getLeftNode(), ma_g = gp->getLeftNode();
    CHECK_EQ(ma_l->getRightNode()->originId(), ma_g->getRightNode()->originId());  // CLOSE 叶

    // The clone of a clone has the same id (the clone chain)
    Indicator e = g.clone();
    CHECK_EQ(origin_id_of(f), origin_id_of(e));

    // After the clone it still differs from an independent construction
    CHECK_NE(origin_id_of(g), origin_id_of(MA(c, 20) + CVAL(1.0)));
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test point: the deserialization gets a new id (origin_id is not archived and load goes
 * through the default construction) */
TEST_CASE("test_IndicatorOriginId_serialize_fresh") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ORIGIN_ID.xml";

    Indicator x1 = MA(CLOSE(), 20);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    // The loaded node holds a new id of this process: it differs from the original object and from
    // a newly constructed object with the same formula
    CHECK_NE(origin_id_of(x2), origin_id_of(x1));
    CHECK_NE(origin_id_of(x2), origin_id_of(MA(CLOSE(), 20)));

    // Loading the same archive again issues another new id, no sharing (a safe direction)
    Indicator x3;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x3);
    }
    CHECK_NE(origin_id_of(x3), origin_id_of(x2));
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
