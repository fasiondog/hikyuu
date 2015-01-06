/*
 * This files has neccesary definitions to provide SZIP DLL 
 * support on Windows platfroms, both MSVC and CodeWarrior 
 * compilers
 */

#ifndef SZAPI_ADPT_H
#define SZAPI_ADPT_H

/* This will only be defined if szip was built with CMake shared libs*/
#ifdef SZ_BUILT_AS_DYNAMIC_LIB

#if defined (szip_EXPORTS)
  #define _SZDLL_
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define __SZ_DLL__ __declspec(dllexport)
/*    #define __DLLVARH425__ __declspec(dllexport)*/
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define __SZ_DLL__ __attribute__ ((visibility("default")))
/*    #define __DLLVARH425__ extern __attribute__ ((visibility("default"))) */
  #endif
#else
  #define _SZUSEDLL_
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define __SZ_DLL__ __declspec(dllimport)
/*    #define __DLLVARH425__ __declspec(dllimport)*/
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define __SZ_DLL__ __attribute__ ((visibility("default")))
/*    #define __DLLVARH425__ extern __attribute__ ((visibility("default"))) */
  #endif
#endif

#else /* SZ_BUILT_AS_DYNAMIC_LIB */
/* This is the original HDFGroup defined preprocessor code */

#if defined(WIN32) && !defined(__MWERKS__)
#if defined(_SZDLL_)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define __SZ_DLL__ __declspec(dllexport)
/*#define __DLLVARH425__ __declspec(dllexport)*/
#elif defined(_SZUSEDLL_)
#define __SZ_DLL__ __declspec(dllimport)
/*#define __DLLVARH425__ __declspec(dllimport)*/
#else
#define __SZ_DLL__
/*#define __DLLVARH425__ extern*/
#endif /* _SZDLL_ */

#else /*WIN32*/
#define __SZ_DLL__ extern
/*#define __DLLVAR__ extern*/
#endif

#endif /* SZ_BUILT_AS_DYNAMIC_LIB */

#endif /* H5API_ADPT_H */
