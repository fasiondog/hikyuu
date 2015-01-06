/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Programmer:  Scott Wegner
 *              June 3, 2008
 *
 * Purpose: This file is used to map HDF macros to Windows functions.  This
 *          should get included H5private mappings, so as to override them.
 *          Any macro not mapped here, however, will receive a similar mapping
 *          inside H5private.h
 *
 */

#ifdef H5_HAVE_WIN32_API

typedef struct _stati64     h5_stat_t;
typedef __int64             h5_stat_size_t;

#define HDaccess(F,M)       _access(F,M)
#define HDclose(F)          _close(F)
#define HDdup(F)            _dup(F)
#define HDfdopen(N,S)       _fdopen(N,S)
#define HDfileno(F)         _fileno(F)
#define HDfstat(F,B)        _fstati64(F,B)
#define HDisatty(F)         _isatty(F)
#define HDgetcwd(S,Z)       _getcwd(S,Z)
#define HDgetdcwd(D,S,Z)    _getdcwd(D,S,Z)
#define HDgetdrive()        _getdrive()
#define HDlseek(F,O,W)      _lseeki64(F,O,W)
#define HDlstat(S,B)        _lstati64(S,B)
#define HDmkdir(S,M)        _mkdir(S)
#define HDoff_t             __int64
/* _O_BINARY must be set in Windows to avoid CR-LF <-> LF EOL
 * transformations when performing I/O.
 */
#define HDopen(S,F,M)       _open(S,F|_O_BINARY,M)
#define HDread(F,M,Z)       _read(F,M,Z)
#define HDsetvbuf(F,S,M,Z)  setvbuf(F,S,M,(Z>1?Z:2))
#define HDsleep(S)          Sleep(S*1000)
#define HDstat(S,B)         _stati64(S,B)
#define HDstrcasecmp(A,B)   _stricmp(A,B)
#define HDstrtoull(S,R,N)   _strtoui64(S,R,N)
#define HDstrdup(S)         _strdup(S)
#define HDsnprintf          _snprintf /*varargs*/
#define HDtzset()           _tzset()
#define HDunlink(S)         _unlink(S)
#define HDvsnprintf(S,N,FMT,A) _vsnprintf(S,N,FMT,A)
#define HDwrite(F,M,Z)      _write(F,M,Z)

#ifdef H5_HAVE_VISUAL_STUDIO
/*
 * The (void*) cast just avoids a compiler warning in H5_HAVE_VISUAL_STUDIO
 */
#define HDmemset(X,C,Z)     memset((void*)(X),C,Z)

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

#ifdef __cplusplus
        extern "C" {
#endif /* __cplusplus */
        H5_DLL int Wgettimeofday(struct timeval *tv, struct timezone *tz);
        H5_DLL char* Wgetlogin();
#ifdef __cplusplus
        }
#endif /* __cplusplus */
#define HDgettimeofday(V,Z) Wgettimeofday(V,Z)
#define HDgetlogin() Wgetlogin()
        
#endif /* H5_HAVE_VISUAL_STUDIO */

/* Non-POSIX functions */

/* Don't use actual pthread_self on Windows because the return
 * type cannot be cast as a ulong like other systems. */
#define HDpthread_self_ulong() ((unsigned long)GetCurrentThreadId())

#ifndef H5_HAVE_MINGW
#define HDftruncate(F,L)    _chsize_s(F,L)
#define HDfseek(F,O,W)      _fseeki64(F,O,W)
#endif
#endif /* H5_HAVE_WIN32_API */
