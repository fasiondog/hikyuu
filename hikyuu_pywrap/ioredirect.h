/*
    pybind11/iostream.h -- Tools to assist with redirecting cout and cerr to Python

    Copyright (c) 2017 Henry F. Schreiner

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/
/*
 *      modified by: fasiondog
 *      modified from https://github.com/pybind/pybind11/include/pybind11/iostream.h
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

class scoped_ostream_redirect {
protected:
    std::streambuf *old;
    std::ostream &costream;
    pythonbuf buffer;

public:
    scoped_ostream_redirect(
            std::ostream &costream = std::cout,
            object pyostream = import("sys").attr("stdout"))
        : costream(costream), buffer(pyostream) {
        old = costream.rdbuf(&buffer);
    }

    ~scoped_ostream_redirect() {
        costream.rdbuf(old);
    }

    scoped_ostream_redirect(const scoped_ostream_redirect &) = delete;
    scoped_ostream_redirect(scoped_ostream_redirect &&other) = default;
    scoped_ostream_redirect &operator=(const scoped_ostream_redirect &) = delete;
    scoped_ostream_redirect &operator=(scoped_ostream_redirect &&) = delete;
};

class scoped_estream_redirect : public scoped_ostream_redirect {
public:
    scoped_estream_redirect(
            std::ostream &costream = std::cerr,
            object pyostream = import("sys").attr("stderr"))
        : scoped_ostream_redirect(costream,pyostream) {}
};

class OstreamRedirect {
    bool do_stdout_;
    bool do_stderr_;
    std::unique_ptr<scoped_ostream_redirect> redirect_stdout;
    std::unique_ptr<scoped_estream_redirect> redirect_stderr;

public:
    //OstreamRedirect() 
    //    : do_stdout_(false), do_stderr_(false) {}
    OstreamRedirect(bool do_stdout=true, bool do_stderr=true)
        : do_stdout_(do_stdout), do_stderr_(do_stderr) {}

    OstreamRedirect(const OstreamRedirect& src) {
        do_stdout_ = src.do_stdout_;
        do_stderr_ = src.do_stderr_;
    }

    ~OstreamRedirect() {
        redirect_stdout.reset();
        redirect_stderr.reset();
        if (do_stdout_) {
            std::cout << "redirected std::cout has been returned" << std::endl;
        }
        if (do_stderr_) {
            std::cout << "redirected std::cerr has been returned" << std::endl;
        }
    }

    void enter() {
        if (do_stdout_) {
            redirect_stdout.reset(new scoped_ostream_redirect());
            std::cout << "std::cout are redirected to python::stdout" << std::endl;
        }
        if (do_stderr_) {
            redirect_stderr.reset(new scoped_estream_redirect());
            std::cout << "std::cerr are redirected to python::stderr" << std::endl;
        }
    }

    void exit() {
        redirect_stdout.reset();
        redirect_stderr.reset();
        if (do_stdout_) {
            std::cout << "redirected std::cout has been returned" << std::endl;
        }
        if (do_stderr_) {
            std::cout << "redirected std::cerr has been returned" << std::endl;
        }
    }
};

#endif /** IOREDIRECT_H_ **/
