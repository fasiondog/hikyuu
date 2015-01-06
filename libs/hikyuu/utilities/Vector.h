/*
 * Vector.h
 *
 *  Created on: 2012-9-4
 *      Author: fasiondog
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <vector>
#include <boost/shared_ptr.hpp>

namespace hku {

using boost::shared_ptr;
using std::vector;

template <class T>
class Vector {
public:
    Vector() { m_data = shared_ptr< vector<T> >(new vector<T>); }

    Vector(const Vector<T>& x) {
        if( m_data != x.m_data) m_data = x.m_data;
    }

    Vector(const vector<T>& x) {
        m_data = shared_ptr< vector<T> >(new vector<T>);
        m_data->assign(x.begin(), x.end());
    }

    typedef typename vector<T>::iterator iterator;
    typedef typename vector<T>::const_iterator const_iterator;
    iterator begin() { return m_data->begin(); }
    const_iterator begin() const { return m_data->begin(); }
    iterator end() { return m_data->end(); }
    const_iterator end() const { return m_data->end(); }
    iterator rbegin() { return m_data->rbegin(); }
    const_iterator rbegin() const { return m_data->rbegin(); }
    iterator rend() { return m_data->rend(); }
    const_iterator rend() const { return m_data->rend(); }


    Vector<T> clone() const {
        Vector<T> result;
        shared_ptr< vector<T> > x = shared_ptr< vector<T> >(new vector<T>(*m_data));
        result.m_data = x;
        return result;
    }

    Vector<T>& operator=(const vector<T> &x) {
        m_data->assign(x.begin(), x.end());
        return *this;
    }

    Vector<T>& operator=(const Vector<T> &x) {
        if(this != &x) m_data = x.m_data;
        return *this;
    }

    size_t size() const { return m_data->size(); }
    bool empty() const { return m_data->empty(); }
    void reserve(size_t n) { m_data->reserve(n); }

    T& operator[](size_t n) { return (*m_data)[n]; }
    const T& operator[](size_t n) const { return (*m_data)[n]; }
    T& at(size_t n) { return m_data->at(n); }
    const T& at(size_t n) const { return m_data->at(n); }
    T& front() { return m_data->front(); }
    const T& front() const { return m_data->front(); }
    T& back() { return m_data->back(); }
    const T& back() const { return m_data->back(); }

    void push_back(const T& x) { m_data->push_back(x); }

    void clear() { m_data->clear(); }

private:
    shared_ptr< vector<T> > m_data;
};


} /* namespace */

#endif /* VECTOR_H_ */
