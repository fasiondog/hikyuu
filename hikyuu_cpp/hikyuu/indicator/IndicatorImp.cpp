/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <stdexcept>
#include "Indicator.h"
#include "../Stock.h"
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
: m_name("IndicatorImp"), m_discard(0), m_result_num(0), 
  m_need_calculate(true), m_optype(LEAF)  {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name)
: m_name(name), m_discard(0), m_result_num(0), 
  m_need_calculate(true), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name, size_t result_num)
: m_name(name), m_discard(0), 
  m_need_calculate(true), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList*) * MAX_RESULT_NUM);
    m_result_num = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
}

void IndicatorImp::initContext() {
    setParam<KData>("kdata", KData());
}

void IndicatorImp::setContext(const Stock& stock, const KQuery& query) {
    m_need_calculate = true;

    //子节点设置上下文
    if (m_left) m_left->setContext(stock, query);
    if (m_right) m_right->setContext(stock, query);
    if (m_three) m_three->setContext(stock, query);

     //如果上下文有变化则重设上下文
    KData kdata = getContext();
    if (kdata.getStock() != stock || kdata.getQuery() != query) {
        setParam<KData>("kdata", stock.getKData(query));        
    }

    //启动重新计算
    calculate();
}

void IndicatorImp::setContext(const KData& k) {
    m_need_calculate = true;

    //子节点设置上下文
    if (m_left) m_left->setContext(k);
    if (m_right) m_right->setContext(k);
    if (m_three) m_three->setContext(k);

    //如果上下文有变化则重设上下文
    KData old_k = getParam<KData>("kdata");
    if (old_k.getStock() != k.getStock() || old_k.getQuery() != k.getQuery()) {
        setParam<KData>("kdata", k);
    }

    //启动重新计算
    calculate();
}


void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
    if (result_num > MAX_RESULT_NUM) {
        throw(std::invalid_argument("result_num oiverload MAX_RESULT_NUM! "
                "[IndicatorImp::_readyBuffer]" + name()));
    }

    if (result_num == 0) {
        //HKU_TRACE("result_num is zeror! (" << name() << ") [IndicatorImp::_readyBuffer]")
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
    p->m_need_calculate = m_need_calculate;
    p->m_optype = m_optype;

    for (size_t i = 0; i < m_result_num; ++i) {
        if (m_pBuffer[i]) {
            p->m_pBuffer[i] = new PriceList(m_pBuffer[i]->begin(), m_pBuffer[i]->end());
        }
    }

    if (m_left) {
        p->m_left = m_left->clone();
    }
    if (m_right) {
        p->m_right = m_right->clone();
    }
    if (m_three) {
        p->m_three = m_three->clone();
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

price_t IndicatorImp::getByDate(Datetime date, size_t num) {
    price_t result = Null<price_t>();
    KData k = getContext();
    size_t pos = k.getPos(date);
    if (pos != Null<size_t>()) {
        result = get(pos, num);
    }
    return result;
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

        case MOD:
            buf << m_left->formula() << " % " << m_right->formula();
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

        case WEAVE:
            buf << "WEAVE(" << m_left->formula() << ", " << m_right->formula() << ")";

        case IF:
            buf << "IF(" << m_three->formula() << ", " 
                << m_left->formula() << ", "
                << m_right->formula() << ")";
            break;

        default:
            HKU_ERROR("Wrong optype!" << m_optype << " [IndicatorImp::formula]");
            break;
    }

    return buf.str();
}


void IndicatorImp::add(OPType op, IndicatorImpPtr left, IndicatorImpPtr right) {
    if (op == LEAF || op >= INVALID || !right) {
        HKU_ERROR("Wrong used [IndicatorImp::add]");
        return;
    }

    if (OP == op && !isLeaf()) {
        if (m_left) {
            if (m_left->isNeedContext()) {
                if (m_left->isLeaf()) {
                    m_need_calculate = true;
                    m_left = right->clone();
                } else {
                    HKU_WARN("Context-dependent indicator can only be at the leaf node!"
                             << "parent node: " << name() 
                             << ", try add node: " << right->name()
                             << " [IndicatorImp::add]");
                }
            } else {
                if (m_left->isLeaf()) {
                    m_left->m_need_calculate = true;
                    m_left->m_optype = op;
                    m_left->m_right = right->clone();
                } else {
                    m_left->add(OP, left, right);
                }
            }
        }
        if (m_right) {
            if (m_right->isNeedContext()) {
                if (m_right->isLeaf()) {
                    m_need_calculate = true;
                    m_right = right->clone();
                } else {
                    HKU_WARN("Context-dependent indicator can only be at the leaf node!"
                             << "parent node: " << name() 
                             << ", try add node: " << right->name()
                             << " [IndicatorImp::add]");
                }
            } else {
                if (m_right->isLeaf()) {
                    m_right->m_need_calculate = true;
                    m_right->m_optype = op;
                    m_right->m_right = right->clone();
                } else {
                    m_right->add(OP, left, right);
                }
            }
        }
    } else {
        m_need_calculate = true;
        m_optype = op;
        m_left = left ? left->clone() : left;
        m_right = right->clone();
    }
}

void IndicatorImp::
add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right) {
    if (!cond || !left || !right) {
        HKU_ERROR("Wrong used [IndicatorImp::add_if]");
        return;
    }

    m_need_calculate = true;
    m_optype = IndicatorImp::IF;
    m_three = cond->clone();
    m_left = left->clone();
    m_right = right->clone();
}

bool IndicatorImp::needCalculate() {
    if (m_need_calculate) {
        return true;
    }
    
    //子节点设置上下文
    if (m_left) {
        m_need_calculate = m_left->needCalculate();
        if (m_need_calculate) {
            return true;
        }
    }

    if (m_right) {
        m_need_calculate = m_right->needCalculate();
        if (m_need_calculate) {
            return true;
        }
    }

    if (m_three) {
        m_need_calculate = m_three->needCalculate();
        if (m_need_calculate) {
            return true;
        }
    }

    return false;
}

Indicator IndicatorImp::calculate() {
    IndicatorImpPtr result;
    if (!check()) {
        HKU_ERROR("Invalid param! " << formula() << " : " << long_name());
        if (m_right) {
            m_right->calculate();
            _readyBuffer(m_right->size(), m_result_num);
            m_discard = m_right->size();
            try {
                result = shared_from_this();
            } catch (...) {
                //Python中继承的实现会出现bad_weak_ptr错误，通过此方式避免
                result = clone();
            }
        }
            
        return Indicator(result);
    }

    if (!needCalculate()) {
        try {
            result = shared_from_this();
        } catch (...) {
            result = clone();
        }
        return Indicator(result);
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

        case MOD:
            execute_mod();
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

        case WEAVE:
            execute_weave();
            break;

        case IF:
            execute_if();
            break;

        default:
            HKU_ERROR("Unkown Indicator::OPType! " 
                      << m_optype << " [IndicatorImp::calculate]");
            break;
    }

    //使用原型方式时，不加此判断无法立刻重新计算
    if (size() != 0) {
        m_need_calculate = false;
    }

    try {
        result = shared_from_this();
    } catch (...) {
        //Python中继承的实现会出现bad_weak_ptr错误，通过此方式避免
        result = clone();
    }

    return Indicator(result);
}

void IndicatorImp::execute_weave() {
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

    size_t result_number = minp->getResultNumber() + maxp->getResultNumber();
    if (result_number > MAX_RESULT_NUM) {
        result_number = MAX_RESULT_NUM;
    }
    size_t diff = maxp->size() - minp->size();
    _readyBuffer(total, result_number);
    setDiscard(discard);
    if (m_left->size() >= m_right->size()) {
        size_t num = m_left->getResultNumber();
        for (size_t r = 0; r < num; ++r) {
            for (size_t i = discard; i < total; ++i) {
                _set(m_left->get(i, r), i, r);
            }
        }
        for (size_t r = num; r < result_number; r++) {
            for (size_t i = discard; i < total; i++) {
                _set(m_right->get(i-diff, r-num), i, r);
            }
        }
    } else {
        size_t num = m_left->getResultNumber();
        for (size_t r = 0; r < num; ++r) {
            for (size_t i = discard; i < total; ++i) {
                _set(m_left->get(i-diff, r), i, r);
            }
        }
        for (size_t r = num; r < result_number; r++) {
            for (size_t i = discard; i < total; i++) {
                _set(m_right->get(i, r-num), i, r);
            }
        }
    }
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                _set(m_left->get(i, r) - m_right->get(i-diff, r), i, r);
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i-diff, r) == 0.0) {
                    _set(Null<price_t>(), i, r);    
                } else {
                    _set(m_left->get(i, r) / m_right->get(i-diff, r), i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i, r) == 0.0) {
                    _set(Null<price_t>(), i, r); 
                } else {
                    _set(m_left->get(i-diff, r) / m_right->get(i, r), i, r);
                }
            }
        } 
    }
}

void IndicatorImp::execute_mod() {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i-diff, r) == 0.0) {
                    _set(Null<price_t>(), i, r);    
                } else {
                    _set(hku_int64(m_left->get(i, r)) % hku_int64(m_right->get(i-diff, r)), i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i, r) == 0.0) {
                    _set(Null<price_t>(), i, r); 
                } else {
                    _set(hku_int64(m_left->get(i-diff, r)) % hku_int64(m_right->get(i, r)), i, r);
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i, r) - m_right->get(i-diff, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i-diff, r) - m_left->get(i, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i, r) > m_right->get(i-diff, r) - IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
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
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i, r) < m_right->get(i-diff, r) + IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
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
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
            if (maxp->get(i, r) >= IND_EQ_THRESHOLD
                    || minp->get(i-diff, r) >= IND_EQ_THRESHOLD) {
                _set(1, i, r);
            } else {
                _set(0, i, r);
            }
        }
    }
}

void IndicatorImp::execute_if() {
    m_three->calculate();
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

    if (m_three->size() >= maxp->size()) {
        total = m_three->size();
        discard = total + discard - maxp->size();
    } else {
        discard = total - m_three->size();
    }

    size_t diff_right = total - m_right->size();
    size_t diff_left = total - m_left->size();
    size_t diff_cond = total - m_three->size();

    size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    setDiscard(discard);
    for (size_t r = 0; r < result_number; ++r) {
        for (size_t i = discard; i < total; ++i) {
            if (m_three->get(i-diff_cond) > 0.0) {
                _set(m_left->get(i-diff_left), i, r);
            } else {
                _set(m_right->get(i-diff_right), i ,r);
            }
        }
    }
}

} /* namespace hku */
