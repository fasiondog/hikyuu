/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <stdexcept>
#include <algorithm>
#include <forward_list>
#include <stack>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/global/sysinfo.h"
#include "Indicator.h"
#include "IndParam.h"
#include "../Stock.h"
#include "../GlobalInitializer.h"
#include "imp/ICval.h"
#include "imp/IContext.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IndicatorImp)
#endif

namespace hku {

bool IndicatorImp::ms_enable_increment_calculate{true};
GlobalStealThreadPool *IndicatorImp::ms_tg = nullptr;

string HKU_API getOPTypeName(IndicatorImp::OPType op) {
    string name;
    switch (op) {
        case IndicatorImp::LEAF:
            name = "LEAF";
            break;

        case IndicatorImp::OP:
            name = "OP";
            break;

        case IndicatorImp::ADD:
            name = "ADD";
            break;

        case IndicatorImp::SUB:
            name = "SUB";
            break;

        case IndicatorImp::MUL:
            name = "MUL";
            break;

        case IndicatorImp::DIV:
            name = "DIV";
            break;

        case IndicatorImp::MOD:
            name = "MOD";
            break;

        case IndicatorImp::EQ:
            name = "EQ";
            break;

        case IndicatorImp::GT:
            name = "GT";
            break;

        case IndicatorImp::LT:
            name = "LT";
            break;

        case IndicatorImp::NE:
            name = "NE";
            break;

        case IndicatorImp::GE:
            name = "GE";
            break;

        case IndicatorImp::LE:
            name = "LE";
            break;

        case IndicatorImp::AND:
            name = "AND";
            break;

        case IndicatorImp::OR:
            name = "OR";
            break;

        case IndicatorImp::WEAVE:
            name = "WEAVE";
            break;

        case IndicatorImp::OP_IF:
            name = "IF";
            break;

        default:
            name = "UNKNOWN";
            break;
    }
    return name;
}

void IndicatorImp::initDynEngine() {
    size_t cpu_num = std::thread::hardware_concurrency() * 3 / 2;
    if (cpu_num > 128) {
        cpu_num = 128;
    } else if (cpu_num > 64) {
        cpu_num = cpu_num * 10 / 8;
    }

    // 由于 GlobalInitializer 机制，目前借用在此处初始化全局任务组
    init_global_task_group(cpu_num);
    ms_tg = get_global_task_group();
}

void IndicatorImp::releaseDynEngine() {
    HKU_TRACE("releaseDynEngine");
    // 目前的 GlobalInitializer 机制，global_task_group 实际可能已经释放
    // 可能导致 double free, 这里只停止，不负责释放
    // release_global_task_group();
    // ms_tg = nullptr;
}

HKU_API std::ostream &operator<<(std::ostream &os, const IndicatorImp &imp) {
    os << imp.str();
    return os;
}

HKU_API std::ostream &operator<<(std::ostream &os, const IndicatorImpPtr &imp) {
    if (!imp) {
        os << "Indicator {}";
    } else {
        os << imp->str();
    }
    return os;
}

IndicatorImp::IndicatorImp() : m_name("IndicatorImp") {
    memset(m_pBuffer, 0, sizeof(vector<value_t> *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name) : m_name(name) {
    memset(m_pBuffer, 0, sizeof(vector<value_t> *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name, size_t result_num) : m_name(name) {
    memset(m_pBuffer, 0, sizeof(vector<value_t> *) * MAX_RESULT_NUM);
    m_result_num = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
    _readyBuffer(0, m_result_num);
}

void IndicatorImp::baseCheckParam(const string &name) const {}

void IndicatorImp::paramChanged() {
    m_need_calculate = true;
    m_param_changed = true;
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

bool IndicatorImp::supportIncrementCalculate() const {
    return false;
}

bool IndicatorImp::can_inner_calculate() {
    if (m_need_calculate || !ms_enable_increment_calculate || m_result_num == 0 ||
        m_context.empty() || size() < m_context.size() || m_old_context.size() < m_context.size() ||
        !supportIncrementCalculate()) {
        return false;
    }

    if (m_context.front().datetime < m_old_context.front().datetime ||
        m_context.back().datetime > m_old_context.back().datetime) {
        return false;
    }

    if (m_context.getStock() != m_old_context.getStock() ||
        m_old_context.getQuery().kType() != m_context.getQuery().kType() ||
        m_old_context.getQuery().recoverType() != m_context.getQuery().recoverType()) {
        return false;
    }

    size_t start_pos = m_old_context.getPos(m_context.front().datetime);
    if (start_pos == Null<size_t>()) {
        return false;
    }

    size_t last_pos = m_old_context.getPos(m_context.back().datetime);
    if (last_pos == Null<size_t>()) {
        return false;
    }

    if (start_pos > last_pos) {
        return false;
    }

    size_t total = m_context.size();
    if (total != last_pos - start_pos + 1) {
        return false;
    }

    for (size_t r = 0; r < m_result_num; ++r) {
        if (m_pBuffer[r] == nullptr) {
            return false;
        }
        auto *dst = m_pBuffer[r]->data();
        memmove(dst, dst + start_pos, sizeof(value_t) * (total));
        m_pBuffer[r]->resize(total);
    }

    m_discard = start_pos >= m_discard ? 0 : m_discard - start_pos;
    m_need_calculate = false;

    return true;
}

void IndicatorImp::setContext(const KData &k) {
    const KData &old_k = getContext();

    // 上下文没变化的情况下根据自身标识进行计算
    if (old_k == k) {
        if (m_need_calculate) {
            calculate();
        }
        return;
    }

    onlySetContext(k);
    if (can_inner_calculate()) {
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
    // onlySetContext(k);

    // 启动重新计算
    calculate();

    // 清理根节点之下所有节点中间计算数据
    if (!m_parent) {
        vector<IndicatorImpPtr> nodes;
        getAllSubNodes(nodes);
        if (ms_enable_increment_calculate) {
            for (const auto &node : nodes) {
                if (!node->m_need_calculate && ((node->m_optype == LEAF || node->m_optype == OP) &&
                                                !node->supportIncrementCalculate())) {
                    node->_clearBuffer();
                }
            }
        } else {
            for (const auto &node : nodes) {
                if (!node->m_need_calculate) {
                    node->_clearBuffer();
                }
            }
        }
    }
}

void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
    HKU_CHECK_THROW(result_num <= MAX_RESULT_NUM, std::invalid_argument,
                    "result_num oiverload MAX_RESULT_NUM! {}", name());
    HKU_IF_RETURN(result_num == 0, void());

    value_t null_price = Null<value_t>();
    for (size_t i = 0; i < result_num; ++i) {
        if (!m_pBuffer[i]) {
            m_pBuffer[i] = new vector<value_t>(len, null_price);

        } else {
            m_pBuffer[i]->resize(len);
            for (size_t j = 0; j < len; ++j) {
                (*m_pBuffer[i])[j] = null_price;
            }
        }
    }

    for (size_t i = result_num; i < m_result_num; ++i) {
        if (m_pBuffer[i]) {
            delete m_pBuffer[i];
            m_pBuffer[i] = NULL;
        }
    }

    m_result_num = result_num;
}

void IndicatorImp::_clearBuffer() {
    for (size_t i = 0; i < m_result_num; ++i) {
        if (m_pBuffer[i]) {
            delete m_pBuffer[i];
            m_pBuffer[i] = NULL;
        }
    }
}

IndicatorImp::~IndicatorImp() {
    for (size_t i = 0; i < m_result_num; ++i) {
        delete m_pBuffer[i];
    }
}

string IndicatorImp::str() const {
    std::ostringstream os;
    os << "Indicator{\n"
       << "  name: " << name() << "\n  size: " << size() << "\n  discard: " << discard()
       << "\n  result sets: " << getResultNumber() << "\n  params: " << getParameter()
       << "\n  is python object: " << (isPythonObject() ? "True" : "False");
    const auto &ind_params = getIndParams();
    if (!ind_params.empty()) {
        os << "\n  ind params: {";
        for (auto iter = ind_params.begin(); iter != ind_params.end(); ++iter) {
            os << iter->first << ": " << iter->second->formula() << ", ";
        }
        os << "}";
    }
    os << "\n  formula: " << formula();
    DatetimeList dates = getDatetimeList();
    if (!dates.empty()) {
        os << "\n  first: " << dates.front();
        os << "\n  last: " << dates.back();
    }
    for (size_t r = 0; r < getResultNumber(); ++r) {
        if (m_pBuffer[r]) {
            os << "\n  values" << r << ": " << *m_pBuffer[r];
        }
    }
    os << "\n}";
    return os.str();
}

void IndicatorImp::swap(IndicatorImp *other) {
    HKU_ASSERT(other != nullptr);
    HKU_IF_RETURN(this == other, void());
    HKU_ASSERT(other->m_result_num == m_result_num);
    HKU_ASSERT(other->size() == size());
    for (size_t r = 0; r < m_result_num; ++r) {
        vector<value_t> *tmp = m_pBuffer[r];
        m_pBuffer[r] = other->m_pBuffer[r];
        other->m_pBuffer[r] = tmp;
    }
}

void IndicatorImp::swap(IndicatorImp *other, size_t other_result_idx, size_t self_result_idx) {
    HKU_ASSERT(other != nullptr);
    HKU_ASSERT(other->size() == size());
    HKU_ASSERT(other_result_idx < other->m_result_num);
    HKU_ASSERT(self_result_idx < m_result_num);
    vector<value_t> *tmp = m_pBuffer[self_result_idx];
    m_pBuffer[self_result_idx] = other->m_pBuffer[other_result_idx];
    other->m_pBuffer[other_result_idx] = tmp;
}

IndicatorImpPtr IndicatorImp::clone() {
    IndicatorImpPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_need_self_alike_compare = m_need_self_alike_compare;
    p->m_is_serial = m_is_serial;
    p->m_discard = m_discard;
    p->m_result_num = m_result_num;
    p->m_context = m_context;
    p->m_need_calculate = m_need_calculate;
    p->m_param_changed = m_param_changed;
    p->m_optype = m_optype;
    p->m_parent = m_parent;

    p->_readyBuffer(size(), m_result_num);
    for (size_t i = 0; i < m_result_num; ++i) {
        if (m_pBuffer[i])
            std::copy(m_pBuffer[i]->begin(), m_pBuffer[i]->end(), p->m_pBuffer[i]->begin());
    }

    if (m_left) {
        p->m_left = m_left->clone();
        p->m_left->m_parent = this;
    }
    if (m_right) {
        p->m_right = m_right->clone();
        p->m_right->m_parent = this;
    }
    if (m_three) {
        p->m_three = m_three->clone();
        p->m_three->m_parent = this;
    }

    for (auto iter = m_ind_params.begin(); iter != m_ind_params.end(); ++iter) {
        p->m_ind_params[iter->first] = iter->second->clone();
    }

    if (!m_parent) {
        // 重构各子节点的父节点
        std::forward_list<IndicatorImp *> stack;
        stack.push_front(p.get());
        while (!stack.empty()) {
            IndicatorImp *node = stack.front();
            stack.pop_front();
            if (node->m_three) {
                node->m_three->m_parent = node;
                stack.push_front(node->m_three.get());
            }
            if (node->m_left) {
                node->m_left->m_parent = node;
                stack.push_front(node->m_left.get());
            }
            if (node->m_right) {
                node->m_right->m_parent = node;
                stack.push_front(node->m_right.get());
            }
        }

        p->repeatALikeNodes();
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
        value_t null_price = Null<value_t>();
        for (size_t i = 0; i < m_result_num; ++i) {
            auto *dst = this->data(i);
            for (size_t j = m_discard; j < tmp_discard; ++j) {
                // _set(null_price, j, i);
                dst[j] = null_price;
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
#if HKU_USE_LOW_PRECISION
    size_t total = size();
    PriceList result(total);
    const auto &src = (*m_pBuffer[result_num]);
    std::copy(src.begin(), src.end(), result.begin());
    return result;
#else
    return (*m_pBuffer[result_num]);
#endif
}

IndicatorImpPtr IndicatorImp::getResult(size_t result_num) {
    HKU_IF_RETURN(result_num >= m_result_num || m_pBuffer[result_num] == NULL, IndicatorImpPtr());
    IndicatorImpPtr imp = make_shared<IndicatorImp>();
    size_t total = size();
    imp->_readyBuffer(total, 1);
    imp->setDiscard(discard());
    auto const *src = this->data(result_num);
    auto *dst = imp->data(0);
    for (size_t i = discard(); i < total; ++i) {
        dst[i] = src[i];
    }
    return imp;
}

IndicatorImp::value_t IndicatorImp::get(size_t pos, size_t num) const {
#if CHECK_ACCESS_BOUND
    // cppcheck-suppress [arrayIndexOutOfBoundsCond]
    HKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && m_pBuffer[num] && pos < m_pBuffer[num]->size()), std::out_of_range,
      "Try to access value out of bounds! num: {}, pos: {}, name: {}", num, pos, name());
#endif
    return (*m_pBuffer[num])[pos];
}

IndicatorImp::value_t IndicatorImp::front(size_t num) const {
#if CHECK_ACCESS_BOUND
    // cppcheck-suppress [arrayIndexOutOfBoundsCond]
    HKU_CHECK_THROW((num <= MAX_RESULT_NUM && m_pBuffer[num] && !m_pBuffer[num]->empty()),
                    std::out_of_range, "Try to access value out of bounds! num: {}, name: {}", num,
                    name());
#endif
    return m_pBuffer[num]->front();
}

IndicatorImp::value_t IndicatorImp::back(size_t num) const {
#if CHECK_ACCESS_BOUND
    // cppcheck-suppress [arrayIndexOutOfBoundsCond]
    HKU_CHECK_THROW((num <= MAX_RESULT_NUM && m_pBuffer[num] && !m_pBuffer[num]->empty()),
                    std::out_of_range, "Try to access value out of bounds! num: {}, name: {}", num,
                    name());
#endif
    return m_pBuffer[num]->back();
}

void IndicatorImp::_set(value_t val, size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
    // cppcheck-suppress [arrayIndexOutOfBoundsCond]
    HKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && m_pBuffer[num] && pos < m_pBuffer[num]->size()), std::out_of_range,
      "Try to access value out of bounds! num: {}, pos: {}, name: {}", num, pos, name());
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
    const KData &k = getContext();
    return pos < k.size() ? k[pos].datetime : Null<Datetime>();
}

IndicatorImp::value_t IndicatorImp::getByDate(Datetime date, size_t num) {
    size_t pos = getPos(date);
    return (pos != Null<size_t>()) ? get(pos, num) : Null<value_t>();
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

bool IndicatorImp::existNan(size_t result_idx) const {
    HKU_CHECK(result_idx < m_result_num, "result_idx: {}", result_idx);
    const value_t *src = data(result_idx);
    for (size_t i = m_discard, total = size(); i < total; i++) {
        if (std::isnan(src[i])) {
            return true;
        }
    }
    return false;
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
                m_left->add(OP, left, right);
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
                m_right->add(OP, left, right);
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
                m_three->add(OP, left, right);
            }
        }
    } else {
        m_need_calculate = true;
        m_optype = op;
        m_left = left ? left->clone() : left;
        m_right = right->clone();
    }

    if (m_left) {
        m_left->m_parent = this;
    }

    if (m_right) {
        m_right->m_parent = this;
    }

    if (m_three) {
        m_three->m_parent = this;
    }

    if (m_name == "IndicatorImp") {
        m_name = getOPTypeName(op);
    }

    if (!m_parent) {
        repeatALikeNodes();
    }
}

void IndicatorImp::add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right) {
    HKU_ERROR_IF_RETURN(!cond || !left || !right, void(), "Wrong used!");
    m_need_calculate = true;
    m_optype = IndicatorImp::OP_IF;
    m_three = cond->clone();
    m_left = left->clone();
    m_right = right->clone();
    m_three->m_parent = this;
    m_left->m_parent = this;
    m_right->m_parent = this;
    if (m_name == "IndicatorImp") {
        m_name = getOPTypeName(IndicatorImp::OP_IF);
    }
    if (!m_parent) {
        repeatALikeNodes();
    }
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

void IndicatorImp::_calculate(const Indicator &ind) {
    if (isLeaf()) {
        const auto &k = getContext();
        size_t total = k.size();
        HKU_IF_RETURN(total == 0, void());
        _readyBuffer(total, 1);
        m_discard = total;
        return;
    }

    size_t total = ind.size();
    m_result_num = ind.getResultNumber();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, m_result_num);
    m_discard = ind.discard();
    for (size_t r = 0; r < m_result_num; ++r) {
        const auto *src = ind.data(r);
        auto *dst = this->data(r);
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = src[i];
        }
    }
}

bool IndicatorImp::can_increment_calculate() {
    if (m_result_num == 0 || m_context.empty() || m_old_context.empty()) {
        return false;
    }

    if (m_context.front().datetime < m_old_context.front().datetime) {
        return false;
    }

    if (m_context.getStock() != m_old_context.getStock() ||
        m_old_context.getQuery().kType() != m_context.getQuery().kType() ||
        m_old_context.getQuery().recoverType() != m_context.getQuery().recoverType()) {
        return false;
    }

    if (m_context.back().datetime <= m_old_context.back().datetime) {
        return false;
    }

    return true;
}

bool IndicatorImp::increment_execute_leaf_or_op(const Indicator &ind) {
    if (m_param_changed || !ms_enable_increment_calculate || !supportIncrementCalculate() ||
        !can_increment_calculate()) {
        return false;
    }

    size_t copy_start_pos = m_old_context.getPos(m_context.front().datetime);
    if (copy_start_pos == Null<size_t>()) {
        return false;
    }

    size_t total = m_context.size();
    size_t copy_len = m_old_context.size() - copy_start_pos;
    if (copy_len < m_discard) {
        return false;
    }

    size_t start_pos = m_context.getPos(m_old_context.back().datetime);
    if (start_pos == Null<size_t>()) {
        return false;
    }

    if (start_pos < min_increment_start()) {
        return false;
    }

    if (copy_len > 0) {
        for (size_t r = 0; r < m_result_num; ++r) {
            HKU_ASSERT(m_pBuffer[r]);
            m_pBuffer[r]->resize(total, Null<value_t>());
            auto *dst = this->data(r);
            memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
        }
    }

    if (start_pos < m_discard) {
        start_pos = m_discard;
    }

    m_discard = 0;

    if (start_pos < total) {
        _increment_calculate(ind, start_pos);
    }

    _update_discard();
    return true;
}

Indicator IndicatorImp::calculate() {
    IndicatorImpPtr result;
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
                if (!increment_execute_leaf_or_op(Indicator())) {
                    _calculate(Indicator());
                }
            } else {
                _dyn_calculate(Indicator());
            }
            break;

        case OP: {
            if (m_ind_params.empty()) {
                if (!increment_execute_leaf_or_op(Indicator(m_right))) {
                    m_right->calculate();
                    _readyBuffer(m_right->size(), m_result_num);
                    _calculate(Indicator(m_right));
                    onlySetContext(m_right->getContext());
                }
            } else {
                m_right->calculate();
                _readyBuffer(m_right->size(), m_result_num);
                _dyn_calculate(Indicator(m_right));
                onlySetContext(m_right->getContext());
            }
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

        default:
            HKU_ERROR("Unkown Indicator::OPType! {}", int(m_optype));
            break;
    }

    // 使用原型方式时，不加此判断无法立刻重新计算
    if (size() != 0) {
        m_need_calculate = false;
    }

    m_param_changed = false;
    m_old_context = KData();

    try {
        result = shared_from_this();
    } catch (const std::exception &e) {
        if (runningInPython()) {
            // Python中继承的实现会出现bad_weak_ptr错误，通过此方式避免
            // 切换至 pybind11 后，目前应已不在出现，目前保留观察
            result = clone();
        } else {
            HKU_ERROR("IndicatorImp::calculate() error! {}", e.what());
            throw e;
        }
    }

    return Indicator(result);
}

size_t IndicatorImp::increment_execute() {
    size_t null_pos = Null<size_t>();
    if (m_param_changed || !ms_enable_increment_calculate || m_right->m_need_calculate ||
        m_left->m_need_calculate) {
        return null_pos;
    }

    if (!can_increment_calculate()) {
        return null_pos;
    }

    size_t copy_start_pos = m_old_context.getPos(m_context.front().datetime);
    if (copy_start_pos == null_pos) {
        return null_pos;
    }

    size_t total = m_context.size();
    size_t copy_len = m_old_context.size() - copy_start_pos;
    if (copy_len == 0) {
        return null_pos;
    }

    size_t start_pos = m_context.getPos(m_old_context.back().datetime);
    if (start_pos == null_pos) {
        return null_pos;
    }

    for (size_t r = 0; r < m_result_num; ++r) {
        HKU_ASSERT(m_pBuffer[r] != nullptr);
        m_pBuffer[r]->resize(total, Null<value_t>());
        auto *dst = this->data(r);
        memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
    }

    return start_pos;
}

void IndicatorImp::execute_weave() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    const IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t diff = maxp->size() - minp->size();
    size_t total = maxp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = minp->getResultNumber() + maxp->getResultNumber();
        if (result_number > MAX_RESULT_NUM) {
            result_number = MAX_RESULT_NUM;
        }
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t const *src = nullptr;
    value_t *dst = nullptr;
    if (m_left->size() >= m_right->size()) {
        size_t num = m_left->getResultNumber();
        for (size_t r = 0; r < num; ++r) {
            src = m_left->data(r);
            dst = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = src[i];
            }
        }
        for (size_t r = num; r < m_result_num; r++) {
            src = m_right->data(r - num);
            dst = this->data(r);
            for (size_t i = start_pos; i < total; i++) {
                dst[i] = src[i - diff];
            }
        }
    } else {
        size_t num = m_left->getResultNumber();
        for (size_t r = 0; r < num; ++r) {
            src = m_left->data(r);
            dst = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = src[i - diff];
            }
        }
        for (size_t r = num; r < m_result_num; r++) {
            src = m_right->data(r - num);
            dst = this->data(r);
            for (size_t i = start_pos; i < total; i++) {
                dst[i] = src[i];
            }
        }
    }
}

void IndicatorImp::execute_add() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto const *data1 = maxp->data(r);
        auto const *data2 = minp->data(r);
        auto *result = this->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            result[i] = data1[i] + data2[i - diff];
        }
    }
}

void IndicatorImp::execute_sub() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            auto *data1 = m_left->data(r);
            auto *data2 = m_right->data(r);
            auto *result = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                result[i] = data1[i] - data2[i - diff];
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            auto *data1 = m_left->data(r);
            auto *data2 = m_right->data(r);
            auto *result = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                result[i] = data1[i - diff] - data2[i];
            }
        }
    }
}

void IndicatorImp::execute_mul() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto const *data1 = maxp->data(r);
        auto const *data2 = minp->data(r);
        auto *result = this->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            result[i] = data1[i] * data2[i - diff];
        }
    }
}

void IndicatorImp::execute_div() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            auto const *data1 = m_left->data(r);
            auto const *data2 = m_right->data(r);
            auto *result = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                result[i] = data1[i] / data2[i - diff];
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            auto const *data1 = m_left->data(r);
            auto const *data2 = m_right->data(r);
            auto *result = this->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                result[i] = data1[i - diff] / data2[i];
            }
        }
    }
}

void IndicatorImp::execute_mod() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t *dst = nullptr;
    value_t const *left = nullptr;
    value_t const *right = nullptr;
    value_t null_value = Null<value_t>();
    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                if (right[i - diff] == 0.0) {
                    dst[i] = null_value;
                } else {
                    dst[i] = int64_t(left[i]) % int64_t(right[i - diff]);
                }
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                if (right[i] == 0.0) {
                    dst[i] = null_value;
                } else {
                    dst[i] = int64_t(left[i - diff]) % int64_t(right[i]);
                }
            }
        }
    }
}

void IndicatorImp::execute_eq() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto *dst = this->data(r);
        auto const *maxdata = maxp->data(r);
        auto const *mindata = minp->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = (maxdata[i] == mindata[i - diff]) ? 1.0 : 0.0;
        }
    }
}

void IndicatorImp::execute_ne() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto *dst = this->data(r);
        auto const *maxdata = maxp->data(r);
        auto const *mindata = minp->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = (maxdata[i] != mindata[i - diff]) ? 1.0 : 0.0;
        }
    }
}

void IndicatorImp::execute_gt() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t *dst = nullptr;
    value_t const *left = nullptr;
    value_t const *right = nullptr;
    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = (left[i] > right[i - diff]) ? 1.0 : 0.0;
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = (left[i - diff] > right[i]) ? 1.0 : 0.0;
            }
        }
    }
}

void IndicatorImp::execute_lt() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t *dst = nullptr;
    value_t const *left = nullptr;
    value_t const *right = nullptr;
    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = (left[i] < right[i - diff]) ? 1.0 : 0.0;
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = left[i - diff] < right[i] ? 1.0 : 0.0;
            }
        }
    }
}

void IndicatorImp::execute_ge() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t *dst = nullptr;
    value_t const *left = nullptr;
    value_t const *right = nullptr;
    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = left[i] >= right[i - diff] ? 1.0 : 0.0;
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = left[i - diff] >= right[i] ? 1.0 : 0.0;
            }
        }
    }
}

void IndicatorImp::execute_le() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    value_t *dst = nullptr;
    value_t const *left = nullptr;
    value_t const *right = nullptr;
    if (m_left->size() > m_right->size()) {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = left[i] <= right[i - diff] ? 1.0 : 0.0;
            }
        }
    } else {
        for (size_t r = 0; r < m_result_num; ++r) {
            dst = this->data(r);
            left = m_left->data(r);
            right = m_right->data(r);
            for (size_t i = start_pos; i < total; ++i) {
                dst[i] = left[i - diff] <= right[i] ? 1.0 : 0.0;
            }
        }
    }
}

void IndicatorImp::execute_and() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto *dst = this->data(r);
        auto const *maxdata = maxp->data(r);
        auto const *mindata = minp->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = (maxdata[i] > 0.0) && (mindata[i - diff] > 0.0) ? 1.0 : 0.0;
        }
    }
}

void IndicatorImp::execute_or() {
    size_t start_pos = increment_execute();
    if (start_pos == Null<size_t>()) {
        m_right->calculate();
        m_left->calculate();
    }

    IndicatorImp *maxp, *minp;
    if (m_right->size() > m_left->size()) {
        maxp = m_right.get();
        minp = m_left.get();
    } else {
        maxp = m_left.get();
        minp = m_right.get();
    }

    size_t total = maxp->size();
    size_t diff = maxp->size() - minp->size();
    size_t discard = maxp->size() - minp->size() + minp->discard();
    if (discard < maxp->discard()) {
        discard = maxp->discard();
    }

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    for (size_t r = 0; r < m_result_num; ++r) {
        auto *dst = this->data(r);
        auto const *maxdata = maxp->data(r);
        auto const *mindata = minp->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = (maxdata[i] > 0.0) || (mindata[i - diff] > 0.0) ? 1.0 : 0.0;
        }
    }
}

size_t IndicatorImp::increment_execute_if() {
    size_t null_pos = Null<size_t>();
    if (m_param_changed || !ms_enable_increment_calculate || m_three->m_need_calculate ||
        m_right->m_need_calculate || m_left->m_need_calculate) {
        return null_pos;
    }

    if (!can_increment_calculate()) {
        return null_pos;
    }

    size_t copy_start_pos = m_old_context.getPos(m_context.front().datetime);
    if (copy_start_pos == null_pos) {
        return null_pos;
    }

    size_t total = m_context.size();
    size_t copy_len = m_old_context.size() - copy_start_pos;
    if (copy_len == 0) {
        return null_pos;
    }

    size_t start_pos = m_context.getPos(m_old_context.back().datetime);
    if (start_pos == null_pos) {
        return null_pos;
    }

    for (size_t r = 0; r < m_result_num; ++r) {
        HKU_ASSERT(m_pBuffer[r]);
        m_pBuffer[r]->resize(total, Null<value_t>());
        auto *dst = this->data(r);
        memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
    }

    return start_pos;
}

void IndicatorImp::execute_if() {
    size_t start_pos = increment_execute_if();
    if (start_pos == Null<size_t>()) {
        m_three->calculate();
        m_right->calculate();
        m_left->calculate();
    }

    const IndicatorImp *maxp, *minp;
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
    if (discard < m_three->discard()) {
        discard = m_three->discard();
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

    if (start_pos == Null<size_t>()) {
        size_t result_number = std::min(minp->getResultNumber(), maxp->getResultNumber());
        _readyBuffer(total, result_number);
        start_pos = discard;
    } else if (start_pos < discard) {
        start_pos = discard;
    }

    setDiscard(discard);

    auto *left = m_left->data(0);
    auto *right = m_right->data(0);
    auto *three = m_three->data(0);
    for (size_t r = 0; r < m_result_num; ++r) {
        auto *dst = this->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            if (three[i - diff_cond] > 0.0) {
                dst[i] = left[i - diff_left];
            } else {
                dst[i] = right[i - diff_right];
            }
        }
    }
}

void IndicatorImp::_dyn_calculate(const Indicator &ind) {
    // SPEND_TIME(IndicatorImp__dyn_calculate);
    const auto &ind_param = getIndParamImp("n");
    // CVAL或PRICELIST可能会大于ind.size()
    HKU_CHECK(ind_param->size() >= ind.size(), "ind_param->size()={}, ind.size()={}!",
              ind_param->size(), ind.size());
    m_discard = std::max(ind.discard(), ind_param->discard());
    size_t total = ind.size();
    HKU_IF_RETURN(0 == total || m_discard >= total, void());

    const value_t *param_data = ind_param->data();

    static const size_t minCircleLength = 400;
    size_t workerNum = ms_tg->worker_num();
    if (total < minCircleLength || isSerial() || workerNum == 1) {
        // HKU_INFO("single_thread");
        for (size_t i = ind.discard(); i < total; i++) {
            if (std::isnan(param_data[i])) {
                _set(Null<value_t>(), i);
            } else {
                size_t step = size_t(param_data[i]);
                _dyn_run_one_step(ind, i, step);
            }
        }
        _update_discard();
        return;
    }

    // HKU_INFO("multi_thread");
    size_t circleLength = minCircleLength;
    if (minCircleLength * workerNum < total) {
        size_t tailCount = total % workerNum;
        circleLength = tailCount == 0 ? total / workerNum : total / workerNum + 1;
    }

    std::vector<std::future<void>> tasks;
    for (size_t group = 0; group < workerNum; group++) {
        size_t first = circleLength * group;
        if (first >= total) {
            break;
        }
        tasks.push_back(
          ms_tg->submit([this, &ind, first, circleLength, total, group, param_data]() {
              size_t endPos = first + circleLength;
              if (endPos > total) {
                  endPos = total;
              }
              for (size_t i = circleLength * group; i < endPos; i++) {
                  if (std::isnan(param_data[i])) {
                      _set(Null<value_t>(), i);
                  } else {
                      size_t step = size_t(param_data[i]);
                      _dyn_run_one_step(ind, i, step);
                  }
              }
          }));
    }

    for (auto &task : tasks) {
        task.get();
    }

    _update_discard();
}

void IndicatorImp::_update_discard(bool force) {
    if (force) {
        m_discard = 0;
    }
    size_t total = size();
    for (size_t result_index = 0; result_index < m_result_num; result_index++) {
        size_t discard = m_discard;
        const auto *dst = this->data(result_index);
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(dst[i])) {
                break;
            }
            discard++;
        }
        if (discard > m_discard) {
            m_discard = discard;
        }
    }
    if (m_discard > total) {
        m_discard = total;
    }
}

bool IndicatorImp::alike(const IndicatorImp &other) const {
    HKU_IF_RETURN(this == &other, true);
    HKU_IF_RETURN(m_optype != other.m_optype || m_discard != other.m_discard ||
                    m_result_num != other.m_result_num || (isLeaf() && !other.isLeaf()) ||
                    (!isLeaf() && other.isLeaf()) || typeid(*this) != typeid(other) ||
                    m_ind_params.size() != other.m_ind_params.size() || m_params != other.m_params,
                  false);

    if (needSelfAlikeCompare()) {
        return selfAlike(other);
    }

    auto iter1 = m_ind_params.cbegin();
    auto iter2 = other.m_ind_params.cend();
    for (; iter1 != m_ind_params.cend() && iter2 != other.m_ind_params.cend(); ++iter1, ++iter2) {
        HKU_IF_RETURN(iter1->first != iter2->first, false);
        HKU_IF_RETURN(!iter1->second->alike(*(iter2->second)), false);
    }

    if (isLeaf() && other.isLeaf()) {
        HKU_IF_RETURN(this->size() != other.size(), false);
        auto const *d1 = this->data();
        auto const *d2 = other.data();
        bool eq = true;
        for (size_t i = 0, len = this->size(); i < len; i++) {
            if (d1[i] != d2[i]) {
                eq = false;
            }
        }
        return eq;
    }

    HKU_IF_RETURN(m_three && other.m_three && !m_three->alike(*other.m_three), false);
    HKU_IF_RETURN(m_left && other.m_left && !m_left->alike(*other.m_left), false);
    HKU_IF_RETURN(m_right && other.m_right && !m_right->alike(*other.m_right), false);

    return true;
}

bool IndicatorImp::contains(const string &name) const {
    HKU_IF_RETURN(m_name == name, true);
    vector<IndicatorImpPtr> all_nodes;
    getAllSubNodes(all_nodes);
    for (const auto &node : all_nodes) {
        if (node->name() == name) {
            return true;
        }
    }
    return false;
}

void IndicatorImp::getAllSubNodes(vector<IndicatorImpPtr> &nodes) const {
    // 使用栈来模拟递归调用，避免深层递归导致的栈溢出
    std::stack<IndicatorImpPtr> nodeStack;

    // 将当前节点的子节点入栈（按相反顺序入栈以保持原有处理顺序）
    if (m_three) {
        nodeStack.push(m_three);
    }
    if (m_left) {
        nodeStack.push(m_left);
    }
    if (m_right) {
        nodeStack.push(m_right);
    }

    // 处理栈中的节点
    while (!nodeStack.empty()) {
        IndicatorImpPtr current = nodeStack.top();
        nodeStack.pop();

        // 添加当前节点到结果列表
        nodes.push_back(current);

        // 将当前节点的子节点入栈（按相反顺序入栈以保持原有处理顺序）
        if (current->m_three) {
            nodeStack.push(current->m_three);
        }
        if (current->m_left) {
            nodeStack.push(current->m_left);
        }
        if (current->m_right) {
            nodeStack.push(current->m_right);
        }

        // 添加当前节点的内部节点（如果有的话）
        current->getSelfInnerNodesWithInputConext(nodes);
    }

    // 添加当前节点的内部节点
    getSelfInnerNodesWithInputConext(nodes);
}

void IndicatorImp::inner_repeatALikeNodes(vector<IndicatorImpPtr> &sub_nodes) {
    vector<IndicatorImpPtr> tmp_nodes;
    size_t total = sub_nodes.size();
    for (size_t i = 0; i < total; i++) {
        const auto &cur = sub_nodes[i];
        if (!cur) {
            continue;
        }
        for (size_t j = i + 1; j < total; j++) {
            auto &node = sub_nodes[j];
            if (!node || cur == node) {
                continue;
            }

            if (cur->alike(*node)) {
                IndicatorImp *node_parent = node->m_parent;
                if (node_parent) {
                    if (node_parent->m_left == node) {
                        node_parent->m_left = cur;
                    }

                    if (node_parent->m_right == node) {
                        node_parent->m_right = cur;
                    }

                    if (node_parent->m_three == node) {
                        node_parent->m_three = cur;
                    }

                    tmp_nodes.clear();
                    node->getAllSubNodes(tmp_nodes);
                    for (const auto &replace_node : tmp_nodes) {
                        for (size_t k = j + 1; k < total; k++) {
                            if (replace_node == sub_nodes[k]) {
                                sub_nodes[k].reset();
                            }
                        }
                    }

                    node = cur;

                } else {
                    HKU_WARN("Exist some errors! node: {} cur: {}", node->name(), cur->name());
                }
            }
        }
    }
}

void IndicatorImp::repeatALikeNodes() {
    vector<IndicatorImpPtr> sub_nodes;
    getAllSubNodes(sub_nodes);
    inner_repeatALikeNodes(sub_nodes);
    repeatSeparateKTypeLeafALikeNodes();
}

void IndicatorImp::repeatSeparateKTypeLeafALikeNodes() {
    // 需要再上层节点已完成优化后，重新获取所有子节点
    vector<IndicatorImpPtr> all_nodes;
    getAllSubNodes(all_nodes);

    std::unordered_set<IndicatorImp *> special_set;
    std::unordered_map<string, vector<IndicatorImpPtr>> special_nodes;
    for (const auto &node : all_nodes) {
        if (node->isLeaf() && node->haveParam("ktype")) {
            if (special_set.find(node.get()) == special_set.end()) {
                special_set.insert(node.get());
                string ktype = node->getParam<string>("ktype");
                auto iter = special_nodes.find(ktype);
                if (iter == special_nodes.end()) {
                    special_nodes.insert(std::make_pair(ktype, vector<IndicatorImpPtr>{node}));
                } else {
                    iter->second.push_back(node);
                }
            }
        }
    }

    vector<IndicatorImpPtr> can_merge_nodes;
    for (const auto &item : special_nodes) {
        const auto &nodes = item.second;
        size_t total = nodes.size();
        // HKU_INFO("first ktype: {}, total: {}", item.first, total);
        if (total <= 1) {
            continue;
        }

        can_merge_nodes.clear();
        for (size_t i = 0; i < total; i++) {
            nodes[i]->getSeparateKTypeLeafSubNodes(can_merge_nodes);
        }
        inner_repeatALikeNodes(can_merge_nodes);
    }
}

void IndicatorImp::_printTree(int depth, bool isLast, bool show_long_name) const {
    // 打印当前节点
    std::string indent;
    if (depth > 0) {
        // 构建层级缩进
        indent = std::string((depth - 1) * 3, ' ');
        indent += isLast ? "└─ " : "├─ ";
    }

    // 打印节点名称
    std::string name = (show_long_name ? long_name() : this->name());
    if (m_parent) {
        if (this == m_parent->m_three.get()) {
            name = "[T]" + name;
        } else if (this == m_parent->m_left.get()) {
            name = "[L]" + name;
        } else if (this == m_parent->m_right.get()) {
            name = "[R]" + name;
        }
    }
    std::cout << indent << name;

    // 如果是叶子节点，直接换行
    if (isLeaf()) {
        std::cout << std::endl;
        return;
    }

    // 有子节点，先换行再打印子节点
    std::cout << std::endl;

    std::vector<IndicatorImp *> children;
    if (m_three) {
        children.emplace_back(m_three.get());
    }
    if (m_left) {
        children.emplace_back(m_left.get());
    }
    if (m_right) {
        children.emplace_back(m_right.get());
    }

    if (children.empty())
        return;

    // 递归打印子节点
    for (size_t i = 0; i < children.size(); ++i) {
        bool lastChild = (i == children.size() - 1);
        std::cout << std::string(depth * 3, ' ');
        children[i]->_printTree(depth + 1, lastChild, show_long_name);
    }
}

void IndicatorImp::printTree(bool show_long_name) const {
    std::cout << "Tree structure starting from root:" << std::endl;
    _printTree(0, true, show_long_name);
}

vector<IndicatorImp *> IndicatorImp::getAllSubTrees() const {
    vector<IndicatorImpPtr> all_nodes;
    getAllSubNodes(all_nodes);
    std::unordered_set<IndicatorImpPtr> leaves;
    for (const auto &node : all_nodes) {
        if (node->isLeaf()) {
            leaves.insert(node);
        }
    }

    std::unordered_set<IndicatorImp *> tree_set;
    for (const auto &leaf : leaves) {
        const IndicatorImp *tree = leaf.get();
        while (tree->m_parent) {
            tree_set.insert(tree->m_parent);
            tree = tree->m_parent;
        }
    }

    vector<IndicatorImp *> trees;
    trees.reserve(leaves.size() + tree_set.size());
    for (const auto &leaf : leaves) {
        trees.push_back(leaf.get());
    }
    for (const auto &tree : tree_set) {
        trees.push_back(tree);
    }
    return trees;
}

size_t IndicatorImp::treeSize(IndicatorImp *tree) {
    HKU_IF_RETURN(tree == nullptr, 0);
    HKU_IF_RETURN(tree->isLeaf(), 1);

    size_t ret = 1;
    if (tree->m_three != nullptr) {
        ret += treeSize(tree->m_three.get());
    }
    if (tree->m_left != nullptr) {
        ret += treeSize(tree->m_left.get());
    }
    if (tree->m_right != nullptr) {
        ret += treeSize(tree->m_right.get());
    }
    return ret;
}

bool IndicatorImp::nodeInTree(IndicatorImp *node, IndicatorImp *tree) {
    HKU_IF_RETURN(node == nullptr || tree == nullptr, false);
    HKU_IF_RETURN(node == tree, true);
    if (tree->m_three) {
        HKU_IF_RETURN(nodeInTree(node, tree->m_three.get()), true);
    }
    if (tree->m_left) {
        HKU_IF_RETURN(nodeInTree(node, tree->m_left.get()), true);
    }
    if (tree->m_right) {
        HKU_IF_RETURN(nodeInTree(node, tree->m_right.get()), true);
    }
    return false;
}

void IndicatorImp::printAllSubTrees(bool show_long_name) const {
    std::cout << "All sub trees:" << std::endl;
    vector<IndicatorImp *> trees = getAllSubTrees();
    std::sort(trees.begin(), trees.end(),
              [](IndicatorImp *a, IndicatorImp *b) { return treeSize(a) < treeSize(b); });
    for (size_t i = 0; i < trees.size(); i++) {
        std::cout << "-------------------------------------------------------------------------"
                  << std::endl;
        std::cout << "Tree " << i << " (size: " << treeSize(trees[i]) << ") :" << std::endl;
        trees[i]->printTree(show_long_name);
    }
}

void IndicatorImp::printLeaves(bool show_long_name) const {
    vector<IndicatorImpPtr> all_nodes;
    getAllSubNodes(all_nodes);
    std::unordered_set<IndicatorImpPtr> leaves;
    for (const auto &node : all_nodes) {
        if (node->isLeaf()) {
            leaves.insert(node);
        }
    }
    size_t ix = 0;
    for (const auto &leaf : leaves) {
        std::cout << "Leaf " << ix++ << ": " << (show_long_name ? leaf->long_name() : leaf->name())
                  << std::endl;
    }
}

} /* namespace hku */
