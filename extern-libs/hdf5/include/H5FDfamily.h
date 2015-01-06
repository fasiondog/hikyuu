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
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Monday, August  4, 1999
 *
 * Purpose:	The public header file for the family driver.
 */
#ifndef H5FDfamily_H
#define H5FDfamily_H

#include "H5Ipublic.h"

#define H5FD_FAMILY	(H5FD_family_init())

#ifdef __cplusplus
extern "C" {
#endif

H5_DLL hid_t H5FD_family_init(void);
H5_DLL void H5FD_family_term(void);
H5_DLL herr_t H5Pset_fapl_family(hid_t fapl_id, hsize_t memb_size,
			  hid_t memb_fapl_id);
H5_DLL herr_t H5Pget_fapl_family(hid_t fapl_id, hsize_t *memb_size/*out*/,
			  hid_t *memb_fapl_id/*out*/);

#ifdef __cplusplus
}
#endif

#endif
