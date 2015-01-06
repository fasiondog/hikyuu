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
 * Programmer: Quincey Koziol <koziol@hdfgroup.org>
 *             Thursday, September 13, 2007
 *
 * Purpose:     This file contains declarations which are visible
 *              only within the H5R package. Source files outside the
 *              H5R package should include H5Rprivate.h instead.
 */
#ifndef H5R_PACKAGE
#error "Do not include this file outside the H5R package!"
#endif

#ifndef _H5Rpkg_H
#define _H5Rpkg_H

/* Get package's private header */
#include "H5Rprivate.h"

/* Other private headers needed by this file */
#include "H5Fprivate.h"         /* File access				*/

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

/* General functions */
H5_DLL herr_t H5R_init(void);
H5_DLL herr_t H5R_get_obj_type(H5F_t *file, hid_t dxpl_id, H5R_type_t ref_type,
    const void *_ref, H5O_type_t *obj_type);


#endif /* _H5Rpkg_H */

