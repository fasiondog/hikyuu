#ifndef LOG4CPLUS_CONFIG_DEFINES_HXX
#define LOG4CPLUS_CONFIG_DEFINES_HXX

/* */
/* #undef LOG4CPLUS_HAVE_SYSLOG_H */

/* */
/* #undef LOG4CPLUS_HAVE_ARPA_INET_H */

/* */
/* #undef LOG4CPLUS_HAVE_NETINET_IN_H */

/* */
/* #undef LOG4CPLUS_HAVE_NETINET_TCP_H */

/* */
#define LOG4CPLUS_HAVE_SYS_TIMEB_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYS_TIME_H */

/* */
#define LOG4CPLUS_HAVE_SYS_TYPES_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_STAT_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYS_SYSCALL_H */

/* */
/* #undef LOG4CPLUS_HAVE_SYS_FILE_H */

/* */
#define LOG4CPLUS_HAVE_TIME_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYS_SOCKET_H */

/* */
/* #undef LOG4CPLUS_HAVE_NETDB_H */

/* */
/* #undef LOG4CPLUS_HAVE_UNISTD_H */

/* */
#define LOG4CPLUS_HAVE_FCNTL_H 1

/* */
#define LOG4CPLUS_HAVE_STDARG_H 1

/* */
#define LOG4CPLUS_HAVE_STDIO_H 1

/* */
#define LOG4CPLUS_HAVE_STDLIB_H 1

/* */
#define LOG4CPLUS_HAVE_ERRNO_H 1

/* */
#define LOG4CPLUS_HAVE_WCHAR_H 1

/* */
/* #undef LOG4CPLUS_HAVE_ICONV_H */

/* */
#define LOG4CPLUS_HAVE_LIMITS_H 1

/* */
#define LOG4CPLUS_HAVE_FTIME 1

/* */
/* #undef LOG4CPLUS_HAVE_GETADDRINFO */

/* */
/* #undef LOG4CPLUS_HAVE_GETHOSTBYNAME_R */

/* */
#define LOG4CPLUS_HAVE_GETPID 1

/* */
/* #undef LOG4CPLUS_HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the `clock_gettime' function. */
/* #undef LOG4CPLUS_HAVE_CLOCK_GETTIME */

/* Define to 1 if you have the `nanosleep' function. */
/* #undef LOG4CPLUS_HAVE_NANOSLEEP */

/* Define to 1 if you have the `clock_nanosleep' function. */
/* #undef LOG4CPLUS_HAVE_CLOCK_NANOSLEEP */

/* */
/* #undef LOG4CPLUS_HAVE_GMTIME_R */

/* */
/* #undef LOG4CPLUS_HAVE_HTONL */

/* */
/* #undef LOG4CPLUS_HAVE_HTONS */

/* */
/* #undef LOG4CPLUS_HAVE_LOCALTIME_R */

/* */
/* #undef LOG4CPLUS_HAVE_LSTAT */

/* */
/* #undef LOG4CPLUS_HAVE_FCNTL */

/* */
/* #undef LOG4CPLUS_HAVE_LOCKF */

/* */
/* #undef LOG4CPLUS_HAVE_FLOCK */

/* */
/* #undef LOG4CPLUS_HAVE_NTOHL */

/* */
/* #undef LOG4CPLUS_HAVE_NTOHS */

/* */
#define LOG4CPLUS_HAVE_STAT 1

/* Define if this is a single-threaded library. */
#undef LOG4CPLUS_SINGLE_THREADED

/* */
#undef LOG4CPLUS_USE_PTHREADS

/* Define for compilers/standard libraries that support more than just the "C"
   locale. */
#define LOG4CPLUS_WORKING_LOCALE 1

/* Define for C99 compilers/standard libraries that support more than just the
   "C" locale. */
/* #undef LOG4CPLUS_WORKING_C_LOCALE */

/* Define to int if undefined. */
#define socklen_t int

/* Defined for --enable-debugging builds. */
/* this is not used anywhere #undef LOG4CPLUS_DEBUGGING */

/* Defined if the compiler understands __declspec(export) or __attribute__((export)) construct. */
#define LOG4CPLUS_DECLSPEC_EXPORT __declspec(dllexport)

/* Defined if the compiler understands __declspec(import) or __attribute__((import)) construct. */
#define LOG4CPLUS_DECLSPEC_IMPORT __declspec(dllimport) /**/

/* Defined if the compiler understands
   __attribute__((visibility("hidden"))) construct. */
/* #undef LOG4CPLUS_DECLSPEC_PRIVATE */

/* */
#define LOG4CPLUS_HAVE_TLS_SUPPORT 1

/* */
#define LOG4CPLUS_THREAD_LOCAL_VAR __declspec(thread)

/* Defined if the host OS provides ENAMETOOLONG errno value. */
#define LOG4CPLUS_HAVE_ENAMETOOLONG 1

/* Defined if the compiler provides __sync_add_and_fetch(). */
/* #undef LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH */

/* Defined if the compiler provides __sync_sub_and_fetch(). */
/* #undef LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH */

/* Defined if the compiler provides C++11 <atomic> header and increment,
   decrement operations. */
/* #undef LOG4CPLUS_HAVE_CXX11_ATOMICS */

/* */
#define LOG4CPLUS_HAVE_C99_VARIADIC_MACROS 1

/* */
/* #undef LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS */

/* */
#define LOG4CPLUS_HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsnwprintf' function. */
/* #undef LOG4CPLUS_HAVE_VSNWPRINTF */

/* Define to 1 if you have the `_vsnwprintf' function. */
/* #undef LOG4CPLUS_HAVE__VSNWPRINTF */

/* */
#define LOG4CPLUS_HAVE__VSNPRINTF 1

/* Define to 1 if you have the `vfprintf_s' function. */
#define LOG4CPLUS_HAVE_VFPRINTF_S 1

/* Define to 1 if you have the `vfwprintf_s' function. */
#define LOG4CPLUS_HAVE_VFWPRINTF_S 1

/* Define to 1 if you have the `vsprintf_s' function. */
#define LOG4CPLUS_HAVE_VSPRINTF_S 1

/* Define to 1 if you have the `vswprintf_s' function. */
#define LOG4CPLUS_HAVE_VSWPRINTF_S 1

/* Define to 1 if you have the `_vsnprintf_s' function. */
#define LOG4CPLUS_HAVE__VSNPRINTF_S 1

/* Define to 1 if you have the `_vsnwprintf_s' function. */
#define LOG4CPLUS_HAVE__VSNWPRINTF_S 1

/* Defined if the compiler supports __FUNCTION__ macro. */
#define LOG4CPLUS_HAVE_FUNCTION_MACRO 1

/* Defined if the compiler supports __PRETTY_FUNCTION__ macro. */
/* #undef LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO */

/* Defined if the compiler supports __func__ symbol. */
/* #undef LOG4CPLUS_HAVE_FUNC_SYMBOL */

/* Define to 1 if you have the `mbstowcs' function. */
#define LOG4CPLUS_HAVE_MBSTOWCS 1

/* Define to 1 if you have the `wcstombs' function. */
#define LOG4CPLUS_HAVE_WCSTOMBS 1

/* Define to 1 if you have Linux style syscall(SYS_gettid). */
/* #undef LOG4CPLUS_HAVE_GETTID */

/* Define when iconv() is available. */
/* #undef LOG4CPLUS_WITH_ICONV */

/* Define to 1 if you have the `iconv' function. */
/* #undef LOG4CPLUS_HAVE_ICONV */

/* Define to 1 if you have the `iconv_close' function. */
/* #undef LOG4CPLUS_HAVE_ICONV_CLOSE */

/* Define to 1 if you have the `iconv_open' function. */
/* #undef LOG4CPLUS_HAVE_ICONV_OPEN */

#endif // LOG4CPLUS_CONFIG_DEFINES_HXX
