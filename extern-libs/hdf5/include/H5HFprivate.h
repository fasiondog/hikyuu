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
 * Created:		H5HFprivate.h
 *			Feb 24 2006
 *			Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:		Private header for library accessible fractal heap routines.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5HFprivate_H
#define _H5HFprivate_H

/* Include package's public header */
#include "H5HFpublic.h"

/* Private headers needed by this file */
#include "H5Fprivate.h"		/* File access				*/
#include "H5Oprivate.h"		/* Object headers		  	*/

/**************************/
/* Library Private Macros */
/**************************/

/* Limit heap ID length to 4096 + 1, due to # of bits required to store
 *      length of 'tiny' objects (12 bits)
 */
#define H5HF_MAX_ID_LEN         (4096 + 1)


/****************************/
/* Library Private Typedefs */
/****************************/

/* Creation parameters for doubling-tables */
typedef struct H5HF_dtable_cparam_t {
    unsigned    width;          /* Number of columns in the table (must be power of 2) */
    size_t      start_block_size; /* Starting block size for table (must be power of 2) */
    size_t      max_direct_size; /* Maximum size of a direct block (must be power of 2) */
    unsigned    max_index;      /* Maximum ID/offset for table (integer log2 of actual value, ie. the # of bits required) */
    unsigned    start_root_rows;        /* Starting number of rows for root indirect block */
                                /* 0 indicates to create the full indirect block for the root,
                                 * right from the start.  Doesn't have to be power of 2
                                 */
} H5HF_dtable_cparam_t;

/* Fractal heap creation parameters */
typedef struct H5HF_create_t {
    H5HF_dtable_cparam_t managed;/* Mapped object doubling-table creation parameters */
    hbool_t checksum_dblocks;   /* Whether the direct blocks should be checksummed */
    uint32_t max_man_size;      /* Max. size of object to manage in doubling table */
                                /* (i.e.  min. size of object to store standalone) */
    uint16_t id_len;            /* Length of IDs to use for heap objects */
                                /* (0 - make ID just large enough to hold length & offset of object in the heap) */
                                /* (1 - make ID just large enough to allow 'huge' objects to be accessed directly) */
                                /* (n - make ID 'n' bytes in size) */
    H5O_pline_t pline;          /* I/O filter pipeline to apply to direct blocks & 'huge' objects */
} H5HF_create_t;

/* Fractal heap metadata statistics info */
typedef struct H5HF_stat_t {
    /* 'Managed' object info */
    hsize_t man_size;           /* Size of 'managed' space in heap            */
    hsize_t man_alloc_size;     /* Size of 'managed' space allocated in heap  */
    hsize_t man_iter_off;       /* Offset of "new block" iterator in 'managed' heap space */
    hsize_t man_free_space;     /* Free space within 'managed' heap blocks    */
    hsize_t man_nobjs;          /* Number of 'managed' objects in heap        */

    /* 'Huge' object info */
    hsize_t huge_size;          /* Size of 'huge' objects in heap             */
    hsize_t huge_nobjs;         /* Number of 'huge' objects in heap           */

    /* 'Tiny' object info */
    hsize_t tiny_size;          /* Size of 'tiny' objects in heap             */
    hsize_t tiny_nobjs;         /* Number of 'tiny' objects in heap           */
} H5HF_stat_t;

/* Fractal heap info (forward decl - defined in H5HFpkg.h) */
typedef struct H5HF_t H5HF_t;

/* Typedef for 'op' operations */
typedef herr_t (*H5HF_operator_t)(const void *obj/*in*/, size_t obj_len,
        void *op_data/*in,out*/);

/*****************************/
/* Library-private Variables */
/*****************************/


/***************************************/
/* Library-private Function Prototypes */
/***************************************/

/* General routines for fractal heap operations */
H5_DLL H5HF_t *H5HF_create(H5F_t *f, hid_t dxpl_id, const H5HF_create_t *cparam);
H5_DLL H5HF_t *H5HF_open(H5F_t *f, hid_t dxpl_id, haddr_t fh_addr);
H5_DLL herr_t H5HF_get_id_len(H5HF_t *fh, size_t *id_len_p/*out*/);
H5_DLL herr_t H5HF_get_heap_addr(const H5HF_t *fh, haddr_t *heap_addr/*out*/);
H5_DLL herr_t H5HF_insert(H5HF_t *fh, hid_t dxpl_id, size_t size,
    const void *obj, void *id/*out*/);
H5_DLL herr_t H5HF_get_obj_len(H5HF_t *fh, hid_t dxpl_id, const void *id,
    size_t *obj_len_p/*out*/);
H5_DLL herr_t H5HF_read(H5HF_t *fh, hid_t dxpl_id, const void *id,
    void *obj/*out*/);
H5_DLL herr_t H5HF_write(H5HF_t *fh, hid_t dxpl_id, void *id, hbool_t *id_changed,
    const void *obj);
H5_DLL herr_t H5HF_op(H5HF_t *fh, hid_t dxpl_id, const void *id,
    H5HF_operator_t op, void *op_data);
H5_DLL herr_t H5HF_remove(H5HF_t *fh, hid_t dxpl_id, const void *id);
H5_DLL herr_t H5HF_close(H5HF_t *fh, hid_t dxpl_id);
H5_DLL herr_t H5HF_delete(H5F_t *f, hid_t dxpl_id, haddr_t fh_addr);

/* Statistics routines */
H5_DLL herr_t H5HF_stat_info(const H5HF_t *fh, H5HF_stat_t *stats);
H5_DLL herr_t H5HF_size(const H5HF_t *fh, hid_t dxpl_id, hsize_t *heap_size/*out*/);

/* Debugging routines */
#ifdef H5HF_DEBUGGING
H5_DLL herr_t H5HF_sects_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr,
    FILE *stream, int indent, int fwidth);
#endif /* H5HF_DEBUGGING */

#endif /* _H5HFprivate_H */

