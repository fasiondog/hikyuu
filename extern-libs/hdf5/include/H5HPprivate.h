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
 * This file contains private information about the H5HP module
 */
#ifndef _H5HPprivate_H
#define _H5HPprivate_H

/**************************************/
/* Public headers needed by this file */
/**************************************/
#ifdef LATER
#include "H5HPpublic.h"
#endif /* LATER */

/***************************************/
/* Private headers needed by this file */
/***************************************/
#include "H5private.h"

/************/
/* Typedefs */
/************/

/* Typedef for heap struct (defined in H5HP.c) */
typedef struct H5HP_t H5HP_t;

/* Typedef for objects which can be inserted into heaps */
/* This _must_ be the first field in objects which can be inserted into heaps */
typedef struct H5HP_info_t {
    size_t heap_loc;                    /* Location of object in heap */
}H5HP_info_t;

/* Typedef for type of heap to create */
typedef enum {
    H5HP_MIN_HEAP,      /* Minimum values in heap are at the "top" */
    H5HP_MAX_HEAP       /* Maximum values in heap are at the "top" */
} H5HP_type_t;

/**********/
/* Macros */
/**********/

/********************/
/* Private routines */
/********************/
H5_DLL H5HP_t *H5HP_create(H5HP_type_t heap_type);
H5_DLL herr_t H5HP_insert(H5HP_t *heap, int val, void *obj);
H5_DLL ssize_t H5HP_count(const H5HP_t *heap);
H5_DLL herr_t H5HP_top(const H5HP_t *heap, int *val);
H5_DLL herr_t H5HP_remove(H5HP_t *heap, int *val, void **ptr);
H5_DLL herr_t H5HP_change(H5HP_t *heap, int val, void *obj);
H5_DLL herr_t H5HP_incr(H5HP_t *heap, unsigned amt, void *obj);
H5_DLL herr_t H5HP_decr(H5HP_t *heap, unsigned amt, void *obj);
H5_DLL herr_t H5HP_close(H5HP_t *heap);

#endif /* _H5HPprivate_H */

