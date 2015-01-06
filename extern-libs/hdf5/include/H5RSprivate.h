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
 * This file contains private information about the H5RS module
 */
#ifndef _H5RSprivate_H
#define _H5RSprivate_H

/**************************************/
/* Public headers needed by this file */
/**************************************/
#ifdef LATER
#include "H5RSpublic.h"
#endif /* LATER */

/***************************************/
/* Private headers needed by this file */
/***************************************/
#include "H5private.h"

/************/
/* Typedefs */
/************/

/* Typedef for reference counted string (defined in H5RS.c) */
typedef struct H5RS_str_t H5RS_str_t;

/**********/
/* Macros */
/**********/

/********************/
/* Private routines */
/********************/
H5_DLL H5RS_str_t *H5RS_create(const char *s);
H5_DLL H5RS_str_t *H5RS_wrap(const char *s);
H5_DLL H5RS_str_t *H5RS_own(char *s);
H5_DLL herr_t H5RS_decr(H5RS_str_t *rs);
H5_DLL herr_t H5RS_incr(H5RS_str_t *rs);
H5_DLL H5RS_str_t *H5RS_dup(H5RS_str_t *s);
H5_DLL H5RS_str_t *H5RS_dup_str(const char *s);
H5_DLL int H5RS_cmp(const H5RS_str_t *rs1, const H5RS_str_t *rs2);
H5_DLL ssize_t H5RS_len(const H5RS_str_t *rs);
H5_DLL char *H5RS_get_str(const H5RS_str_t *rs);
H5_DLL unsigned H5RS_get_count(const H5RS_str_t *rs);

#endif /* _H5RSprivate_H */

