/*
 * test_sys.h
 *
 *  Created on: 2018年1月20日
 *      Author: fasiondog
 */

#ifndef LIBS_HIKYUU_TRADE_SYS_SYSTEM_TEST_SYS_H_
#define LIBS_HIKYUU_TRADE_SYS_SYSTEM_TEST_SYS_H_

#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>

using namespace hku;

class TestEV1: public EnvironmentBase {
public:
    TestEV1(): EnvironmentBase("TEST_EV1") {};
    virtual ~TestEV1() {}

    virtual EnvironmentPtr _clone() {
        return make_shared<TestEV1>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200001040000LL));
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
        _addValid(Datetime(200001070000LL));
    };
};

class TestEV2: public EnvironmentBase {
public:
    TestEV2(): EnvironmentBase("TEST_EV2") {};
    virtual ~TestEV2() {}

    virtual EnvironmentPtr _clone() {
        return make_shared<TestEV2>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
        _addValid(Datetime(200001070000LL));
        _addValid(Datetime(200001100000LL));
        _addValid(Datetime(200001110000LL));
        _addValid(Datetime(200001120000LL));
        _addValid(Datetime(200001130000LL));
        _addValid(Datetime(200001140000LL));
    };
};

class TestEV3: public EnvironmentBase {
public:
    TestEV3(): EnvironmentBase("TEST_EV3") {};
    virtual ~TestEV3() {}

    virtual EnvironmentPtr _clone() {
        return make_shared<TestEV3>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
    };
};

class TestCN1: public ConditionBase {
public:
    TestCN1(): ConditionBase("Test_CN1") {}
    virtual ~TestCN1() {}

    virtual ConditionPtr _clone() {
        return make_shared<TestCN1>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200001040000LL));
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
        _addValid(Datetime(200001070000LL));
    }
};

class TestCN2: public ConditionBase {
public:
    TestCN2(): ConditionBase("Test_CN2") {}
    virtual ~TestCN2() {}

    virtual ConditionPtr _clone() {
        return make_shared<TestCN2>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200012300000LL));
        _addValid(Datetime(200001040000LL));
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
        _addValid(Datetime(200001070000LL));
        _addValid(Datetime(200001100000LL));
    }
};

class TestCN3: public ConditionBase {
public:
    TestCN3(): ConditionBase("Test_CN3") {}
    virtual ~TestCN3() {}

    virtual ConditionPtr _clone() {
        return make_shared<TestCN3>();
    }

    virtual void _calculate() {
        _addValid(Datetime(200001050000LL));
        _addValid(Datetime(200001060000LL));
    }
};



#endif /* LIBS_HIKYUU_TRADE_SYS_SYSTEM_TEST_SYS_H_ */
