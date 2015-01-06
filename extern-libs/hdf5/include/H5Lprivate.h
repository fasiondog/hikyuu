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
 * This file contains private information about the H5L module
 * for dealing with links in an HDF5 file.
 */
#ifndef _H5Lprivate_H
#define _H5Lprivate_H

/* Include package's public header */
#include "H5Lpublic.h"

/* Private headers needed by this file */
#include "H5Gprivate.h"		/* Groups				*/
#include "H5Oprivate.h"		/* Object headers			*/


/**************************/
/* Library Private Macros */
/**************************/

/* Default number of soft links to traverse */
#define H5L_NUM_LINKS   16

/* ========  Link creation property names ======== */
#define H5L_CRT_INTERMEDIATE_GROUP_NAME         "intermediate_group" /* Create intermediate groups flag */

/* ========  Link access property names ======== */
#define H5L_ACS_NLINKS_NAME        "max soft links"         /* Number of soft links to traverse */
#define H5L_ACS_ELINK_PREFIX_NAME  "external link prefix"   /* External link prefix */
#define H5L_ACS_ELINK_FAPL_NAME    "external link fapl"     /* file access property list for external link access */
#define H5L_ACS_ELINK_FLAGS_NAME   "external link flags"    /* file access flags for external link traversal */
#define H5L_ACS_ELINK_CB_NAME      "external link callback" /*  callback function for external link traversal */


/****************************/
/* Library Private Typedefs */
/****************************/

/* Structure for external link traversal callback property */
typedef struct H5L_elink_cb_t {
    H5L_elink_traverse_t      func;
    void                      *user_data;
} H5L_elink_cb_t;


/*****************************/
/* Library Private Variables */
/*****************************/


/******************************/
/* Library Private Prototypes */
/******************************/

/* General operations on links */
H5_DLL herr_t H5L_init(void);
H5_DLL herr_t H5L_link(const H5G_loc_t *new_loc, const char *new_name,
    H5G_loc_t *obj_loc, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_link_object(const H5G_loc_t *new_loc, const char *new_name,
    H5O_obj_create_t *ocrt_info, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_create_hard(H5G_loc_t *cur_loc, const char *cur_name,
    const H5G_loc_t *link_loc, const char *link_name, hid_t lcpl_id,
    hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_create_soft(const char *target_path, const H5G_loc_t *cur_loc,
    const char *cur_name, hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id);
H5_DLL hid_t H5L_get_default_lcpl(void);
H5_DLL herr_t H5L_move(H5G_loc_t *src_loc, const char *src_name,
    H5G_loc_t *dst_loc, const char *dst_name, hbool_t copy_flag,
    hid_t lcpl_id, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_get_info(const H5G_loc_t *loc, const char *name,
    H5L_info_t *linkbuf/*out*/, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_delete(H5G_loc_t *loc, const char *name, hid_t lapl_id,
    hid_t dxpl_id);
H5_DLL herr_t H5L_get_val(H5G_loc_t *loc, const char *name, void *buf/*out*/,
    size_t size, hid_t lapl_id, hid_t dxpl_id);
H5_DLL herr_t H5L_register_external(void);

/* User-defined link functions */
H5_DLL herr_t H5L_register(const H5L_class_t *cls);
H5_DLL herr_t H5L_unregister(H5L_type_t id);
H5_DLL const H5L_class_t *H5L_find_class(H5L_type_t id);

#endif /* _H5Lprivate_H */

