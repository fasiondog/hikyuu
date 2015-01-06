/* include/log4cplus/config.h.in.  Generated from configure.in by autoheader.  */

#ifndef LOG4CPLUS_CONFIG_H

#define LOG4CPLUS_CONFIG_H

/* Defined if the compiler supports C99 style variadic macros with
   __VA_ARGS__. */
/* #undef HAS_C99_VARIADIC_MACROS */

/* Defined if the compiler supports GNU style variadic macros. */
/* #undef HAS_GNU_VARIADIC_MACROS */

/* Define to 1 if you have the `clock_gettime' function. */
#undef HAVE_CLOCK_GETTIME

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* */
/* #undef HAVE_GETADDRINFO */

/* */
/* #undef HAVE_GETHOSTBYNAME_R */

/* Define to 1 if you have the `getpid' function. */
#define HAVE_GETPID 1

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the `gmtime_r' function. */
/* #undef HAVE_GMTIME_R */

/* Define to 1 if you have the `htonl' function. */
/* #undef HAVE_HTONL */

/* Define to 1 if you have the `htons' function. */
/* #undef HAVE_HTONS */

/* Define to 1 if you have the `iconv' function. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the `iconv_close' function. */
/* #undef HAVE_ICONV_CLOSE */

/* Define to 1 if you have the `iconv_open' function. */
/* #undef HAVE_ICONV_OPEN */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `advapi32' library (-ladvapi32). */
/* #undef HAVE_LIBADVAPI32 */

/* Define to 1 if you have the `libiconv' function. */
/* #undef HAVE_LIBICONV */

/* Define to 1 if you have the `libiconv_close' function. */
/* #undef HAVE_LIBICONV_CLOSE */

/* Define to 1 if you have the `libiconv_open' function. */
/* #undef HAVE_LIBICONV_OPEN */

/* Define to 1 if you have the `kernel32' library (-lkernel32). */
/* #undef HAVE_LIBKERNEL32 */

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `rt' library (-lrt). */
/* #undef HAVE_LIBRT */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `ws2_32' library (-lws2_32). */
/* #undef HAVE_LIBWS2_32 */

/* Define to 1 if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define to 1 if you have the `lstat' function. */
/* #undef HAVE_LSTAT */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `ntohl' function. */
/* #undef HAVE_NTOHL */

/* Define to 1 if you have the `ntohs' function. */
/* #undef HAVE_NTOHS */

/* Define if you have POSIX threads libraries and header files. */
#undef HAVE_PTHREAD

/* Define to 1 if you have the `stat' function. */
#define HAVE_STAT 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the `vfprintf_s' function. */
#define HAVE_VFPRINTF_S 1

/* Define to 1 if you have the `vfwprintf_s' function. */
#define HAVE_VFWPRINTF_S 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsprintf_s' function. */
#define HAVE_VSPRINTF_S 1

/* Define to 1 if you have the `vswprintf_s' function. */
#define HAVE_VSWPRINTF_S 1

/* Define to 1 if you have the `_vsnprintf' function. */
#define HAVE__VSNPRINTF 1

/* Define to 1 if you have the `_vsnprintf_s' function. */
#define HAVE__VSNPRINTF_S 1

/* Define to 1 if you have the `_vsnwprintf_s' function. */
#define HAVE__VSNWPRINTF_S 1

/* Defined if the compiler supports __FUNCTION__ macro. */
/* #undef HAVE___FUNCTION___MACRO */

/* Defined if the compiler supports __PRETTY_FUNCTION__ macro. */
/* #undef HAVE___PRETTY_FUNCTION___MACRO */

/* Defined if the compiler provides __sync_add_and_fetch(). */
/* #undef HAVE___SYNC_ADD_AND_FETCH */

/* Defined if the compiler provides __sync_sub_and_fetch(). */
/* #undef HAVE___SYNC_SUB_AND_FETCH */

/* Defined for --enable-debugging builds. */
#undef LOG4CPLUS_DEBUGGING

/* Defined if the compiler understands __declspec(dllimport) or
   __attribute__((visibility("default"))) construct. */
#define LOG4CPLUS_DECLSPEC_EXPORT __declspec(dllexport)

/* Defined if the compiler understands __declspec(dllexport) or construct. */
#define LOG4CPLUS_DECLSPEC_IMPORT __declspec(dllimport) /**/

/* */
#define LOG4CPLUS_HAVE_C99_VARIADIC_MACROS 1

/* */
/* #undef LOG4CPLUS_HAVE_CLOCK_GETTIME */

/* */
#define LOG4CPLUS_HAVE_ENAMETOOLONG 1

/* */
#define LOG4CPLUS_HAVE_ERRNO_H 1

/* */
#define LOG4CPLUS_HAVE_FTIME 1

/* */
#define LOG4CPLUS_HAVE_FUNCTION_MACRO 1

/* */
/* #undef LOG4CPLUS_HAVE_GETADDRINFO */

/* */
/* #undef LOG4CPLUS_HAVE_GETHOSTBYNAME_R */

/* */
#define LOG4CPLUS_HAVE_GETPID 1

/* */
/* #undef LOG4CPLUS_HAVE_GETTID */

/* */
/* #undef LOG4CPLUS_HAVE_GETTIMEOFDAY */

/* */
/* #undef LOG4CPLUS_HAVE_GMTIME_R */

/* */
/* #undef LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS */

/* */
/* #undef LOG4CPLUS_HAVE_HTONL */

/* */
/* #undef LOG4CPLUS_HAVE_HTONS */

/* */
/* #undef LOG4CPLUS_HAVE_ICONV */

/* */
/* #undef LOG4CPLUS_HAVE_ICONV_CLOSE */

/* */
/* #undef LOG4CPLUS_HAVE_ICONV_H */

/* */
/* #undef LOG4CPLUS_HAVE_ICONV_OPEN */

/* */
#define LOG4CPLUS_HAVE_LIMITS_H 1

/* */
/* #undef LOG4CPLUS_HAVE_LOCALTIME_R */

/* */
/* #undef LOG4CPLUS_HAVE_LSTAT */

/* */
/* #undef LOG4CPLUS_HAVE_NETDB_H */

/* */
/* #undef LOG4CPLUS_HAVE_NETINET_IN_H */

/* */
/* #undef LOG4CPLUS_HAVE_NETINET_TCP_H */

/* */
/* #undef LOG4CPLUS_HAVE_NTOHL */

/* */
/* #undef LOG4CPLUS_HAVE_NTOHS */

/* */
/* #undef LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO */

/* */
#define LOG4CPLUS_HAVE_STAT 1

/* */
#define LOG4CPLUS_HAVE_STDARG_H 1

/* */
#define LOG4CPLUS_HAVE_STDIO_H 1

/* */
#define LOG4CPLUS_HAVE_STDLIB_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYSLOG_H */

/* */
/* #undef LOG4CPLUS_HAVE_SYS_SOCKET_H */

/* */
#define LOG4CPLUS_HAVE_SYS_STAT_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYS_SYSCALL_H */

/* */
#define LOG4CPLUS_HAVE_SYS_TIMEB_H 1

/* */
/* #undef LOG4CPLUS_HAVE_SYS_TIME_H */

/* */
#define LOG4CPLUS_HAVE_SYS_TYPES_H 1

/* */
#define LOG4CPLUS_HAVE_TIME_H 1

/* */
/* #undef LOG4CPLUS_HAVE_UNISTD_H */

/* */
#define LOG4CPLUS_HAVE_VFPRINTF_S 1

/* */
#define LOG4CPLUS_HAVE_VFWPRINTF_S 1

/* */
#define LOG4CPLUS_HAVE_VSNPRINTF 1

/* */
#define LOG4CPLUS_HAVE_VSPRINTF_S 1

/* */
#define LOG4CPLUS_HAVE_VSWPRINTF_S 1

/* */
#define LOG4CPLUS_HAVE_WCHAR_H 1

/* */
#define LOG4CPLUS_HAVE__VSNPRINTF 1

/* */
#define LOG4CPLUS_HAVE__VSNPRINTF_S 1

/* */
#define LOG4CPLUS_HAVE__VSNWPRINTF_S 1

/* */
/* #undef LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH */

/* */
/* #undef LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH */

/* Define if this is a single-threaded library. */
#undef LOG4CPLUS_SINGLE_THREADED

/* */
#undef LOG4CPLUS_USE_PTHREADS

/* Define for compilers/standard libraries that support more than just the "C"
   locale. */
#undef LOG4CPLUS_WORKING_LOCALE

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#undef PTHREAD_CREATE_JOINABLE

/* Define to 1 if you have the ANSI C header files. Seems to be unused*/
#define STDC_HEADERS 1

/* Define to int if undefined. */
#define socklen_t int

#endif // LOG4CPLUS_CONFIG_H
