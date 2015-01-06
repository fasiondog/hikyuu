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
 *              Monday, August  2, 1999
 *
 * Purpose:	The public header file for the "multi" driver.
 */
#ifndef H5FDmulti_H
#define H5FDmulti_H

#include "H5Ipublic.h"
#include "H5Ppublic.h"             /* Property lists */
#include "H5Fpublic.h"

#define H5FD_MULTI	(H5FD_multi_init())

#ifdef __cplusplus
extern "C" {
#endif
H5_DLL hid_t H5FD_multi_init(void);
H5_DLL void H5FD_multi_term(void);
H5_DLL herr_t H5Pset_fapl_multi(hid_t fapl_id, const H5FD_mem_t *memb_map,
			 const hid_t *memb_fapl, const char * const *memb_name,
			 const haddr_t *memb_addr, hbool_t relax);
H5_DLL herr_t H5Pget_fapl_multi(hid_t fapl_id, H5FD_mem_t *memb_map/*out*/,
			 hid_t *memb_fapl/*out*/, char **memb_name/*out*/,
			 haddr_t *memb_addr/*out*/, hbool_t *relax/*out*/);
H5_DLL herr_t H5Pset_dxpl_multi(hid_t dxpl_id, const hid_t *memb_dxpl);
H5_DLL herr_t H5Pget_dxpl_multi(hid_t dxpl_id, hid_t *memb_dxpl/*out*/);

H5_DLL herr_t H5Pset_fapl_split(hid_t fapl, const char *meta_ext,
			 hid_t meta_plist_id, const char *raw_ext,
			 hid_t raw_plist_id);
#ifdef __cplusplus
}
#endif

#endif
