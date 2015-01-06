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
 * This file contains private information about the H5T module
 */
#ifndef _H5Tprivate_H
#define _H5Tprivate_H

/* Get package's public header */
#include "H5Tpublic.h"

/* Other public headers needed by this file */
#include "H5MMpublic.h"         /* Memory management                    */

/* Private headers needed by this file */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Gprivate.h"		/* Groups 			  	*/
#include "H5Rprivate.h"		/* References				*/

/* Macro for size of temporary buffers to contain a single element */
#define H5T_ELEM_BUF_SIZE       256

/* If the module using this macro is allowed access to the private variables, access them directly */
#ifdef H5T_PACKAGE
#define H5T_GET_SIZE(T)                 ((T)->shared->size)
#define H5T_GET_SHARED(T)               ((T)->shared)
#define H5T_GET_MEMBER_OFFSET(T, I)     ((T)->u.compnd.memb[I].offset)
#define H5T_GET_MEMBER_SIZE(T, I)       ((T)->u.compnd.memb[I].shared->size)
#else /* H5T_PACKAGE */
#define H5T_GET_SIZE(T)                 (H5T_get_size(T))
#define H5T_GET_SHARED(T)               (H5T_get_shared(T))
#define H5T_GET_MEMBER_OFFSET(T, I)     (H5T_get_member_offset((T), (I)))
#define H5T_GET_MEMBER_SIZE(T, I)       (H5T_get_member_size((T), (I)))
#endif /* H5T_PACKAGE */

/* Forward references of package typedefs (declared in H5Tpkg.h) */
typedef struct H5T_t H5T_t;
typedef struct H5T_stats_t H5T_stats_t;
typedef struct H5T_path_t H5T_path_t;

/* How to copy a datatype */
typedef enum H5T_copy_t {
    H5T_COPY_TRANSIENT,
    H5T_COPY_ALL,
    H5T_COPY_REOPEN
} H5T_copy_t;

/* Location of datatype information */
typedef enum {
    H5T_LOC_BADLOC =   0,  /* invalid datatype location */
    H5T_LOC_MEMORY,        /* data stored in memory */
    H5T_LOC_DISK,          /* data stored on disk */
    H5T_LOC_MAXLOC         /* highest value (Invalid as true value) */
} H5T_loc_t;

/* VL allocation information */
typedef struct {
    H5MM_allocate_t alloc_func; /* Allocation function */
    void *alloc_info;           /* Allocation information */
    H5MM_free_t free_func;      /* Free function */
    void *free_info;            /* Free information */
} H5T_vlen_alloc_info_t;

/* Structure for conversion callback property */
typedef struct H5T_conv_cb_t {
    H5T_conv_except_func_t      func;
    void*                       user_data;
} H5T_conv_cb_t;

/* Values for the optimization of compound data reading and writing.  They indicate
 * whether the fields of the source and destination are subset of each other and
 * there is no conversion needed.
 */
typedef enum {
    H5T_SUBSET_BADVALUE = -1,   /* Invalid value */
    H5T_SUBSET_FALSE = 0,       /* Source and destination aren't subset of each other */
    H5T_SUBSET_SRC,             /* Source is the subset of dest and no conversion is needed */
    H5T_SUBSET_DST,             /* Dest is the subset of source and no conversion is needed */
    H5T_SUBSET_CAP              /* Must be the last value */
} H5T_subset_t;

typedef struct H5T_subset_info_t {
    H5T_subset_t    subset;     /* See above */
    size_t          copy_size;  /* Size in bytes, to copy for each element */
} H5T_subset_info_t;

/* Forward declarations for prototype arguments */
struct H5O_t;

/* The native endianess of the platform */
H5_DLLVAR H5T_order_t H5T_native_order_g;

/* Private functions */
H5_DLL herr_t H5TN_init_interface(void);
H5_DLL herr_t H5T_init(void);
H5_DLL H5T_t *H5T_copy(H5T_t *old_dt, H5T_copy_t method);
H5_DLL herr_t H5T_lock(H5T_t *dt, hbool_t immutable);
H5_DLL herr_t H5T_close(H5T_t *dt);
H5_DLL H5T_t *H5T_get_super(const H5T_t *dt);
H5_DLL H5T_class_t H5T_get_class(const H5T_t *dt, htri_t internal);
H5_DLL htri_t H5T_detect_class(const H5T_t *dt, H5T_class_t cls, hbool_t from_api);
H5_DLL size_t H5T_get_size(const H5T_t *dt);
H5_DLL int    H5T_cmp(const H5T_t *dt1, const H5T_t *dt2, hbool_t superset);
H5_DLL herr_t H5T_debug(const H5T_t *dt, FILE * stream);
H5_DLL struct H5O_loc_t *H5T_oloc(H5T_t *dt);
H5_DLL H5G_name_t *H5T_nameof(H5T_t *dt);
H5_DLL htri_t H5T_is_immutable(const H5T_t *dt);
H5_DLL htri_t H5T_is_named(const H5T_t *dt);
H5_DLL htri_t H5T_is_relocatable(const H5T_t *dt);
H5_DLL H5T_path_t *H5T_path_find(const H5T_t *src, const H5T_t *dst,
    const char *name, H5T_conv_t func, hid_t dxpl_id, hbool_t is_api);
H5_DLL hbool_t H5T_path_noop(const H5T_path_t *p);
H5_DLL H5T_bkg_t H5T_path_bkg(const H5T_path_t *p);
H5_DLL H5T_subset_info_t *H5T_path_compound_subset(const H5T_path_t *p);
H5_DLL herr_t H5T_convert(H5T_path_t *tpath, hid_t src_id, hid_t dst_id,
    size_t nelmts, size_t buf_stride, size_t bkg_stride, void *buf, void *bkg,
    hid_t dset_xfer_plist);
H5_DLL herr_t H5T_vlen_reclaim(void *elem, hid_t type_id, unsigned ndim, const hsize_t *point, void *_op_data);
H5_DLL herr_t H5T_vlen_reclaim_elmt(void *elem, H5T_t *dt, hid_t dxpl_id);
H5_DLL herr_t H5T_vlen_get_alloc_info(hid_t dxpl_id, H5T_vlen_alloc_info_t **vl_alloc_info);
H5_DLL htri_t H5T_set_loc(H5T_t *dt, H5F_t *f, H5T_loc_t loc);
H5_DLL htri_t H5T_is_sensible(const H5T_t *dt);
H5_DLL uint32_t H5T_hash(H5F_t * file, const H5T_t *dt);
H5_DLL herr_t H5T_set_latest_version(H5T_t *dt);
H5_DLL herr_t H5T_patch_file(H5T_t *dt, H5F_t *f);
H5_DLL htri_t H5T_is_variable_str(const H5T_t *dt);

/* Reference specific functions */
H5_DLL H5R_type_t H5T_get_ref_type(const H5T_t *dt);

/* Operations on named datatypes */
H5_DLL H5T_t *H5T_open(const H5G_loc_t *loc, hid_t dxpl_id);
H5_DLL htri_t H5T_committed(const H5T_t *type);
H5_DLL int H5T_link(const H5T_t *type, int adjust, hid_t dxpl_id);
H5_DLL herr_t H5T_update_shared(H5T_t *type);

/* Field functions (for both compound & enumerated types) */
H5_DLL int H5T_get_nmembers(const H5T_t *dt);
H5_DLL H5T_t *H5T_get_member_type(const H5T_t *dt, unsigned membno, H5T_copy_t method);
H5_DLL size_t H5T_get_member_offset(const H5T_t *dt, unsigned membno);

/* Atomic functions */
H5_DLL H5T_order_t H5T_get_order(const H5T_t *dt);
H5_DLL size_t H5T_get_precision(const H5T_t *dt);
H5_DLL int H5T_get_offset(const H5T_t *dt);

/* Fixed-point functions */
H5_DLL H5T_sign_t H5T_get_sign(H5T_t const *dt);

#endif /* _H5Tprivate_H */

