/*
 * ioredirect.hpp
 *
 *  Created on: 2018年8月27日
 *      Author: fasiondog
 *      modified from https://github.com/pybind/pybind11
 */

 #ifndef IOREDIRECT_H_
#define IOREDIRECT_H_

#include <boost/python.hpp>
#include <streambuf>
#include <ostream>
#include <string>
#include <memory>
#include <iostream>

using namespace boost::python;

// Buffer that writes to Python instead of C++
class pythonbuf : public std::streambuf {
private:
    using traits_type = std::streambuf::traits_type;

    char d_buffer[1024];
    object pywrite;
    object pyflush;

    int overflow(int c) {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(c);
            pbump(1);
        }
        return sync() ? traits_type::not_eof(c) : traits_type::eof();
    }

    int sync() {
        if (pbase() != pptr()) {
            // This subtraction cannot be negative, so dropping the sign
            str line(pbase(), static_cast<size_t>(pptr() - pbase()));

            pywrite(line);
            pyflush();

            setp(pbase(), epptr());
        }
        return 0;
    }

public:
    pythonbuf(object pyostream)
        : pywrite(pyostream.attr("write")),
          pyflush(pyostream.attr("flush")) {
        setp(d_buffer, d_buffer + sizeof(d_buffer) - 1);
    }

    /// Sync before destroy
    ~pythonbuf() {
        sync();
    }
};

class ostream_redirect {
protected:
    std::streambuf *stdout_old;
    std::streambuf *stderr_old;
    std::ostream &stdout_ostream;
    std::ostream &stderr_ostream;
    pythonbuf stdout_buffer;
    pythonbuf stderr_buffer;

public:
    ostream_redirect()
    : stdout_ostream(std::cout),
      stderr_ostream(std::cerr),
      stdout_buffer(import("sys").attr("stdout")),
      stderr_buffer(import("sys").attr("stderr"))
    {
        stdout_old = stdout_ostream.rdbuf(&stdout_buffer);
        stderr_old = stderr_ostream.rdbuf(&stderr_buffer);
    }

    ~ostream_redirect() {
        stdout_ostream.rdbuf(stdout_old);
        stderr_ostream.rdbuf(stderr_old);
    }

};

#endif /** IOREDIRECT_H_ **/
