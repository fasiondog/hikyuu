#pragma once
#include <python.h>
#include <hikyuu/Log.h>

namespace hku {
class PyThreadStateLock {
public:
    PyThreadStateLock() {
        m_state = PyGILState_Ensure();
        HKU_INFO("gil state: {}", m_state);
    }

    ~PyThreadStateLock() {
        PyGILState_Release(m_state);
    }

private:
    PyGILState_STATE m_state;
};

}  // namespace hku