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

/*-------------------------------------------------------------------------
 *
 * Created:		H5TSprivate.h
 *			May 2 2000
 *			Chee Wai LEE
 *
 * Purpose:		Private non-prototype header.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifndef H5TSprivate_H_
#define H5TSprivate_H_

/* Public headers needed by this file */
#ifdef LATER
#include "H5TSpublic.h"		/*Public API prototypes */
#endif /* LATER */

#ifdef H5_HAVE_WIN_THREADS

/* Library level data structures */

/* Mutexes, Threads, and Attributes */
typedef struct H5TS_mutex_struct {
	CRITICAL_SECTION CriticalSection;
} H5TS_mutex_t;
typedef CRITICAL_SECTION H5TS_mutex_simple_t;
typedef HANDLE H5TS_thread_t;
typedef HANDLE H5TS_attr_t;
typedef DWORD H5TS_key_t;
typedef INIT_ONCE H5TS_once_t;

/* Defines */
/* not used on windows side, but need to be defined to something */
#define H5TS_SCOPE_SYSTEM 0
#define H5TS_SCOPE_PROCESS 0

/* Functions */
#define H5TS_get_thread_local_value(key)	TlsGetValue( key )
#define H5TS_set_thread_local_value(key, value)	TlsSetValue( key, value )
#define H5TS_attr_init(attr_ptr) 0
#define H5TS_attr_setscope(attr_ptr, scope) 0
#define H5TS_attr_destroy(attr_ptr) 0
#define H5TS_wait_for_thread(thread) WaitForSingleObject(thread, INFINITE)
#define H5TS_mutex_init(mutex) InitializeCriticalSection(mutex)
#define H5TS_mutex_lock_simple(mutex) EnterCriticalSection(mutex)
#define H5TS_mutex_unlock_simple(mutex) LeaveCriticalSection(mutex)

H5_DLL BOOL CALLBACK 
H5TS_win32_first_thread_init(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContex);

#else /* H5_HAVE_WIN_THREADS */

/* Library level data structures */

/* Mutexes, Threads, and Attributes */
typedef struct H5TS_mutex_struct {
    pthread_t owner_thread;		/* current lock owner */
    pthread_mutex_t atomic_lock;	/* lock for atomicity of new mechanism */
    pthread_cond_t cond_var;		/* condition variable */
    unsigned int lock_count;
} H5TS_mutex_t;
typedef pthread_t      H5TS_thread_t;
typedef pthread_attr_t H5TS_attr_t;
typedef pthread_mutex_t H5TS_mutex_simple_t;
typedef pthread_key_t  H5TS_key_t;
typedef pthread_once_t H5TS_once_t;

/* Scope Definitions */
#define H5TS_SCOPE_SYSTEM PTHREAD_SCOPE_SYSTEM
#define H5TS_SCOPE_PROCESS PTHREAD_SCOPE_PROCESS

/* Functions */
#define H5TS_get_thread_local_value(key)	pthread_getspecific( key )
#define H5TS_set_thread_local_value(key, value)	pthread_setspecific( key, value )
#define H5TS_attr_init(attr_ptr) pthread_attr_init((attr_ptr))
#define H5TS_attr_setscope(attr_ptr, scope) pthread_attr_setscope(attr_ptr, scope)
#define H5TS_attr_destroy(attr_ptr) pthread_attr_destroy(attr_ptr)
#define H5TS_wait_for_thread(thread) pthread_join(thread, NULL)
#define H5TS_mutex_init(mutex) pthread_mutex_init(mutex, NULL)
#define H5TS_mutex_lock_simple(mutex) pthread_mutex_lock(mutex)
#define H5TS_mutex_unlock_simple(mutex) pthread_mutex_unlock(mutex)

#endif /* H5_HAVE_WIN_THREADS */

/* External global variables */
extern H5TS_once_t H5TS_first_init_g;
extern H5TS_key_t H5TS_errstk_key_g;
extern H5TS_key_t H5TS_funcstk_key_g;

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif	/* c_plusplus || __cplusplus */

H5_DLL void   H5TS_pthread_first_thread_init(void);
H5_DLL herr_t H5TS_mutex_lock(H5TS_mutex_t *mutex);
H5_DLL herr_t H5TS_mutex_unlock(H5TS_mutex_t *mutex);
H5_DLL herr_t H5TS_cancel_count_inc(void);
H5_DLL herr_t H5TS_cancel_count_dec(void);
H5_DLL H5TS_thread_t H5TS_create_thread(void * func, H5TS_attr_t * attr, void *udata);



#if defined c_plusplus || defined __cplusplus
}
#endif	/* c_plusplus || __cplusplus */

#endif	/* H5TSprivate_H_ */
