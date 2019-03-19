/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <stdexcept>
#include "Indicator.h"
#include "../Stock.h"
#include "../Context.h"
#include "../Log.h"

namespace hku {


HKU_API std::ostream & operator<<(std::ostream& os, const IndicatorImp& imp) {
    os << "Indicator(" << imp.name() << ", " << imp.getParameter() << ")";
    return os;
}


HKU_API std::ostream & operator<<(std::ostream& os, const IndicatorImpPtr& imp) {
    if (!imp) {
        os << "Indicator(NULL)";
    } else {
        os << *imp;
    }
    return os;
}

IndicatorImp::IndicatorImp()
: m_name("IndicatorImp"), m_discard(0), m_result_num(0), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name)
: m_name(name), m_discard(0), m_result_num(0), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name, size_t result_num)
: m_name(name), m_discard(0), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
    m_result_num = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
}

void IndicatorImp::initContext() {
    setParam<KData>("kdata", KData());
}

void IndicatorImp::setContext(const Stock& stock, const KQuery& query) {
    //子节点设置上下文
    if (m_left) m_left->setContext(stock, query);
    if (m_right) m_right->setContext(stock, query);

    //如果该节点依赖上下文
    if (isNeedContext()) {
        //如果上下文有变化则重设上下文
        KData kdata = getCurrentKData();
        if (kdata.getStock() != stock || kdata.getQuery() != query) {
            setParam<KData>("kdata", stock.getKData(query));        
        }
    }

    //启动重新计算
    calculate();
}

KData IndicatorImp::getCurrentKData() {
    KData kdata = getParam<KData>("kdata");
    if (kdata.getStock().isNull()) {
        kdata = getGlobalContextKData();
    }

    return kdata;
}

void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
    if (result_num > MAX_RESULT_NUM) {
        throw(std::invalid_argument("result_num oiverload MAX_RESULT_NUM! "
                "[IndicatorImp::_readyBuffer]" + name()));
        return;
    }

    price_t null_price = Null<price_t>();

    for (size_t i = 0; i < result_num; ++i) {
        if (!m_pBuffer[i]) {
            m_pBuffer[i] = new PriceList(len, null_price);

        } else {
            m_pBuffer[i]->clear();
            m_pBuffer[i]->reserve(len);
            for (size_t j = 0; j < len; ++j) {
                m_pBuffer[i]->push_back(null_price);
            }
        }
    }

    for (size_t i = result_num; i < m_result_num; ++i) {
        delete m_pBuffer[i];
        m_pBuffer[i] = NULL;
    }

    m_result_num = result_num;
}

IndicatorImp::~IndicatorImp() {
    for (size_t i = 0; i < m_result_num; ++i) {
        delete m_pBuffer[i];
    }
}

IndicatorImpPtr IndicatorImp::clone() {
    IndicatorImpPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_discard = m_discard;
    p->m_result_num = m_result_num;
    p->m_optype = m_optype;
    if (m_left) {
        p->m_left = m_left->clone();
    }
    if (m_right) {
        p->m_right = m_right->clone();
    }
    return p;
}


IndicatorImpPtr IndicatorImp::operator()(const Indicator& ind) {
    HKU_INFO("This indicator not support operator()! " << *this
            << " [IndicatorImp::operator()]");
    //保证对齐
    IndicatorImpPtr result = make_shared<IndicatorImp>();
    size_t total = ind.size();
    result->_readyBuffer(total, m_result_num);
    result->setDiscard(total);
    return result;
}

void IndicatorImp::setDiscard(size_t discard) {
    size_t tmp_discard = discard > size() ? size() : discard;
    if (tmp_discard > m_discard) {
        price_t null_price = Null<price_t>();
        for (size_t i = 0; i < m_result_num; ++i) {
            for (size_t j = m_discard; j < tmp_discard; ++j) {
                _set(null_price, j, i);
            }
        }
        m_discard = tmp_discard;
    }
}

string IndicatorImp::long_name() const {
    return name() + "(" + m_params.getNameValueList() + ")";
}

PriceList IndicatorImp::getResultAsPriceList(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return PriceList();
    }

    return (*m_pBuffer[result_num]);
}


IndicatorImpPtr IndicatorImp::getResult(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return IndicatorImpPtr();
    }

    IndicatorImpPtr imp = make_shared<IndicatorImp>();
    size_t total = size();
    imp->_readyBuffer(total, 1);
    imp->setDiscard(discard());
    for (size_t i = discard(); i < total; ++i) {
        imp->_set(get(i, result_num), i);
    }
    return imp;
}


string IndicatorImp::formula() const {
    std::stringstream buf;

    switch (m_optype) {
        case LEAF:
            buf << m_name;
            break;

        case OP:
            buf << m_name << "("  << m_right->formula() << ")";
            break;

        case ADD:
            buf << m_left->formula() << " + " << m_right->formula();
            break;

        case SUB:
            buf << m_left->formula() << " + " << m_right->formula();
            break;

        case MUL:
            buf << m_left->formula() << " * " << m_right->formula();
            break;

        case DIV:
            buf << m_left->formula() << " / " << m_right->formula();
            break;

        case EQ:
            buf << m_left->formula() << " == " << m_right->formula();
            break;

        case GT:
            buf << m_left->formula() << " > " << m_right->formula();
            break;

        case LT:
            buf << m_left->formula() << " < " << m_right->formula();
            break;

        case NE:
            buf << m_left->formula() << " != " << m_right->formula();
            break;

        case GE:
            buf << m_left->formula() << " >= " << m_right->formula();
            break;

        case LE:
            buf << m_left->formula() << " <= " << m_right->formula();
            break;

        case AND:
            buf << m_left->formula() << " & " << m_right->formula();
            break;

        case OR:
            buf << m_left->formula() << " | " << m_right->formula();
            break;

        default:
            HKU_ERROR("Wrong optype!" << m_optype << " [IndicatorImp::formula]");
            break;
    }

    return buf.str();
}


IndicatorImpPtr IndicatorImp::getSameNameNeedContextLeaf(const string& name) {
    if (isNeedContext() && m_name == name) {
        return shared_from_this();
    }

    IndicatorImpPtr p;
    if (m_left) {
        p = m_left->getSameNameNeedContextLeaf(name);
        if (p) {
            return p;
        }
    }

    if (m_right) {
        p = m_right->getSameNameNeedContextLeaf(name);
    }

    return p;
}


void IndicatorImp::add(OPType op, IndicatorImpPtr left, IndicatorImpPtr right) {
    if (op == LEAF || op >= INVALID || !right) {
        HKU_ERROR("Wrong used [IndicatorImp::add]");
        return;
    }

    if (op == LEAF && left && left->isNeedContext()) {
        HKU_ERROR("Syntax error! [IndicatorImp::add]");
        return;
    }

    IndicatorImpPtr new_right;
    if (right->isNeedContext()) {
        new_right = right->getSameNameNeedContextLeaf(right->name());
    }

    m_optype = op;
    m_left = left;
    m_right = new_right ? new_right : right;
}

Indicator IndicatorImp::calculate() {
    if (!check()) {
        HKU_WARN("Invalid param! " << formula() << " : " << long_name());
        if (m_right) {
            m_right->calculate();
            _readyBuffer(m_right->size(), m_result_num);
            m_discard = m_right->size();
            return shared_from_this();
        } else {
            return Indicator();
        }
    }

    switch (m_optype) {
        case LEAF:
            _calculate(Indicator());
            break;

        case OP:
            m_right->calculate();
            _readyBuffer(m_right->size(), m_result_num);
            _calculate(Indicator(m_right));
            break;

        case ADD:
            execute_add();
            break; 

        case SUB:
            execute_sub();
            break;

        case MUL:
            execute_mul();
            break;

        case DIV:
            execute_div();
            break;

        case EQ:
            execute_eq();
            break;

        case NE:
            execute_ne();
            break;

        case GT:
            execute_gt();
            break;

        case LT:
            execute_lt();
            break;

        case GE:
            execute_ge();
            break;

        case LE:
            execute_le();
            break;

        case AND:
            execute_and();
            break;

        case OR:
            execute_or();
            break;

        default:
            break;
    }

    return shared_from_this();
}

void IndicatorImp::execute_add() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            _set(maxp->get(i, r) + minp->get(i-diff, r), i, r);
        }
    }
}

void IndicatorImp::execute_sub() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                _set(m_left->get(i, r) - m_right->get(i-diff, r), i, r);
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                _set(m_left->get(i-diff, r) - m_right->get(i, r), i, r);
            }
        } 
    }
}

void IndicatorImp::execute_mul() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            _set(maxp->get(i, r) * minp->get(i-diff, r), i, r);
        }
    }
}

void IndicatorImp::execute_div() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_right->get(i-diff, r) == 0.0) {
                    _set(Null<price_t>(), i, r);    
                } else {
                    _set(m_left->get(i, r) / m_right->get(i-diff, r), i, r);
                }
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_right->get(i, r) == 0.0) {
                    _set(Null<price_t>(), i, r); 
                } else {
                    _set(m_left->get(i-diff, r) / m_right->get(i, r), i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_eq() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(maxp->get(i, r) - minp->get(i-diff, r)) < IND_EQ_THRESHOLD) {
                _set(1, i, r);
            } else {
                _set(0, i, r);
            }
        }
    }
}

void IndicatorImp::execute_ne() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(maxp->get(i, r) - minp->get(i-diff, r)) < IND_EQ_THRESHOLD) {
                _set(0, i, r);
            } else {
                _set(1, i, r);
            }
        }
    }
}

void IndicatorImp::execute_gt() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i, r) - m_right->get(i-diff, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i-diff, r) - m_right->get(i, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_lt() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_right->get(i-diff, r) - m_left->get(i, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_right->get(i, r) - m_left->get(i-diff, r)>= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_ge() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i, r) > m_right->get(i-diff, r) - IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i-diff, r) > m_right->get(i, r) - IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_le() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_left->size() > m_right->size()) {
        maxp = m_left.get();
        minp = m_right.get();
    } else {
        maxp = m_right.get();
        minp = m_left.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() > m_right->size()) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i, r) < m_right->get(i-diff, r) + IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (m_left->get(i-diff, r) < m_right->get(i, r) + IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_and() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (maxp->get(i, r) >= IND_EQ_THRESHOLD
                    && minp->get(i-diff, r) >= IND_EQ_THRESHOLD) {
                _set(1, i, r);
            } else {
                _set(0, i, r);
            }
        }
    }
}

void IndicatorImp::execute_or() {
    m_right->calculate();
    m_left->calculate();

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (maxp->get(i, r) >= IND_EQ_THRESHOLD
                    || minp->get(i-diff, r) >= IND_EQ_THRESHOLD) {
                _set(1, i, r);
            } else {
                _set(0, i, r);
            }
        }
    }
}

} /* namespace hku */
