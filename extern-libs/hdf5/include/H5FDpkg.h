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
 * Programmer:	Quincey Koziol <koziol@hdfgroup.org>
 *		Thursday, January  3, 2008
 *
 * Purpose:	This file contains declarations which are visible only within
 *		the H5FD package.  Source files outside the H5FD package should
 *		include H5FDprivate.h instead.
 */
#ifndef H5FD_PACKAGE
#error "Do not include this file outside the H5FD package!"
#endif

#ifndef _H5FDpkg_H
#define _H5FDpkg_H

/* Get package's private header */
#include "H5FDprivate.h"	/* File drivers				*/

/* Other private headers needed by this file */
#include "H5FLprivate.h"	/* Free lists                           */

/**************************/
/* Package Private Macros */
/**************************/


/****************************/
/* Package Private Typedefs */
/****************************/


/*****************************/
/* Package Private Variables */
/*****************************/


/******************************/
/* Package Private Prototypes */
/******************************/
H5_DLL herr_t H5FD_init(void);
H5_DLL haddr_t H5FD_alloc_real(H5FD_t *file, hid_t dxpl_id, H5FD_mem_t type,
    hsize_t size, haddr_t *align_addr, hsize_t *align_size);
H5_DLL herr_t H5FD_free_real(H5FD_t *file, hid_t dxpl_id, H5FD_mem_t type,
    haddr_t addr, hsize_t size);


/* Testing routines */
#ifdef H5FD_TESTING
#endif /* H5FD_TESTING */

#endif /* _H5FDpkg_H */

