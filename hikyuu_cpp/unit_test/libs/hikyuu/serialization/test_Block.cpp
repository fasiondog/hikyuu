/*
 * test_Block.cpp
 *
 *  Created on: 2015年2月9日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_serialize_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/serialization/Block_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_Block_serialize test_Block_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Block_serialize ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/Block.xml";

    Block blk1("test", "test_1");
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(blk1);
    }

    Block blk2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(blk2);
    }

    BOOST_CHECK(blk1.category() == blk2.category());
    BOOST_CHECK(blk1.name() == blk2.name());
    BOOST_CHECK(blk1.size() == blk2.size());

    blk1.add("sh000001");
    blk1.add("sz000001");
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(blk1);
    }
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(blk2);
    }
    BOOST_CHECK(blk1.category() == blk2.category());
    BOOST_CHECK(blk1.name() == blk2.name());
    BOOST_CHECK(blk1.size() == blk2.size());
    BOOST_CHECK(blk2.have("sh000001"));
    BOOST_CHECK(blk2.have("sz000001"));
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */

