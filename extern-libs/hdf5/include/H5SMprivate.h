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
 * Programmer:	James Laird <jlaird@ncsa.uiuc.edu>
 *		Thursday, March 2, 2006
 *
 * Purpose:	This file contains private declarations for the H5SM
 *              shared object header messages module.
 */
#ifndef _H5SMprivate_H
#define _H5SMprivate_H

#include "H5Oprivate.h"		/* Object headers			*/
#include "H5Pprivate.h"		/* Property lists			*/

/**************************/
/* Library Private Macros */
/**************************/

/* Flags for the "defer_flags" argument to H5SM_try_share
 */
#define H5SM_DEFER              0x01u   /* Don't actually write shared message to index, heap; just update shared info */
#define H5SM_WAS_DEFERRED       0x02u   /* Message was previously updated by a call to H5SM_try_share with H5SM_DEFER */


/****************************/
/* Library Private Typedefs */
/****************************/

/* Forward references of package typedefs */
typedef struct H5SM_master_table_t H5SM_master_table_t;


/******************************/
/* Library Private Prototypes */
/******************************/

/* Generally useful shared message routines */
H5_DLL herr_t H5SM_init(H5F_t *f, H5P_genplist_t *fc_plist,
    const H5O_loc_t *ext_loc, hid_t dxpl_id);
H5_DLL htri_t H5SM_can_share(H5F_t *f, hid_t dxpl_id, H5SM_master_table_t *table,
    ssize_t *sohm_index_num, unsigned type_id, const void *mesg);
H5_DLL htri_t H5SM_try_share(H5F_t *f, hid_t dxpl_id, H5O_t *open_oh,
    unsigned defer_flags, unsigned type_id, void *mesg, unsigned *mesg_flags);
H5_DLL herr_t H5SM_delete(H5F_t *f, hid_t dxpl_id, H5O_t *open_oh,
    H5O_shared_t *sh_mesg);
H5_DLL herr_t H5SM_get_info(const H5O_loc_t *ext_loc, H5P_genplist_t *fc_plist,
    hid_t dxpl_id);
H5_DLL htri_t H5SM_type_shared(H5F_t *f, unsigned type_id, hid_t dxpl_id);
H5_DLL herr_t H5SM_get_fheap_addr(H5F_t *f, hid_t dxpl_id, unsigned type_id,
    haddr_t *fheap_addr);
H5_DLL herr_t H5SM_reconstitute(H5O_shared_t *sh_mesg, H5F_t *f,
    unsigned msg_type_id, H5O_fheap_id_t heap_id);
H5_DLL herr_t H5SM_get_refcount(H5F_t *f, hid_t dxpl_id, unsigned type_id,
    const H5O_shared_t *sh_mesg, hsize_t *ref_count);
H5_DLL herr_t H5SM_ih_size(H5F_t *f, hid_t dxpl_id, H5F_info_t *bh_info);


/* Debugging routines */
H5_DLL herr_t H5SM_table_debug(H5F_t *f, hid_t dxpl_id, haddr_t table_addr,
    FILE *stream, int indent, int fwidth, unsigned table_vers,
    unsigned num_indexes);
H5_DLL herr_t H5SM_list_debug(H5F_t *f, hid_t dxpl_id, haddr_t list_addr,
    FILE *stream, int indent, int fwidth, unsigned list_vers, size_t num_messages);

#endif /*_H5SMprivate_H*/

