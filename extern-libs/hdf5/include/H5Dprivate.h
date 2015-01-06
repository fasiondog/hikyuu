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
 * This file contains private information about the H5D module
 */
#ifndef _H5Dprivate_H
#define _H5Dprivate_H

/* Include package's public header */
#include "H5Dpublic.h"

/* Private headers needed by this file */
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Sprivate.h"		/* Dataspaces 				*/
#include "H5Zprivate.h"		/* Data filters				*/


/**************************/
/* Library Private Macros */
/**************************/

/*
 * Feature: Define H5D_DEBUG on the compiler command line if you want to
 *	    debug dataset I/O. NDEBUG must not be defined in order for this
 *	    to have any effect.
 */
#ifdef NDEBUG
#  undef H5D_DEBUG
#endif

/* ========  Dataset creation property names ======== */
#define H5D_CRT_LAYOUT_NAME        "layout"             /* Storage layout */
#define H5D_CRT_FILL_VALUE_NAME    "fill_value"         /* Fill value */
#define H5D_CRT_ALLOC_TIME_STATE_NAME "alloc_time_state" /* Space allocation time state */
#define H5D_CRT_EXT_FILE_LIST_NAME "efl"                /* External file list */

/* ========  Dataset access property names ======== */
#define H5D_ACS_DATA_CACHE_NUM_SLOTS_NAME   "rdcc_nslots"   /* Size of raw data chunk cache(slots) */
#define H5D_ACS_DATA_CACHE_BYTE_SIZE_NAME   "rdcc_nbytes"   /* Size of raw data chunk cache(bytes) */
#define H5D_ACS_PREEMPT_READ_CHUNKS_NAME    "rdcc_w0"       /* Preemption read chunks first */

/* ======== Data transfer properties ======== */
#define H5D_XFER_MAX_TEMP_BUF_NAME      "max_temp_buf"  /* Maximum temp buffer size */
#define H5D_XFER_TCONV_BUF_NAME         "tconv_buf"     /* Type conversion buffer */
#define H5D_XFER_BKGR_BUF_NAME          "bkgr_buf"      /* Background buffer */
#define H5D_XFER_BKGR_BUF_TYPE_NAME     "bkgr_buf_type" /* Background buffer type */
#define H5D_XFER_BTREE_SPLIT_RATIO_NAME "btree_split_ratio" /* B-tree node splitting ratio */
#define H5D_XFER_VLEN_ALLOC_NAME        "vlen_alloc"    /* Vlen allocation function */
#define H5D_XFER_VLEN_ALLOC_INFO_NAME   "vlen_alloc_info" /* Vlen allocation info */
#define H5D_XFER_VLEN_FREE_NAME         "vlen_free"     /* Vlen free function */
#define H5D_XFER_VLEN_FREE_INFO_NAME    "vlen_free_info" /* Vlen free info */
#define H5D_XFER_VFL_ID_NAME            "vfl_id"        /* File driver ID */
#define H5D_XFER_VFL_INFO_NAME          "vfl_info"      /* File driver info */
#define H5D_XFER_HYPER_VECTOR_SIZE_NAME "vec_size"      /* Hyperslab vector size */
#ifdef H5_HAVE_PARALLEL
#define H5D_XFER_IO_XFER_MODE_NAME      "io_xfer_mode"  /* I/O transfer mode */
#define H5D_XFER_MPIO_COLLECTIVE_OPT_NAME "mpio_collective_opt" /* Optimization of MPI-IO transfer mode */
#define H5D_XFER_MPIO_CHUNK_OPT_HARD_NAME "mpio_chunk_opt_hard"
#define H5D_XFER_MPIO_CHUNK_OPT_NUM_NAME "mpio_chunk_opt_num"
#define H5D_XFER_MPIO_CHUNK_OPT_RATIO_NAME "mpio_chunk_opt_ratio"
#define H5D_MPIO_ACTUAL_CHUNK_OPT_MODE_NAME "actual_chunk_opt_mode"
#define H5D_MPIO_ACTUAL_IO_MODE_NAME    "actual_io_mode"
#define H5D_MPIO_LOCAL_NO_COLLECTIVE_CAUSE_NAME "local_no_collective_cause"  /* cause of broken collective I/O in each process */
#define H5D_MPIO_GLOBAL_NO_COLLECTIVE_CAUSE_NAME "global_no_collective_cause"  /* cause of broken collective I/O in all processes */
#endif /* H5_HAVE_PARALLEL */
#define H5D_XFER_EDC_NAME               "err_detect"    /* EDC */
#define H5D_XFER_FILTER_CB_NAME         "filter_cb"     /* Filter callback function */
#define H5D_XFER_CONV_CB_NAME           "type_conv_cb"  /* Type conversion callback function */
#define H5D_XFER_XFORM_NAME             "data_transform" /* Data transform */
#ifdef H5_HAVE_INSTRUMENTED_LIBRARY
/* Collective chunk instrumentation properties */
#define H5D_XFER_COLL_CHUNK_LINK_HARD_NAME "coll_chunk_link_hard"
#define H5D_XFER_COLL_CHUNK_MULTI_HARD_NAME "coll_chunk_multi_hard"
#define H5D_XFER_COLL_CHUNK_LINK_NUM_TRUE_NAME "coll_chunk_link_true"
#define H5D_XFER_COLL_CHUNK_LINK_NUM_FALSE_NAME "coll_chunk_link_false"
#define H5D_XFER_COLL_CHUNK_MULTI_RATIO_COLL_NAME "coll_chunk_multi_coll"
#define H5D_XFER_COLL_CHUNK_MULTI_RATIO_IND_NAME "coll_chunk_multi_ind"
#define H5D_XFER_COLL_CHUNK_LINK_TO_MULTI "coll_chunk_link_mul"/* Internal transferring from link to multiple chunk */
#define H5D_XFER_COLL_CHUNK_LINK_TO_MULTI_OPT "coll_chunk_link_mul_opt"/* Internal transferring from link opt to multiple chunk opt*/

/* Definitions for all collective chunk instrumentation properties */
#define H5D_XFER_COLL_CHUNK_SIZE        sizeof(unsigned)
#define H5D_XFER_COLL_CHUNK_DEF         1
#define H5D_XFER_COLL_CHUNK_FIX         0
#endif /* H5_HAVE_INSTRUMENTED_LIBRARY */

/* Default temporary buffer size */
#define H5D_TEMP_BUF_SIZE       (1024 * 1024)

/* Default I/O vector size */
#define H5D_IO_VECTOR_SIZE      1024

/* Default VL allocation & free info */
#define H5D_VLEN_ALLOC          NULL
#define H5D_VLEN_ALLOC_INFO     NULL
#define H5D_VLEN_FREE           NULL
#define H5D_VLEN_FREE_INFO      NULL


/****************************/
/* Library Private Typedefs */
/****************************/

/* Typedef for dataset in memory (defined in H5Dpkg.h) */
typedef struct H5D_t H5D_t;

/* Typedef for cached dataset transfer property list information */
typedef struct H5D_dxpl_cache_t {
    size_t max_temp_buf;        /* Maximum temporary buffer size (H5D_XFER_MAX_TEMP_BUF_NAME) */
    void *tconv_buf;            /* Temporary conversion buffer (H5D_XFER_TCONV_BUF_NAME) */
    void *bkgr_buf;             /* Background conversion buffer (H5D_XFER_BKGR_BUF_NAME) */
    H5T_bkg_t bkgr_buf_type;    /* Background buffer type (H5D_XFER_BKGR_BUF_NAME) */
    H5Z_EDC_t err_detect;       /* Error detection info (H5D_XFER_EDC_NAME) */
    double btree_split_ratio[3];/* B-tree split ratios (H5D_XFER_BTREE_SPLIT_RATIO_NAME) */
    size_t vec_size;            /* Size of hyperslab vector (H5D_XFER_HYPER_VECTOR_SIZE_NAME) */
#ifdef H5_HAVE_PARALLEL
    H5FD_mpio_xfer_t xfer_mode; /* Parallel transfer for this request (H5D_XFER_IO_XFER_MODE_NAME) */
    H5FD_mpio_collective_opt_t coll_opt_mode; /* Parallel transfer with independent IO or collective IO with this mode */
#endif /*H5_HAVE_PARALLEL*/
    H5Z_cb_t filter_cb;         /* Filter callback function (H5D_XFER_FILTER_CB_NAME) */
    H5Z_data_xform_t *data_xform_prop; /* Data transform prop (H5D_XFER_XFORM_NAME) */
} H5D_dxpl_cache_t;

/* Typedef for cached dataset creation property list information */
typedef struct H5D_dcpl_cache_t {
    H5O_fill_t fill;            /* Fill value info (H5D_CRT_FILL_VALUE_NAME) */
    H5O_pline_t pline;          /* I/O pipeline info (H5O_CRT_PIPELINE_NAME) */
    H5O_efl_t efl;              /* External file list info (H5D_CRT_EXT_FILE_LIST_NAME) */
} H5D_dcpl_cache_t;

/* Callback information for copying datasets */
typedef struct H5D_copy_file_ud_t {
    H5O_copy_file_ud_common_t common;   /* Shared information (must be first) */
    struct H5S_extent_t *src_space_extent;     /* Copy of dataspace extent for dataset */
    H5T_t *src_dtype;                   /* Copy of datatype for dataset */
} H5D_copy_file_ud_t;


/*****************************/
/* Library Private Variables */
/*****************************/


/******************************/
/* Library Private Prototypes */
/******************************/

H5_DLL herr_t H5D_init(void);
H5_DLL H5D_t *H5D_open(const H5G_loc_t *loc, hid_t dapl_id, hid_t dxpl_id);
H5_DLL herr_t H5D_close(H5D_t *dataset);
H5_DLL H5O_loc_t *H5D_oloc(H5D_t *dataset);
H5_DLL H5G_name_t *H5D_nameof(H5D_t *dataset);
H5_DLL H5T_t *H5D_typeof(const H5D_t *dset);
H5_DLL herr_t H5D_flush(const H5F_t *f, hid_t dxpl_id);

/* Functions that operate on vlen data */
H5_DLL herr_t H5D_vlen_reclaim(hid_t type_id, H5S_t *space, hid_t plist_id,
    void *buf);

/* Functions that operate on chunked storage */
H5_DLL herr_t H5D_chunk_idx_reset(H5O_storage_chunk_t *storage, hbool_t reset_addr);

/* Functions that operate on indexed storage */
H5_DLL herr_t H5D_btree_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr, FILE * stream,
				int indent, int fwidth, unsigned ndims);

#endif /* _H5Dprivate_H */

