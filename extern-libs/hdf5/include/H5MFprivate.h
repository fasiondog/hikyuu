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
 * Created:             H5MFprivate.h
 *                      Jul 11 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             Private header file for file memory management.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifndef _H5MFprivate_H
#define _H5MFprivate_H

/* Private headers needed by this file */
#include "H5Fprivate.h"         /* File access				*/
#include "H5FDprivate.h"	/* File Drivers				*/

/**************************/
/* Library Private Macros */
/**************************/

/*
 * Feature: Define H5MF_DEBUG on the compiler command line if you want to
 *	    see diagnostics from this layer.
 */
#ifdef NDEBUG
#  undef H5MF_DEBUG
#endif

/****************************/
/* Library Private Typedefs */
/****************************/


/*****************************/
/* Library-private Variables */
/*****************************/


/***************************************/
/* Library-private Function Prototypes */
/***************************************/

/* File space manager routines */
H5_DLL herr_t H5MF_init_merge_flags(H5F_t *f);
H5_DLL herr_t H5MF_get_freespace(H5F_t *f, hid_t dxpl_id, hsize_t *tot_space,
    hsize_t *meta_size);
H5_DLL herr_t H5MF_close(H5F_t *f, hid_t dxpl_id);

/* File space allocation routines */
H5_DLL haddr_t H5MF_alloc(H5F_t *f, H5FD_mem_t type, hid_t dxpl_id, hsize_t size);
H5_DLL haddr_t H5MF_aggr_vfd_alloc(H5F_t *f, H5FD_mem_t type, hid_t dxpl_id, hsize_t size);
H5_DLL herr_t H5MF_xfree(H5F_t *f, H5FD_mem_t type, hid_t dxpl_id, haddr_t addr,
			  hsize_t size);
H5_DLL herr_t H5MF_try_extend(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type,
    haddr_t addr, hsize_t size, hsize_t extra_requested);
H5_DLL htri_t H5MF_try_shrink(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id,
    haddr_t addr, hsize_t size);

/* File 'temporary' space allocation routines */
H5_DLL haddr_t H5MF_alloc_tmp(H5F_t *f, hsize_t size);

/* 'block aggregator' routines */
H5_DLL herr_t H5MF_free_aggrs(H5F_t *f, hid_t dxpl_id);
H5_DLL htri_t H5MF_aggrs_try_shrink_eoa(H5F_t *f, hid_t dxpl_id);

/* Debugging routines */
#ifdef H5MF_DEBUGGING
#ifdef NOT_YET
H5_DLL herr_t H5MF_sects_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr,
    FILE *stream, int indent, int fwidth);
#endif /* NOT_YET */
#endif /* H5MF_DEBUGGING */

#endif /* end _H5MFprivate_H */

