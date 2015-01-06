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
 * Created:             H5Gprivate.h
 *                      Jul 11 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             Library-visible declarations.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5Gprivate_H
#define _H5Gprivate_H

/* Include package's public header */
#include "H5Gpublic.h"

/* Private headers needed by this file */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Bprivate.h"		/* B-trees				*/
#include "H5Fprivate.h"		/* File access				*/
#include "H5RSprivate.h"        /* Reference-counted strings            */

/*
 * Define this to enable debugging.
 */
#ifdef NDEBUG
#  undef H5G_DEBUG
#endif

/*
 * The disk size for a symbol table entry...
 */
#define H5G_SIZEOF_SCRATCH      16
#define H5G_SIZEOF_ENTRY(F)                                                   \
   (H5F_SIZEOF_SIZE(F) +        /*offset of name into heap              */    \
    H5F_SIZEOF_ADDR(F) +        /*address of object header              */    \
    4 +                         /*entry type                            */    \
    4 +				/*reserved				*/    \
    H5G_SIZEOF_SCRATCH)         /*scratch pad space                     */

/* ========= Group Creation properties ============ */

/* Defaults for link info values */
#define H5G_CRT_LINFO_TRACK_CORDER              FALSE
#define H5G_CRT_LINFO_INDEX_CORDER              FALSE
#define H5G_CRT_LINFO_NLINKS                    0
#define H5G_CRT_LINFO_MAX_CORDER                0
#define H5G_CRT_LINFO_LINK_FHEAP_ADDR           HADDR_UNDEF
#define H5G_CRT_LINFO_NAME_BT2_ADDR             HADDR_UNDEF
#define H5G_CRT_LINFO_CORDER_BT2_ADDR           HADDR_UNDEF

/* Definitions for link info settings */
#define H5G_CRT_LINK_INFO_NAME                  "link info"
#define H5G_CRT_LINK_INFO_SIZE                  sizeof(H5O_linfo_t)
#define H5G_CRT_LINK_INFO_DEF                   {H5G_CRT_LINFO_TRACK_CORDER, \
                                                    H5G_CRT_LINFO_INDEX_CORDER, \
                                                    H5G_CRT_LINFO_MAX_CORDER, \
                                                    H5G_CRT_LINFO_CORDER_BT2_ADDR, \
                                                    H5G_CRT_LINFO_NLINKS, \
                                                    H5G_CRT_LINFO_LINK_FHEAP_ADDR, \
                                                    H5G_CRT_LINFO_NAME_BT2_ADDR \
                                                }

/* Defaults for group info values */
#define H5G_CRT_GINFO_LHEAP_SIZE_HINT           0
#define H5G_CRT_GINFO_STORE_LINK_PHASE_CHANGE   FALSE
#define H5G_CRT_GINFO_MAX_COMPACT               8
#define H5G_CRT_GINFO_MIN_DENSE                 6
#define H5G_CRT_GINFO_STORE_EST_ENTRY_INFO      FALSE
#define H5G_CRT_GINFO_EST_NUM_ENTRIES           4
#define H5G_CRT_GINFO_EST_NAME_LEN              8

/* Definitions for group info settings */
#define H5G_CRT_GROUP_INFO_NAME                 "group info"
#define H5G_CRT_GROUP_INFO_SIZE                 sizeof(H5O_ginfo_t)
#define H5G_CRT_GROUP_INFO_DEF                  {H5G_CRT_GINFO_LHEAP_SIZE_HINT, \
                                                    H5G_CRT_GINFO_STORE_LINK_PHASE_CHANGE, \
                                                    H5G_CRT_GINFO_MAX_COMPACT, \
                                                    H5G_CRT_GINFO_MIN_DENSE, \
                                                    H5G_CRT_GINFO_STORE_EST_ENTRY_INFO, \
                                                    H5G_CRT_GINFO_EST_NUM_ENTRIES, \
                                                    H5G_CRT_GINFO_EST_NAME_LEN \
                                                }

/* If the module using this macro is allowed access to the private variables, access them directly */
#ifdef H5G_PACKAGE
#define H5G_MOUNTED(G)              ((G)->shared->mounted)
#else /* H5G_PACKAGE */
#define H5G_MOUNTED(G)              (H5G_mounted(G))
#endif /* H5G_PACKAGE */

/*
 * During name lookups (see H5G_traverse()) we sometimes want information about
 * a symbolic link or a mount point.  The normal operation is to follow the
 * symbolic link or mount point and return information about its target.
 */
#define H5G_TARGET_NORMAL	0x0000
#define H5G_TARGET_SLINK	0x0001
#define H5G_TARGET_MOUNT	0x0002
#define H5G_TARGET_UDLINK	0x0004
#define H5G_TARGET_EXISTS	0x0008
#define H5G_CRT_INTMD_GROUP	0x0010

/* Type of operation being performed for call to H5G_name_replace() */
typedef enum {
    H5G_NAME_MOVE = 0,          /* H5*move call    */
    H5G_NAME_DELETE,            /* H5Ldelete call  */
    H5G_NAME_MOUNT,             /* H5Fmount call   */
    H5G_NAME_UNMOUNT            /* H5Funmount call */
} H5G_names_op_t;

/* Status returned from traversal callbacks; whether the object location
 * or group location need to be closed */
#define H5G_OWN_NONE 0
#define H5G_OWN_OBJ_LOC 1
#define H5G_OWN_GRP_LOC 2
#define H5G_OWN_BOTH 3
typedef int H5G_own_loc_t;

/* Structure to store information about the name an object was opened with */
typedef struct {
    H5RS_str_t  *full_path_r;           /* Path to object, as seen from root of current file mounting hierarchy */
    H5RS_str_t  *user_path_r;           /* Path to object, as opened by user */
    unsigned    obj_hidden;             /* Whether the object is visible in group hier. */
} H5G_name_t;

/* Forward declarations (for prototypes & struct definitions) */
struct H5O_loc_t;
struct H5O_link_t;

/*
 * The "location" of an object in a group hierarchy.  This points to an object
 * location and a group hierarchy path for the object.
 */
typedef struct {
    struct H5O_loc_t *oloc;             /* Object header location            */
    H5G_name_t *path;                   /* Group hierarchy path              */
} H5G_loc_t;

/* Typedef for path traversal operations */
/* grp_loc is the location of the group in which the targeted object is located.
 * name is the last component of the object's name
 * lnk is the link between the group and the object
 * obj_loc is the target of the traversal (or NULL if the object doesn't exist)
 * operator_data is whatever udata was supplied when H5G_traverse was called
 * own_loc should be set to H5G_OWN_OBJ_LOC if this callback takes ownership of obj_loc,
 * H5G_OWN_GRP_LOC if it takes ownership of grp_loc, and H5G_OWN_NONE if obj_loc and
 * grp_loc need to be deleted.
 */
typedef herr_t (*H5G_traverse_t)(H5G_loc_t *grp_loc/*in*/, const char *name,
    const struct H5O_link_t *lnk/*in*/, H5G_loc_t *obj_loc/*out*/, void *operator_data/*in,out*/,
    H5G_own_loc_t *own_loc/*out*/);

/* Describe kind of callback to make for each link */
typedef enum H5G_link_iterate_op_type_t {
#ifndef H5_NO_DEPRECATED_SYMBOLS
    H5G_LINK_OP_OLD,                /* "Old" application callback */
#endif /* H5_NO_DEPRECATED_SYMBOLS */
    H5G_LINK_OP_NEW                 /* "New" application callback */
} H5G_link_iterate_op_type_t;

typedef struct {
    H5G_link_iterate_op_type_t op_type;
    union {
#ifndef H5_NO_DEPRECATED_SYMBOLS
        H5G_iterate_t op_old;           /* "Old" application callback for each link */
#endif /* H5_NO_DEPRECATED_SYMBOLS */
        H5L_iterate_t op_new;           /* "New" application callback for each link */
    } op_func;
} H5G_link_iterate_t;

typedef struct H5G_t H5G_t;
typedef struct H5G_shared_t H5G_shared_t;
typedef struct H5G_entry_t H5G_entry_t;

/*
 * Library prototypes...  These are the ones that other packages routinely
 * call.
 */
H5_DLL struct H5O_loc_t *H5G_oloc(H5G_t *grp);
H5_DLL H5G_name_t * H5G_nameof(H5G_t *grp);
H5_DLL H5F_t *H5G_fileof(H5G_t *grp);
H5_DLL H5G_t *H5G_open(const H5G_loc_t *loc, hid_t dxpl_id);
H5_DLL herr_t H5G_close(H5G_t *grp);
H5_DLL herr_t H5G_get_shared_count(H5G_t *grp);
H5_DLL herr_t H5G_mount(H5G_t *grp);
H5_DLL hbool_t H5G_mounted(H5G_t *grp);
H5_DLL herr_t H5G_unmount(H5G_t *grp);
#ifndef H5_NO_DEPRECATED_SYMBOLS
H5_DLL H5G_obj_t H5G_map_obj_type(H5O_type_t obj_type);
#endif /* H5_NO_DEPRECATED_SYMBOLS */

/*
 * Utility functions
 */
H5_DLL char *H5G_normalize(const char *name);

/*
 * Group hierarchy traversal routines
 */
H5_DLL herr_t H5G_traverse(const H5G_loc_t *loc, const char *name,
    unsigned target, H5G_traverse_t op, void *op_data, hid_t lapl_id,
    hid_t dxpl_id);
H5_DLL herr_t H5G_iterate(hid_t loc_id, const char *group_name,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t skip, hsize_t *last_lnk,
    const H5G_link_iterate_t *lnk_op, void *op_data, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5G_visit(hid_t loc_id, const char *group_name,
    H5_index_t idx_type, H5_iter_order_t order, H5L_iterate_t op, void *op_data,
    hid_t lapl_id, hid_t dxpl_id);

/* 
 * Functions that understand links in groups
 */
H5_DLL herr_t H5G_link_to_info(const struct H5O_link_t *lnk, H5L_info_t *linfo);

/*
 * Functions that understand group objects
 */
H5_DLL herr_t H5G_obj_insert(const struct H5O_loc_t *grp_oloc, const char *name,
    struct H5O_link_t *obj_lnk, hbool_t adj_link, H5O_type_t obj_type,
    const void *crt_info, hid_t dxpl_id);
H5_DLL ssize_t H5G_obj_get_name_by_idx(const struct H5O_loc_t *oloc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, char* name, size_t size, hid_t dxpl_id);
H5_DLL herr_t H5G_obj_remove(const struct H5O_loc_t *oloc, H5RS_str_t *grp_full_path_r,
    const char *name, hid_t dxpl_id);
H5_DLL herr_t H5G_obj_remove_by_idx(const struct H5O_loc_t *grp_oloc, H5RS_str_t *grp_full_path_r,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t n, hid_t dxpl_id);
H5_DLL herr_t H5G_obj_lookup_by_idx(const struct H5O_loc_t *grp_oloc, H5_index_t idx_type,
    H5_iter_order_t order, hsize_t n, struct H5O_link_t *lnk, hid_t dxpl_id);

/*
 * These functions operate on symbol table nodes.
 */
H5_DLL herr_t H5G_node_close(const H5F_t *f);
H5_DLL herr_t H5G_node_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr, FILE *stream,
			      int indent, int fwidth, haddr_t heap);

/*
 * These functions operate on group object locations.
 */
H5_DLL herr_t H5G_ent_encode(const H5F_t *f, uint8_t **pp, const H5G_entry_t *ent);
H5_DLL herr_t H5G_ent_decode(const H5F_t *f, const uint8_t **pp, H5G_entry_t *ent);

/*
 * These functions operate on group hierarchy names.
 */
H5_DLL herr_t H5G_name_set(const H5G_name_t *loc, H5G_name_t *obj, const char *name);
H5_DLL herr_t H5G_name_replace(const struct H5O_link_t *lnk, H5G_names_op_t op,
    H5F_t *src_file, H5RS_str_t *src_full_path_r, H5F_t *dst_file,
    H5RS_str_t *dst_full_path_r, hid_t dxpl_id);
H5_DLL herr_t H5G_name_reset(H5G_name_t *name);
H5_DLL herr_t H5G_name_copy(H5G_name_t *dst, const H5G_name_t *src, H5_copy_depth_t depth);
H5_DLL herr_t H5G_name_free(H5G_name_t *name);
H5_DLL ssize_t H5G_get_name(const H5G_loc_t *loc, char *name/*out*/, size_t size,
    hbool_t *cached, hid_t lapl_id, hid_t dxpl_id);
H5_DLL ssize_t H5G_get_name_by_addr(hid_t fid, hid_t lapl_id, hid_t dxpl_id,
    const struct H5O_loc_t *loc, char* name, size_t size);
H5_DLL H5RS_str_t *H5G_build_fullpath_refstr_str(H5RS_str_t *path_r, const char *name);

/*
 * These functions operate on group "locations"
 */
H5_DLL herr_t H5G_loc(hid_t loc_id, H5G_loc_t *loc);
H5_DLL herr_t H5G_loc_find(const H5G_loc_t *loc, const char *name,
    H5G_loc_t *obj_loc/*out*/, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5G_loc_find_by_idx(H5G_loc_t *loc, const char *group_name,
    H5_index_t idx_type, H5_iter_order_t order, hsize_t n,
    H5G_loc_t *obj_loc/*out*/, hid_t lapl_id, hid_t dxpl_id);
H5_DLL htri_t H5G_loc_exists(const H5G_loc_t *loc, const char *name,
    hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5G_loc_info(H5G_loc_t *loc, const char *name,
    hbool_t want_ih_info, H5O_info_t *oinfo/*out*/, hid_t lapl_id,
    hid_t dxpl_id);
H5_DLL herr_t H5G_loc_set_comment(H5G_loc_t *loc, const char *name,
    const char *comment, hid_t lapl_id, hid_t dxpl_id);
H5_DLL ssize_t H5G_loc_get_comment(H5G_loc_t *loc, const char *name,
    char *comment/*out*/, size_t bufsize, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5G_loc_reset(H5G_loc_t *loc);
H5_DLL herr_t H5G_loc_free(H5G_loc_t *loc);

/*
 * These functions operate on the root group
 */
H5_DLL herr_t H5G_mkroot(H5F_t *f, hid_t dxpl_id, hbool_t create_root);
H5_DLL herr_t H5G_root_loc(H5F_t *f, H5G_loc_t *loc);
H5_DLL herr_t H5G_root_free(H5G_t *grp);
H5_DLL H5G_t *H5G_rootof(H5F_t *f);

#endif /* _H5Gprivate_H */

