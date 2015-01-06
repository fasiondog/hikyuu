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
 * Created:		H5WBprivate.h
 *			Jun 26 2007
 *			Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:		Private header for library accessible wrapped buffer routines.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5WBprivate_H
#define _H5WBprivate_H

/* Include package's public header */
/* #include "H5WBpublic.h" */

/* Private headers needed by this file */

/**************************/
/* Library Private Macros */
/**************************/


/****************************/
/* Library Private Typedefs */
/****************************/

/* Wrapped buffer info (forward decl - defined in H5WB.c) */
typedef struct H5WB_t H5WB_t;


/*****************************/
/* Library-private Variables */
/*****************************/


/***************************************/
/* Library-private Function Prototypes */
/***************************************/

/* General routines for wrapped buffer operations */
H5_DLL H5WB_t *H5WB_wrap(void *buf, size_t buf_size);
H5_DLL void *H5WB_actual(H5WB_t *wb, size_t need);
H5_DLL void *H5WB_actual_clear(H5WB_t *wb, size_t need);
H5_DLL herr_t H5WB_unwrap(H5WB_t *wb);

#endif /* _H5WBprivate_H */

