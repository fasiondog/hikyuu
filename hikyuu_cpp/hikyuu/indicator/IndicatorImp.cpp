/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <stdexcept>
#include <algorithm>
#include "Indicator.h"
#include "IndParam.h"
#include "../Stock.h"
#include "../Log.h"
#include "../GlobalInitializer.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IndicatorImp)
#endif

namespace hku {

StealThreadPool *IndicatorImp::ms_tg = nullptr;

void IndicatorImp::initDynEngine() {
    auto cpu_num = std::thread::hardware_concurrency();
    if (cpu_num > 32) {
        cpu_num = 32;
    } else if (cpu_num >= 4) {
        cpu_num -= 2;
    } else if (cpu_num > 1) {
        cpu_num--;
    }
    ms_tg = new StealThreadPool(cpu_num);
    HKU_CHECK(ms_tg, "Failed init indicator dynamic engine");
}

void IndicatorImp::releaseDynEngine() {
    HKU_TRACE("releaseDynEngine");
    if (ms_tg) {
        ms_tg->stop();
        delete ms_tg;
        ms_tg = nullptr;
    }
}

HKU_API std::ostream &operator<<(std::ostream &os, const IndicatorImp &imp) {
    os << "Indicator{\n"
       << "  name: " << imp.name() << "\n  size: " << imp.size() << "\n  discard: " << imp.discard()
       << "\n  result sets: " << imp.getResultNumber() << "\n  params: " << imp.getParameter()
       << "\n  support indicator param: " << (imp.supportIndParam() ? "True" : "False");
    if (imp.supportIndParam()) {
        os << "\n  ind params: {";
        const auto &ind_params = imp.getIndParams();
        for (auto iter = ind_params.begin(); iter != ind_params.end(); ++iter) {
            os << iter->first << ": " << iter->second->formula() << ", ";
        }
        os << "}";
    }
    os << "\n  formula: " << imp.formula() << "\n}";
    return os;
}

HKU_API std::ostream &operator<<(std::ostream &os, const IndicatorImpPtr &imp) {
    if (!imp) {
        os << "Indicator {}";
    } else {
        os << *imp;
    }
    return os;
}

IndicatorImp::IndicatorImp()
: m_name("IndicatorImp"), m_discard(0), m_result_num(0), m_need_calculate(true), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name)
: m_name(name), m_discard(0), m_result_num(0), m_need_calculate(true), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name, size_t result_num)
: m_name(name), m_discard(0), m_need_calculate(true), m_optype(LEAF) {
    initContext();
    memset(m_pBuffer, 0, sizeof(PriceList *) * MAX_RESULT_NUM);
    m_result_num = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
}

void IndicatorImp::setIndParam(const string &name, const Indicator &ind) {
    IndicatorImpPtr imp = ind.getImp();
    HKU_CHECK(imp, "Invalid input ind, no concrete implementation!");
    m_ind_params[name] = imp;
}

void IndicatorImp::setIndParam(const string &name, const IndParam &ind) {
    IndicatorImpPtr imp = ind.getImp();
    HKU_CHECK(imp, "Invalid input ind, no concrete implementation!");
    m_ind_params[name] = imp;
}

IndParam IndicatorImp::getIndParam(const string &name) const {
    return IndParam(m_ind_params.at(name));
}

const IndicatorImpPtr &IndicatorImp::getIndParamImp(const string &name) const {
    return m_ind_params.at(name);
}

void IndicatorImp::initContext() {
    setParam<KData>("kdata", KData());
}

void IndicatorImp::setContext(const Stock &stock, const KQuery &query) {
    KData kdata = getContext();
    if (kdata.getStock() == stock && kdata.getQuery() == query) {
        if (m_need_calculate) {
            calculate();
        }
        return;
    }

    m_need_calculate = true;

    // 子节点设置上下文
    if (m_left)
        m_left->setContext(stock, query);
    if (m_right)
        m_right->setContext(stock, query);
    if (m_three)
        m_three->setContext(stock, query);

    // 如果上下文有变化则重设上下文
    setParam<KData>("kdata", stock.getKData(query));

    // 启动重新计算
    calculate();
}

void IndicatorImp::setContext(const KData &k) {
    KData old_k = getParam<KData>("kdata");

    // 上下文没变化的情况下根据自身标识进行计算
    if (old_k == k) {
        if (m_need_calculate) {
            calculate();
        }
        return;
    }

    m_need_calculate = true;

    // 子节点设置上下文
    if (m_left)
        m_left->setContext(k);
    if (m_right)
        m_right->setContext(k);
    if (m_three)
        m_three->setContext(k);

    // 对动态参数设置上下文
    for (auto iter = m_ind_params.begin(); iter != m_ind_params.end(); ++iter) {
        iter->second->setContext(k);
    }

    // 重设上下文
    setParam<KData>("kdata", k);

    // 启动重新计算
    calculate();
}

void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
    HKU_CHECK_THROW(result_num <= MAX_RESULT_NUM, std::invalid_argument,
                    "result_num oiverload MAX_RESULT_NUM! {}", name());
    HKU_IF_RETURN(result_num == 0, void());

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

    for (auto iter = m_ind_params.begin(); iter != m_ind_params.end(); ++iter) {
        p->m_ind_params[iter->first] = iter->second->clone();
    }
    return p;
}

IndicatorImpPtr IndicatorImp::operator()(const Indicator &ind) {
    HKU_INFO("This indicator not support operator()! {}", *this);
    // 保证对齐
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
    }
    m_discard = tmp_discard;
}

string IndicatorImp::long_name() const {
    return name() + "(" + m_params.getNameValueList() + ")";
}

PriceList IndicatorImp::getResultAsPriceList(size_t result_num) {
    HKU_IF_RETURN(result_num >= m_result_num || m_pBuffer[result_num] == NULL, PriceList());
    return (*m_pBuffer[result_num]);
}

IndicatorImpPtr IndicatorImp::getResult(size_t result_num) {
    HKU_IF_RETURN(result_num >= m_result_num || m_pBuffer[result_num] == NULL, IndicatorImpPtr());
    IndicatorImpPtr imp = make_shared<IndicatorImp>();
    size_t total = size();
    imp->_readyBuffer(total, 1);
    imp->setDiscard(discard());
    for (size_t i = discard(); i < total; ++i) {
        imp->_set(get(i, result_num), i);
    }
    return imp;
}

price_t IndicatorImp::get(size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
    HKU_CHECK_THROW((m_pBuffer[num] != NULL) && pos < m_pBuffer[num]->size(), std::out_of_range,
                    "Try to access value ({}) out of bounds [0..{})! {}", pos,
                    m_pBuffer[num]->size(), name());
#endif
    return (*m_pBuffer[num])[pos];
}

void IndicatorImp::_set(price_t val, size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
    HKU_CHECK_THROW((m_pBuffer[num] != NULL) && pos < m_pBuffer[num]->size(), std::out_of_range,
                    "Try to access value out of bounds! (pos={}) {}", pos, name());
#endif
    (*m_pBuffer[num])[pos] = val;
}

DatetimeList IndicatorImp::getDatetimeList() const {
    HKU_IF_RETURN(haveParam("align_date_list"), getParam<DatetimeList>("align_date_list"));
    return getContext().getDatetimeList();
}

Datetime IndicatorImp::getDatetime(size_t pos) const {
    if (haveParam("align_date_list")) {
        DatetimeList dates(getParam<DatetimeList>("align_date_list"));
        return pos < dates.size() ? dates[pos] : Null<Datetime>();
    }
    KData k = getContext();
    return pos < k.size() ? k[pos].datetime : Null<Datetime>();
}

price_t IndicatorImp::getByDate(Datetime date, size_t num) {
    size_t pos = getPos(date);
    return (pos != Null<size_t>()) ? get(pos, num) : Null<price_t>();
}

size_t IndicatorImp::getPos(Datetime date) const {
    if (haveParam("align_date_list")) {
        DatetimeList dates(getParam<DatetimeList>("align_date_list"));
        auto iter = std::lower_bound(dates.begin(), dates.end(), date);
        if (iter != dates.end() && *iter == date) {
            return iter - dates.begin();
        } else {
            return Null<size_t>();
        }
    }
    return getContext().getPos(date);
}

string IndicatorImp::formula() const {
    std::stringstream buf;

    switch (m_optype) {
        case LEAF:
            buf << m_name;
            break;

        case OP:
            buf << m_name << "(" << m_right->formula() << ")";
            break;

        case ADD:
            buf << m_left->formula() << " + " << m_right->formula();
            break;

        case SUB:
            buf << m_left->formula() << " - " << m_right->formula();
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
            buf << m_name << "(" << m_left->formula() << ", " << m_right->formula() << ")";
            break;

        case OP_IF:
            buf << "IF(" << m_three->formula() << ", " << m_left->formula() << ", "
                << m_right->formula() << ")";
            break;

        case CORR:
            buf << m_name << "(" << m_left->formula() << ", " << m_right->formula() << ")";
            break;

        default:
            HKU_ERROR("Wrong optype! {}", int(m_optype));
            break;
    }

    return buf.str();
}

void IndicatorImp::add(OPType op, IndicatorImpPtr left, IndicatorImpPtr right) {
    HKU_ERROR_IF_RETURN(op == LEAF || op >= INVALID || !right, void(), "Wrong used!");
    if (OP == op && !isLeaf()) {
        if (m_left) {
            if (m_left->isNeedContext()) {
                if (m_left->isLeaf()) {
                    m_need_calculate = true;
                    m_left = right->clone();
                } else {
                    HKU_WARN(
                      "Context-dependent indicator can only be at the leaf node!"
                      "parent node: {}, try add node: {}",
                      name(), right->name());
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
                    HKU_WARN(
                      "Context-dependent indicator can only be at the leaf node!"
                      "parent node: {}, try add node: {}",
                      name(), right->name());
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
        if (m_three) {
            if (m_three->isNeedContext()) {
                if (m_three->isLeaf()) {
                    m_need_calculate = true;
                    m_three = right->clone();
                } else {
                    HKU_WARN(
                      "Context-dependent indicator can only be at the leaf node!"
                      "parent node: {}, try add node: {}",
                      name(), right->name());
                }
            } else {
                if (m_three->isLeaf()) {
                    m_three->m_need_calculate = true;
                    m_three->m_optype = op;
                    m_three->m_right = right->clone();
                } else {
                    m_three->add(OP, left, right);
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

void IndicatorImp::add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right) {
    HKU_ERROR_IF_RETURN(!cond || !left || !right, void(), "Wrong used!");
    m_need_calculate = true;
    m_optype = IndicatorImp::OP_IF;
    m_three = cond->clone();
    m_left = left->clone();
    m_right = right->clone();
}

bool IndicatorImp::needCalculate() {
    if (m_need_calculate) {
        return true;
    }

    // 子节点设置上下文
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

    for (auto iter = m_ind_params.begin(); iter != m_ind_params.end(); ++iter) {
        m_need_calculate = iter->second->needCalculate();
        if (m_need_calculate) {
            return true;
        }
    }

    return false;
}

Indicator IndicatorImp::calculate() {
    IndicatorImpPtr result;
    if (!check()) {
        HKU_ERROR("Invalid param! {} : {}", formula(), long_name());
        if (m_right) {
            m_right->calculate();
            _readyBuffer(m_right->size(), m_result_num);
            m_discard = m_right->size();
            try {
                result = shared_from_this();
            } catch (...) {
                // Python中继承的实现会出现bad_weak_ptr错误，通过此方式避免
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
            if (m_ind_params.empty()) {
                _calculate(Indicator());
            } else {
                _dyn_calculate(Indicator());
            }
            break;

        case OP: {
            m_right->calculate();
            Indicator tmp_ind(m_right);
            for (auto iter = m_ind_params.begin(); iter != m_ind_params.end(); ++iter) {
                if (iter->second->m_ind_params.empty()) {
                    iter->second->_calculate(tmp_ind);
                } else {
                    iter->second->_dyn_calculate(tmp_ind);
                }
            }
            _readyBuffer(m_right->size(), m_result_num);
            if (m_ind_params.empty()) {
                _calculate(tmp_ind);
            } else {
                _dyn_calculate(tmp_ind);
            }
            setParam<KData>("kdata", m_right->getParam<KData>("kdata"));
        } break;

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

        case OP_IF:
            execute_if();
            break;

        case CORR:
            execute_corr();
            break;

        default:
            HKU_ERROR("Unkown Indicator::OPType! {}", int(m_optype));
            break;
    }

    // 使用原型方式时，不加此判断无法立刻重新计算
    if (size() != 0) {
        m_need_calculate = false;
    }

    try {
        result = shared_from_this();
    } catch (...) {
        // Python中继承的实现会出现bad_weak_ptr错误，通过此方式避免
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
                _set(m_right->get(i - diff, r - num), i, r);
            }
        }
    } else {
        size_t num = m_left->getResultNumber();
        for (size_t r = 0; r < num; ++r) {
            for (size_t i = discard; i < total; ++i) {
                _set(m_left->get(i - diff, r), i, r);
            }
        }
        for (size_t r = num; r < result_number; r++) {
            for (size_t i = discard; i < total; i++) {
                _set(m_right->get(i, r - num), i, r);
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
            _set(maxp->get(i, r) + minp->get(i - diff, r), i, r);
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
                _set(m_left->get(i, r) - m_right->get(i - diff, r), i, r);
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                _set(m_left->get(i - diff, r) - m_right->get(i, r), i, r);
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
            _set(maxp->get(i, r) * minp->get(i - diff, r), i, r);
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
                if (m_right->get(i - diff, r) == 0.0) {
                    _set(Null<price_t>(), i, r);
                } else {
                    _set(m_left->get(i, r) / m_right->get(i - diff, r), i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i, r) == 0.0) {
                    _set(Null<price_t>(), i, r);
                } else {
                    _set(m_left->get(i - diff, r) / m_right->get(i, r), i, r);
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
                if (m_right->get(i - diff, r) == 0.0) {
                    _set(Null<price_t>(), i, r);
                } else {
                    _set(int64_t(m_left->get(i, r)) % int64_t(m_right->get(i - diff, r)), i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i, r) == 0.0) {
                    _set(Null<price_t>(), i, r);
                } else {
                    _set(int64_t(m_left->get(i - diff, r)) % int64_t(m_right->get(i, r)), i, r);
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
            if (std::fabs(maxp->get(i, r) - minp->get(i - diff, r)) < IND_EQ_THRESHOLD) {
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
            if (std::fabs(maxp->get(i, r) - minp->get(i - diff, r)) < IND_EQ_THRESHOLD) {
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
                if (m_left->get(i, r) - m_right->get(i - diff, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i - diff, r) - m_right->get(i, r) >= IND_EQ_THRESHOLD) {
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
                if (m_right->get(i - diff, r) - m_left->get(i, r) >= IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_right->get(i, r) - m_left->get(i - diff, r) >= IND_EQ_THRESHOLD) {
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
                if (m_left->get(i, r) > m_right->get(i - diff, r) - IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i - diff, r) > m_right->get(i, r) - IND_EQ_THRESHOLD) {
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
                if (m_left->get(i, r) < m_right->get(i - diff, r) + IND_EQ_THRESHOLD) {
                    _set(1, i, r);
                } else {
                    _set(0, i, r);
                }
            }
        }
    } else {
        for (size_t r = 0; r < result_number; ++r) {
            for (size_t i = discard; i < total; ++i) {
                if (m_left->get(i - diff, r) < m_right->get(i, r) + IND_EQ_THRESHOLD) {
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
            if (maxp->get(i, r) >= IND_EQ_THRESHOLD && minp->get(i - diff, r) >= IND_EQ_THRESHOLD) {
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
            if (maxp->get(i, r) >= IND_EQ_THRESHOLD || minp->get(i - diff, r) >= IND_EQ_THRESHOLD) {
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
            if (m_three->get(i - diff_cond) > 0.0) {
                _set(m_left->get(i - diff_left), i, r);
            } else {
                _set(m_right->get(i - diff_right), i, r);
            }
        }
    }
}

void IndicatorImp::execute_corr() {
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

    // 结果 0 存放相关系数结果
    // 结果 1 存放协方差（COV）结果
    _readyBuffer(total, 2);

    int n = getParam<int>("n");
    if (n < 2 || discard + 2 > total) {
        setDiscard(total);
        return;
    }

    price_t null_price = Null<price_t>();
    vector<price_t> prebufx(total, null_price);
    vector<price_t> prebufy(total, null_price);
    vector<price_t> prepowx(total, null_price);
    vector<price_t> prepowy(total, null_price);
    vector<price_t> prepowxy(total, null_price);
    price_t kx = maxp->get(discard);
    price_t ky = minp->get(discard);
    price_t ex = 0.0, ey = 0.0, exy = 0.0, varx = 0.0, vary = 0.0, cov = 0.0;
    price_t ex2 = 0.0, ey2 = 0.0, exy2 = 0.0;
    prebufx[discard] = 0.0;
    prebufy[discard] = 0.0;
    prepowx[discard] = 0.0;
    prepowy[discard] = 0.0;
    prepowxy[discard] = 0.0;

    for (size_t i = discard, nobs = 0; i < total; ++i) {
        price_t ix = maxp->get(i) - kx;
        price_t iy = minp->get(i) - ky;
        price_t preix = prebufx[i - nobs];
        price_t preiy = prebufy[i - nobs];
        price_t prepowix = prepowx[i - nobs];
        price_t prepowiy = prepowy[i - nobs];
        price_t prepowixy = prepowxy[i - nobs];
        HKU_INFO_IF(i % 100 == 0, "{}: ix: {}, iy: {}, preix: {}, preiy: {}", i, ix, iy, preix,
                    preiy);
        if (!std::isnan(preix) && !std::isnan(preiy) && !std::isnan(ix) && !std::isnan(iy)) {
            if (nobs < n) {
                nobs++;
                ex += ix;
                ey += iy;
                exy += ix * iy;
                ex2 += std::pow(ix, 2);
                ey2 += std::pow(iy, 2);
                varx = nobs == 1 ? 0. : (ex2 - std::pow(ex, 2) / nobs) / (nobs - 1);
                vary = nobs == 1 ? 0. : (ey2 - std::pow(ey, 2) / nobs) / (nobs - 1);
                cov = nobs == 1 ? 0. : (exy - ex * ey / nobs) / (nobs - 1);
                _set(cov / std::sqrt(varx * vary), i, 0);
                _set(cov, i, 1);
            } else {
                ex += ix - preix;
                ey += iy - preiy;
                ex2 = ex2 - prepowix + std::pow(ix, 2);
                ey2 = ey2 - prepowiy + std::pow(iy, 2);
                exy = exy + ix * iy - prepowixy;
                varx = (ex2 - std::pow(ex, 2) / n) / (n - 1);
                vary = (ey2 - std::pow(ey, 2) / n) / (n - 1);
                cov = (exy - ex * ey / n) / (n - 1);
                _set(cov / std::sqrt(varx * vary), i, 0);
                _set(cov, i, 1);
            }
            prebufx[i] = ix;
            prebufy[i] = iy;
            prepowx[i] = ex2;
            prepowy[i] = ey2;
            prepowxy[i] = exy2;
        }
    }

    // 修正 discard
    setDiscard(++discard);
}

void IndicatorImp::_dyn_calculate(const Indicator &ind) {
    // SPEND_TIME(IndicatorImp__dyn_calculate);
    const auto &ind_param = getIndParamImp("n");
    HKU_CHECK(ind_param->size() == ind.size(), "ind_param->size()={}, ind.size()={}!",
              ind_param->size(), ind.size());
    m_discard = std::max(ind.discard(), ind_param->discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    static const size_t minCircleLength = 400;
    size_t workerNum = ms_tg->worker_num();
    if (total < minCircleLength || isSerial() || workerNum == 1) {
        // HKU_INFO("single_thread");
        for (size_t i = ind.discard(); i < total; i++) {
            size_t step = size_t(ind_param->get(i));
            _dyn_run_one_step(ind, i, step);
        }
        _update_discard();
        return;
    }

    // HKU_INFO("multi_thread");
    size_t circleLength = minCircleLength;
    if (minCircleLength * workerNum >= total) {
        circleLength = minCircleLength;
    } else {
        size_t tailCount = total % workerNum;
        circleLength = tailCount == 0 ? total / workerNum : total / workerNum + 1;
    }

    std::vector<std::future<void>> tasks;
    for (size_t group = 0; group < workerNum; group++) {
        size_t first = circleLength * group;
        if (first >= total) {
            break;
        }
        tasks.push_back(ms_tg->submit([=, &ind, &ind_param]() {
            size_t endPos = first + circleLength;
            if (endPos > total) {
                endPos = total;
            }
            for (size_t i = circleLength * group; i < endPos; i++) {
                size_t step = size_t(ind_param->get(i));
                _dyn_run_one_step(ind, i, step);
            }
        }));
    }

    for (auto &task : tasks) {
        task.get();
    }

    _update_discard();
}

void IndicatorImp::_update_discard() {
    size_t total = size();
    for (size_t result_index = 0; result_index < m_result_num; result_index++) {
        size_t discard = m_discard;
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(get(i, result_index))) {
                break;
            }
            discard++;
        }
        if (discard > m_discard) {
            m_discard = discard;
        }
    }
}

} /* namespace hku */
