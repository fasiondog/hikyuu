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
 * This file contains private information about the H5P module
 */
#ifndef _H5Pprivate_H
#define _H5Pprivate_H

/* Include package's public header */
#include "H5Ppublic.h"

/* Private headers needed by this file */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Oprivate.h"		/* Object headers		  	*/

/**************************/
/* Library Private Macros */
/**************************/

/* ========  String creation property names ======== */
#define H5P_STRCRT_CHAR_ENCODING_NAME  "character_encoding"     /* Character set encoding for string */


/****************************/
/* Library Private Typedefs */
/****************************/

/* Forward declarations for anonymous H5P objects */
typedef struct H5P_genplist_t H5P_genplist_t;
typedef struct H5P_genclass_t H5P_genclass_t;

typedef enum H5P_plist_type_t {
    H5P_TYPE_USER              = 0,
    H5P_TYPE_ROOT              = 1,
    H5P_TYPE_OBJECT_CREATE     = 2,
    H5P_TYPE_FILE_CREATE       = 3,
    H5P_TYPE_FILE_ACCESS       = 4,
    H5P_TYPE_DATASET_CREATE    = 5,
    H5P_TYPE_DATASET_ACCESS    = 6,
    H5P_TYPE_DATASET_XFER      = 7,
    H5P_TYPE_FILE_MOUNT        = 8,
    H5P_TYPE_GROUP_CREATE      = 9,
    H5P_TYPE_GROUP_ACCESS      = 10,
    H5P_TYPE_DATATYPE_CREATE   = 11,
    H5P_TYPE_DATATYPE_ACCESS   = 12,
    H5P_TYPE_STRING_CREATE     = 13,
    H5P_TYPE_ATTRIBUTE_CREATE  = 14,
    H5P_TYPE_OBJECT_COPY       = 15,
    H5P_TYPE_LINK_CREATE       = 16,
    H5P_TYPE_LINK_ACCESS       = 17,
    H5P_TYPE_MAX_TYPE
} H5P_plist_type_t;

/*****************************/
/* Library Private Variables */
/*****************************/


/******************************/
/* Library Private Prototypes */
/******************************/

/* Package initialization routine */
H5_DLL herr_t H5P_init(void);

/* Internal versions of API routines */
H5_DLL herr_t H5P_close(void *_plist);
H5_DLL hid_t H5P_create_id(H5P_genclass_t *pclass, hbool_t app_ref);
H5_DLL hid_t H5P_copy_plist(const H5P_genplist_t *old_plist, hbool_t app_ref);
H5_DLL herr_t H5P_get(const H5P_genplist_t *plist, const char *name, void *value);
H5_DLL herr_t H5P_set(H5P_genplist_t *plist, const char *name, const void *value);
H5_DLL herr_t H5P_insert(H5P_genplist_t *plist, const char *name, size_t size,
    void *value, H5P_prp_set_func_t prp_set, H5P_prp_get_func_t prp_get,
    H5P_prp_delete_func_t prp_delete, H5P_prp_copy_func_t prp_copy,
    H5P_prp_compare_func_t prp_cmp, H5P_prp_close_func_t prp_close);
H5_DLL herr_t H5P_remove(hid_t plist_id, H5P_genplist_t *plist, const char *name);
H5_DLL htri_t H5P_exist_plist(const H5P_genplist_t *plist, const char *name);
H5_DLL char *H5P_get_class_name(H5P_genclass_t *pclass);
H5_DLL herr_t H5P_get_nprops_pclass(const H5P_genclass_t *pclass, size_t *nprops,
    hbool_t recurse);
H5_DLL hid_t H5P_get_driver(H5P_genplist_t *plist);
H5_DLL void * H5P_get_driver_info(H5P_genplist_t *plist);
H5_DLL herr_t H5P_set_driver(H5P_genplist_t *plist, hid_t new_driver_id,
            const void *new_driver_info);
H5_DLL herr_t H5P_set_vlen_mem_manager(H5P_genplist_t *plist,
        H5MM_allocate_t alloc_func, void *alloc_info, H5MM_free_t free_func,
        void *free_info);
H5_DLL herr_t H5P_is_fill_value_defined(const H5O_fill_t *fill,
        H5D_fill_value_t *status);
H5_DLL int H5P_fill_value_cmp(const void *value1, const void *value2,
    size_t size);
H5_DLL herr_t H5P_modify_filter(H5P_genplist_t *plist, H5Z_filter_t filter,
    unsigned flags, size_t cd_nelmts, const unsigned cd_values[]);
H5_DLL herr_t H5P_get_filter_by_id(H5P_genplist_t *plist, H5Z_filter_t id,
    unsigned int *flags, size_t *cd_nelmts, unsigned cd_values[],
    size_t namelen, char name[], unsigned *filter_config);

/* *SPECIAL* Don't make more of these! -QAK */
H5_DLL htri_t H5P_isa_class(hid_t plist_id, hid_t pclass_id);
H5_DLL H5P_genplist_t *H5P_object_verify(hid_t plist_id, hid_t pclass_id);

/* Private functions to "peek" at properties of a certain type */
H5_DLL unsigned H5P_peek_unsigned(H5P_genplist_t *plist, const char *name);
H5_DLL hid_t H5P_peek_hid_t(H5P_genplist_t *plist, const char *name);
H5_DLL void *H5P_peek_voidp(H5P_genplist_t *plist, const char *name);
H5_DLL size_t H5P_peek_size_t(H5P_genplist_t *plist, const char *name);

/* Private DCPL routines */
H5_DLL herr_t H5P_fill_value_defined(H5P_genplist_t *plist,
    H5D_fill_value_t *status);
H5_DLL herr_t H5P_get_fill_value(H5P_genplist_t *plist, H5T_t *type,
    void *value, hid_t dxpl_id);

#endif /* _H5Pprivate_H */

