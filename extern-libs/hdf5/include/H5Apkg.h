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
 * Programmer:  Quincey Koziol
 *              Monday, Apr 20
 *
 * Purpose:     This file contains declarations which are visible only within
 *              the H5A package.  Source files outside the H5A package should
 *              include H5Aprivate.h instead.
 */
#ifndef H5A_PACKAGE
#error "Do not include this file outside the H5A package!"
#endif

#ifndef _H5Apkg_H
#define _H5Apkg_H

/*
 * Define this to enable debugging.
 */
#ifdef NDEBUG
#  undef H5A_DEBUG
#endif

/* Get package's private header */
#include "H5Aprivate.h"

/* Other private headers needed by this file */
#include "H5B2private.h"	/* v2 B-trees				*/
#include "H5FLprivate.h"	/* Free Lists				*/
#include "H5HFprivate.h"	/* Fractal heaps			*/
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Sprivate.h"		/* Dataspace				*/
#include "H5Tprivate.h"		/* Datatype functions			*/


/**************************/
/* Package Private Macros */
/**************************/

/* This is the initial version, which does not have support for shared datatypes */
#define H5O_ATTR_VERSION_1	1

/* This version allows support for shared datatypes & dataspaces by adding a
 *      'flag' byte indicating when those components are shared.  This version
 *      also dropped the alignment on all the components.
 */
#define H5O_ATTR_VERSION_2	2

/* Add support for different character encodings of attribute names */
#define H5O_ATTR_VERSION_3      3

/* The latest version of the format.  Look through the 'encode', 'decode'
 *      and 'size' message callbacks for places to change when updating this.
 */
#define H5O_ATTR_VERSION_LATEST H5O_ATTR_VERSION_3


/****************************/
/* Package Private Typedefs */
/****************************/
/* Define the shared attribute structure */
typedef struct H5A_shared_t {
    uint8_t     version;    /* Version to encode attribute with */

    char        *name;      /* Attribute's name */
    H5T_cset_t  encoding;   /* Character encoding of attribute name */

    H5T_t       *dt;        /* Attribute's datatype */
    size_t      dt_size;    /* Size of datatype on disk */

    H5S_t       *ds;        /* Attribute's dataspace */
    size_t      ds_size;    /* Size of dataspace on disk */

    void        *data;      /* Attribute data (on a temporary basis) */
    size_t      data_size;  /* Size of data on disk */
    H5O_msg_crt_idx_t crt_idx;  /* Attribute's creation index in the object header */
    unsigned	nrefs;		/* Ref count for times this object is refered	*/
} H5A_shared_t;

/* Define the main attribute structure */
struct H5A_t {
    H5O_shared_t sh_loc;     /* Shared message info (must be first) */
    H5O_loc_t    oloc;       /* Object location for object attribute is on */
    hbool_t      obj_opened; /* Object header entry opened? */
    H5G_name_t   path;       /* Group hierarchy path */
    H5A_shared_t *shared;    /* Shared attribute information */
};

/* Typedefs for "dense" attribute storage */
/* (fractal heap & v2 B-tree info) */

/* Typedef for native 'name' field index records in the v2 B-tree */
/* (Keep 'id' field first so generic record handling in callbacks works) */
typedef struct H5A_dense_bt2_name_rec_t {
    H5O_fheap_id_t id;                  /* Heap ID for attribute */
    uint8_t flags;                      /* Object header message flags for attribute */
    H5O_msg_crt_idx_t corder;           /* 'creation order' field value */
    uint32_t hash;                      /* Hash of 'name' field value */
} H5A_dense_bt2_name_rec_t;

/* Typedef for native 'creation order' field index records in the v2 B-tree */
/* (Keep 'id' field first so generic record handling in callbacks works) */
typedef struct H5A_dense_bt2_corder_rec_t {
    H5O_fheap_id_t id;                  /* Heap ID for attribute */
    uint8_t flags;                      /* Object header message flags for attribute */
    H5O_msg_crt_idx_t corder;           /* 'creation order' field value */
} H5A_dense_bt2_corder_rec_t;

/* Define the 'found' callback function pointer for matching an attribute record in a v2 B-tree */
typedef herr_t (*H5A_bt2_found_t)(const H5A_t *attr, hbool_t *took_ownership, void *op_data);

/*
 * Common data exchange structure for dense attribute storage.  This structure
 * is passed through the v2 B-tree layer to the methods for the objects
 * to which the v2 B-tree points.
 */
typedef struct H5A_bt2_ud_common_t {
    /* downward */
    H5F_t       *f;                     /* Pointer to file that fractal heap is in */
    hid_t       dxpl_id;                /* DXPL for operation                */
    H5HF_t      *fheap;                 /* Fractal heap handle               */
    H5HF_t      *shared_fheap;          /* Fractal heap handle for shared messages */
    const char  *name;                  /* Name of attribute to compare      */
    uint32_t    name_hash;              /* Hash of name of attribute to compare */
    uint8_t     flags;                  /* Flags for attribute storage location */
    H5O_msg_crt_idx_t corder;           /* Creation order value of attribute to compare */
    H5A_bt2_found_t found_op;           /* Callback when correct attribute is found */
    void        *found_op_data;         /* Callback data when correct attribute is found */
} H5A_bt2_ud_common_t;

/*
 * Data exchange structure for dense attribute storage.  This structure is
 * passed through the v2 B-tree layer when inserting attributes.
 */
typedef struct H5A_bt2_ud_ins_t {
    /* downward */
    H5A_bt2_ud_common_t common;         /* Common info for B-tree user data (must be first) */
    H5O_fheap_id_t id;                  /* Heap ID of attribute to insert    */
} H5A_bt2_ud_ins_t;

/* Data structure to hold table of attributes for an object */
typedef struct {
    size_t      nattrs;         /* # of attributes in table */
    H5A_t       **attrs;        /* Pointer to array of attribute pointers */
} H5A_attr_table_t;


/*****************************/
/* Package Private Variables */
/*****************************/

/* Declare extern the free list for H5A_t's */
H5FL_EXTERN(H5A_t);

/* Declare the external free lists for H5A_shared_t's */
H5FL_EXTERN(H5A_shared_t);

/* Declare extern a free list to manage blocks of type conversion data */
H5FL_BLK_EXTERN(attr_buf);

/* The v2 B-tree class for indexing 'name' field on attributes */
H5_DLLVAR const H5B2_class_t H5A_BT2_NAME[1];

/* The v2 B-tree class for indexing 'creation order' field on attributes */
H5_DLLVAR const H5B2_class_t H5A_BT2_CORDER[1];


/******************************/
/* Package Private Prototypes */
/******************************/

/* Function prototypes for H5A package scope */
H5_DLL herr_t H5A_init(void);
H5_DLL hid_t H5A_create(const H5G_loc_t *loc, const char *name,
    const H5T_t *type, const H5S_t *space, hid_t acpl_id, hid_t dxpl_id);
H5_DLL H5A_t * H5A_open_by_name(const H5G_loc_t *loc, const char *obj_name,
    const char *attr_name, hid_t lapl_id, hid_t dxpl_id);
H5_DLL H5A_t *H5A_open_by_idx(const H5G_loc_t *loc, const char *obj_name,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t n, hid_t lapl_id, hid_t dxpl_id);
H5_DLL ssize_t H5A_get_name(H5A_t *attr, size_t buf_size, char *buf);
H5_DLL H5A_t *H5A_copy(H5A_t *new_attr, const H5A_t *old_attr);
H5_DLL herr_t H5A_get_info(const H5A_t *attr, H5A_info_t *ainfo);
H5_DLL herr_t H5A_free(H5A_t *attr);
H5_DLL herr_t H5A_close(H5A_t *attr);
H5_DLL htri_t H5A_get_ainfo(H5F_t *f, hid_t dxpl_id, H5O_t *oh, H5O_ainfo_t *ainfo);
H5_DLL herr_t H5A_set_version(const H5F_t *f, H5A_t *attr);

/* Attribute "dense" storage routines */
H5_DLL herr_t H5A_dense_create(H5F_t *f, hid_t dxpl_id, H5O_ainfo_t *ainfo);
H5_DLL H5A_t *H5A_dense_open(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    const char *name);
H5_DLL herr_t H5A_dense_insert(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    H5A_t *attr);
H5_DLL herr_t H5A_dense_write(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    H5A_t *attr);
H5_DLL herr_t H5A_dense_rename(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    const char *old_name, const char *new_name);
H5_DLL herr_t H5A_dense_iterate(H5F_t *f, hid_t dxpl_id, hid_t loc_id,
    const H5O_ainfo_t *ainfo, H5_index_t idx_type, H5_iter_order_t order,
    hsize_t skip, hsize_t *last_attr, const H5A_attr_iter_op_t *attr_op,
    void *op_data);
H5_DLL herr_t H5A_dense_remove(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    const char *name);
H5_DLL herr_t H5A_dense_remove_by_idx(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t n);
H5_DLL htri_t H5A_dense_exists(H5F_t *f, hid_t dxpl_id, const H5O_ainfo_t *ainfo,
    const char *name);
H5_DLL herr_t H5A_dense_delete(H5F_t *f, hid_t dxpl_id, H5O_ainfo_t *ainfo);


/* Attribute table operations */
H5_DLL herr_t H5A_compact_build_table(H5F_t *f, hid_t dxpl_id, H5O_t *oh,
    H5_index_t idx_type, H5_iter_order_t order, H5A_attr_table_t *atable);
H5_DLL herr_t H5A_dense_build_table(H5F_t *f, hid_t dxpl_id,
    const H5O_ainfo_t *ainfo, H5_index_t idx_type, H5_iter_order_t order,
    H5A_attr_table_t *atable);
H5_DLL herr_t H5A_attr_iterate_table(const H5A_attr_table_t *atable,
    hsize_t skip, hsize_t *last_attr, hid_t loc_id,
    const H5A_attr_iter_op_t *attr_op, void *op_data);
H5_DLL herr_t H5A_attr_release_table(H5A_attr_table_t *atable);

/* Attribute operations */
H5_DLL herr_t H5O_attr_create(const H5O_loc_t *loc, hid_t dxpl_id, H5A_t *attr);
H5_DLL H5A_t *H5O_attr_open_by_name(const H5O_loc_t *loc, const char *name,
    hid_t dxpl_id);
H5_DLL H5A_t *H5O_attr_open_by_idx(const H5O_loc_t *loc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, hid_t dxpl_id);
H5_DLL herr_t H5O_attr_update_shared(H5F_t *f, hid_t dxpl_id, H5O_t *oh,
        H5A_t *attr, H5O_shared_t *sh_mesg);
H5_DLL herr_t H5O_attr_write(const H5O_loc_t *loc, hid_t dxpl_id,
    H5A_t *attr);
H5_DLL herr_t H5O_attr_rename(const H5O_loc_t *loc, hid_t dxpl_id,
    const char *old_name, const char *new_name);
H5_DLL herr_t H5O_attr_remove(const H5O_loc_t *loc, const char *name,
    hid_t dxpl_id);
H5_DLL herr_t H5O_attr_remove_by_idx(const H5O_loc_t *loc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, hid_t dxpl_id);
H5_DLL htri_t H5O_attr_exists(const H5O_loc_t *loc, const char *name, hid_t dxpl_id);
#ifndef H5_NO_DEPRECATED_SYMBOLS
H5_DLL int H5O_attr_count(const H5O_loc_t *loc, hid_t dxpl_id);
#endif /* H5_NO_DEPRECATED_SYMBOLS */
H5_DLL H5A_t *H5A_attr_copy_file(const H5A_t *attr_src, H5F_t *file_dst, hbool_t *recompute_size,
    H5O_copy_t *cpy_info, hid_t dxpl_id);
H5_DLL herr_t H5A_attr_post_copy_file(const H5O_loc_t *src_oloc, const H5A_t *mesg_src,
    H5O_loc_t *dst_oloc, const H5A_t *mesg_dst, hid_t dxpl_id, H5O_copy_t *cpy_info);
H5_DLL herr_t H5A_dense_post_copy_file_all(const H5O_loc_t *src_oloc, const H5O_ainfo_t * ainfo_src,
    H5O_loc_t *dst_oloc, H5O_ainfo_t *ainfo_dst, hid_t dxpl_id, H5O_copy_t *cpy_info);


/* Testing functions */
#ifdef H5A_TESTING
H5_DLL htri_t H5A_is_shared_test(hid_t aid);
H5_DLL herr_t H5A_get_shared_rc_test(hid_t attr_id, hsize_t *ref_count);
#endif /* H5A_TESTING */

#endif /* _H5Apkg_H */

