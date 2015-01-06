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

/*-----------------------------------------------------------------------------
 * File:    H5Iprivate.h
 * Purpose: header file for ID API
 *---------------------------------------------------------------------------*/

/* avoid re-inclusion */
#ifndef _H5Iprivate_H
#define _H5Iprivate_H

/* Include package's public header */
#include "H5Ipublic.h"

/* Private headers needed by this file */
#include "H5private.h"

/**************************/
/* Library Private Macros */
/**************************/

/* Macro to determine if a H5I_type_t is a "library type" */
#define H5I_IS_LIB_TYPE( type ) (type > 0 && type < H5I_NTYPES)

/* Flags for ID class */
#define H5I_CLASS_IS_APPLICATION        0x01


/****************************/
/* Library Private Typedefs */
/****************************/

typedef struct H5I_class_t {
    H5I_type_t type_id;         /* Class ID for the type */
    unsigned flags;             /* Class behavior flags */
    size_t hash_size;           /* Minimum hash table size for the type */
    unsigned reserved;          /* Number of reserved IDs for this type */
                                /* [A specific number of type entries may be
                                 * reserved to enable "constant" values to be
                                 * handed out which are valid IDs in the type,
                                 * but which do not map to any data structures
                                 * and are not allocated dynamically later.]
                                 */
    H5I_free_t free_func;       /* Free function for object's of this type */
} H5I_class_t;


/*****************************/
/* Library-private Variables */
/*****************************/


/***************************************/
/* Library-private Function Prototypes */
/***************************************/
H5_DLL herr_t H5I_register_type(const H5I_class_t *cls);
H5_DLL int H5I_nmembers(H5I_type_t type);
H5_DLL herr_t H5I_clear_type(H5I_type_t type, hbool_t force, hbool_t app_ref);
H5_DLL int H5I_destroy_type(H5I_type_t type);
H5_DLL hid_t H5I_register(H5I_type_t type, const void *object, hbool_t app_ref);
H5_DLL void *H5I_subst(hid_t id, const void *new_object);
H5_DLL void *H5I_object(hid_t id);
H5_DLL void *H5I_object_verify(hid_t id, H5I_type_t id_type);
H5_DLL H5I_type_t H5I_get_type(hid_t id);
H5_DLL hid_t H5I_get_file_id(hid_t obj_id, hbool_t app_ref);
H5_DLL void *H5I_remove(hid_t id);
H5_DLL void *H5I_remove_verify(hid_t id, H5I_type_t id_type);
H5_DLL herr_t H5I_iterate(H5I_type_t type, H5I_search_func_t func, void *udata, hbool_t app_ref);
H5_DLL int H5I_get_ref(hid_t id, hbool_t app_ref);
H5_DLL int H5I_inc_ref(hid_t id, hbool_t app_ref);
H5_DLL int H5I_dec_ref(hid_t id);
H5_DLL int H5I_dec_app_ref(hid_t id);
H5_DLL int H5I_dec_app_ref_always_close(hid_t id);
H5_DLL int H5I_inc_type_ref(H5I_type_t type);
H5_DLL herr_t H5I_dec_type_ref(H5I_type_t type);
H5_DLL int H5I_get_type_ref(H5I_type_t type);

#endif /* _H5Iprivate_H */

