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

/*
 * H5api_adpt.h
 * Used for the HDF5 dll project
 * Created by Patrick Lu on 1/12/99
 */
#ifndef H5API_ADPT_H
#define H5API_ADPT_H

/* This will only be defined if HDF5 was built with CMake */
#ifdef H5_BUILT_AS_DYNAMIC_LIB

#if defined (hdf5_EXPORTS)
  #define _HDF5DLL_
#else
  #define _HDF5USEDLL_
#endif

#if defined (hdf5_test_EXPORTS)
  #define _HDF5TESTDLL_
#else
  #define _HDF5TESTUSEDLL_
#endif

#if defined (hdf5_tools_EXPORTS)
  #define _HDF5TOOLSDLL_
#else
  #define _HDF5TOOLSUSEDLL_
#endif

#if defined (hdf5_cpp_EXPORTS)
  #define HDF5_CPPDLL_EXPORTS
#else
  #define HDF5CPP_USEDLL
#endif

#if defined (hdf5_hl_EXPORTS)
  #define _HDF5_HLDLL_EXPORTS_
#else
  #define _HDF5USEHLDLL_
#endif

#if defined (hdf5_hl_cpp_EXPORTS)
  #define HDF5_HL_CPPDLL_EXPORTS
#else
  #define HDF5USE_HLCPPDLL
#endif

#if defined (hdf5_f90cstub_EXPORTS)
  #define HDF5FORT_CSTUB_DLL_EXPORTS
#else
  #define HDF5FORT_CSTUB_USEDLL
#endif

#if defined (hdf5_test_f90cstub_EXPORTS)
  #define HDF5FORTTEST_CSTUB_DLL_EXPORTS
#endif

#if defined (hdf5_hl_f90cstub_EXPORTS)
  #define HDF5_HL_F90CSTUBDLL_EXPORTS
#endif

#if defined(hdf5_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_DLL __declspec(dllexport)
    #define H5_DLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_DLL __attribute__ ((visibility("default")))
    #define H5_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_DLL __declspec(dllimport)
    #define H5_DLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_DLL __attribute__ ((visibility("default")))
    #define H5_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_DLL
  #define H5_DLL
  #define H5_DLLVAR extern
#endif /* _HDF5DLL_ */

#if defined(hdf5_test_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5TEST_DLL __declspec(dllexport)
    #define H5TEST_DLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5TEST_DLL __attribute__ ((visibility("default")))
    #define H5TEST_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5TEST_DLL __declspec(dllimport)
    #define H5TEST_DLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5TEST_DLL __attribute__ ((visibility("default")))
    #define H5TEST_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5TEST_DLL
  #define H5TEST_DLL
  #define H5TEST_DLLVAR extern
#endif /* H5TEST_DLL */

#if defined(hdf5_tools_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5TOOLS_DLL __declspec(dllexport)
    #define H5TOOLS_DLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5TOOLS_DLL __attribute__ ((visibility("default")))
    #define H5TOOLS_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5TOOLS_DLL __declspec(dllimport)
    #define H5TOOLS_DLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5TOOLS_DLL __attribute__ ((visibility("default")))
    #define H5TOOLS_DLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5TOOLS_DLL
  #define H5TOOLS_DLL
  #define H5TOOLS_DLLVAR extern
#endif /* H5TOOLS_DLL */

#if defined(hdf5_cpp_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_DLLCPP __declspec(dllexport)
    #define H5_DLLCPPVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_DLLCPP __attribute__ ((visibility("default")))
    #define H5_DLLCPPVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_DLLCPP __declspec(dllimport)
    #define H5_DLLCPPVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_DLLCPP __attribute__ ((visibility("default")))
    #define H5_DLLCPPVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_DLLCPP
  #define H5_DLLCPP
  #define H5_DLLCPPVAR extern
#endif /* H5_DLLCPP */

#if defined(hdf5_hl_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_HLDLL __declspec(dllexport)
    #define H5_HLDLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_HLDLL __attribute__ ((visibility("default")))
    #define H5_HLDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_HLDLL __declspec(dllimport)
    #define H5_HLDLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_HLDLL __attribute__ ((visibility("default")))
    #define H5_HLDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_HLDLL
  #define H5_HLDLL
  #define H5_HLDLLVAR extern
#endif /* H5_HLDLL */

#if defined(hdf5_hl_cpp_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_HLCPPDLL __declspec(dllexport)
    #define H5_HLCPPDLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_HLCPPDLL __attribute__ ((visibility("default")))
    #define H5_HLCPPDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_HLCPPDLL __declspec(dllimport)
    #define H5_HLCPPDLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_HLCPPDLL __attribute__ ((visibility("default")))
    #define H5_HLCPPDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_HLCPPDLL
  #define H5_HLCPPDLL
  #define H5_HLCPPDLLVAR extern
#endif /* H5_HLCPPDLL */

#if defined(hdf5_f90cstub_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_FCDLL __declspec(dllexport)
    #define H5_FCDLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_FCDLL __attribute__ ((visibility("default")))
    #define H5_FCDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_FCDLL __declspec(dllimport)
    #define H5_FCDLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_FCDLL __attribute__ ((visibility("default")))
    #define H5_FCDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_FCDLL
  #define H5_FCDLL
  #define H5_FCDLLVAR extern
#endif /* H5_FCDLL */

#if defined(hdf5_test_f90cstub_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_FCTESTDLL __declspec(dllexport)
    #define H5_FCTESTDLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_FCTESTDLL __attribute__ ((visibility("default")))
    #define H5_FCTESTDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define H5_FCTESTDLL __declspec(dllimport)
    #define H5_FCTESTDLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define H5_FCTESTDLL __attribute__ ((visibility("default")))
    #define H5_FCTESTDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef H5_FCTESTDLL
  #define H5_FCTESTDLL
  #define H5_FCTESTDLLVAR extern
#endif /* H5_FCTESTDLL */

#if defined(hdf5_hl_f90cstub_EXPORTS)
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define HDF5_HL_F90CSTUBDLL __declspec(dllexport)
    #define HDF5_HL_F90CSTUBDLLVAR extern __declspec(dllexport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define HDF5_HL_F90CSTUBDLL __attribute__ ((visibility("default")))
    #define HDF5_HL_F90CSTUBDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#else
  #if defined (_MSC_VER)  /* MSVC Compiler Case */
    #define HDF5_HL_F90CSTUBDLL __declspec(dllimport)
    #define HDF5_HL_F90CSTUBDLLVAR __declspec(dllimport)
  #elif (__GNUC__ >= 4)  /* GCC 4.x has support for visibility options */
    #define HDF5_HL_F90CSTUBDLL __attribute__ ((visibility("default")))
    #define HDF5_HL_F90CSTUBDLLVAR extern __attribute__ ((visibility("default")))
  #endif
#endif

#ifndef HDF5_HL_F90CSTUBDLL
  #define HDF5_HL_F90CSTUBDLL
  #define HDF5_HL_F90CSTUBDLLVAR extern
#endif /* HDF5_HL_F90CSTUBDLL */

#elif defined(H5_BUILT_AS_STATIC_LIB)
  #define H5_DLL
  #define H5_HLDLL
  #define H5_HLCPPDLL
  #define HDF5_HL_F90CSTUBDLL
  #define H5_DLLVAR extern
  #define H5_DLLCPP
  #define H5TEST_DLL
  #define H5TEST_DLLVAR extern
  #define H5TOOLS_DLL
  #define H5TOOLS_DLLVAR extern
  #define H5_FCDLL
  #define H5_FCDLLVAR extern
  #define H5_FCTESTDLL
  #define H5_FCTESTDLLVAR extern

#else
/* This is the original HDFGroup defined preprocessor code which should still work
 * with the VS projects that are maintained by "The HDF Group"
 * The Visual Studio project files will not be supported in the next major release of 1.10.
 */

#if defined(H5_HAVE_WIN32_API)

#if defined(_HDF5DLL_)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5_DLL __declspec(dllexport)
#define H5_DLLVAR extern __declspec(dllexport)
#elif defined(_HDF5USEDLL_)
#define H5_DLL __declspec(dllimport)
#define H5_DLLVAR __declspec(dllimport)
#else
#define H5_DLL
#define H5_DLLVAR extern
#endif /* _HDF5DLL_ */

#if defined(_HDF5TESTDLL_)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5TEST_DLL __declspec(dllexport)
#define H5TEST_DLLVAR extern __declspec(dllexport)
#elif defined(_HDF5TESTUSEDLL_)
#define H5TEST_DLL __declspec(dllimport)
#define H5TEST_DLLVAR __declspec(dllimport)
#else
#define H5TEST_DLL
#define H5TEST_DLLVAR extern
#endif /* _HDF5TESTDLL_ */

#if defined(_HDF5TOOLSDLL_)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5TOOLS_DLL __declspec(dllexport)
#define H5TOOLS_DLLVAR extern __declspec(dllexport)
#elif defined(_HDF5TOOLSUSEDLL_)
#define H5TOOLS_DLL __declspec(dllimport)
#define H5TOOLS_DLLVAR __declspec(dllimport)
#else
#define H5TOOLS_DLL
#define H5TOOLS_DLLVAR extern
#endif /* _HDF5TOOLSDLL_ */

#if defined(_HDF5_HLDLL_EXPORTS_)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5_HLDLL __declspec(dllexport)
#elif defined(_HDF5USEHLDLL_)
#define H5_HLDLL __declspec(dllimport)
#else
#define H5_HLDLL
#endif /* _HDF5_HLDLL_EXPORTS */

#if defined(HDF5_HL_CPPDLL_EXPORTS)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5_HLCPPDLL __declspec(dllexport)
#elif defined(HDF5USE_HLCPPDLL)
#define H5_HLCPPDLL __declspec(dllimport)
#else
#define H5_HLCPPDLL
#endif /*HDF5_HL_CPPDLL_EXPORTS*/

#if defined(HDF5_HL_F90CSTUBDLL_EXPORTS)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define HDF5_HL_F90CSTUBDLL __declspec(dllexport)
#elif defined(HDF5USE_HLF90CSTUBDLL)
#define HDF5_HL_F90CSTUBDLL __declspec(dllimport)
#else
#define HDF5_HL_F90CSTUBDLL
#endif /*HDF5_HL_F90CSTUBDLL_EXPORTS*/


#if defined(HDF5FORT_CSTUB_DLL_EXPORTS)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5_FCDLL __declspec(dllexport)
#define H5_FCDLLVAR extern __declspec(dllexport)
#elif defined(HDF5FORT_CSTUB_USEDLL)
#define H5_FCDLL __declspec(dllimport)
#define H5_FCDLLVAR __declspec(dllimport)
#else
#define H5_FCDLL
#define H5_FCDLLVAR extern
#endif /* _HDF5_FORTRANDLL_EXPORTS_ */

#if defined(HDF5FORTTEST_CSTUB_DLL_EXPORTS)
#pragma warning(disable: 4273)  /* Disable the dll linkage warnings */
#define H5_FCTESTDLL __declspec(dllexport)
#define H5_FCTESTDLLVAR extern __declspec(dllexport)
#elif defined(HDF5FORTTEST_CSTUB_USEDLL)
#define H5_FCTESTDLL __declspec(dllimport)
#define H5_FCTESTDLLVAR __declspec(dllimport)
#else
#define H5_FCTESTDLL
#define H5_FCTESTDLLVAR extern
#endif /* _HDF5_FORTRANDLL_EXPORTS_ */

/* Added to export or to import C++ APIs - BMR (02-15-2002) */
#if defined(HDF5_CPPDLL_EXPORTS) /* this name is generated at creation */
#define H5_DLLCPP __declspec(dllexport)
#elif defined(HDF5CPP_USEDLL)
#define H5_DLLCPP __declspec(dllimport)
#else
#define H5_DLLCPP
#endif /* HDF5_CPPDLL_EXPORTS */

#else /*H5_HAVE_WIN32_API*/
#define H5_DLL
#define H5_HLDLL
#define H5_HLCPPDLL
#define HDF5_HL_F90CSTUBDLL
#define H5_DLLVAR extern
#define H5_DLLCPP
#define H5TEST_DLL
#define H5TEST_DLLVAR extern
#define H5TOOLS_DLL
#define H5TOOLS_DLLVAR extern
#define H5_FCDLL
#define H5_FCDLLVAR extern
#define H5_FCTESTDLL
#define H5_FCTESTDLLVAR extern
#endif /*H5_HAVE_WIN32_API*/

#endif /* H5API_ADPT_H */

#endif /*  */
