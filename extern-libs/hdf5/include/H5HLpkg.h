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
 * Programmer: Quincey Koziol <koziol@ncsa.uiuc.edu>
 *             Wednesday, July 9, 2003
 *
 * Purpose:     This file contains declarations which are visible
 *              only within the H5HL package. Source files outside the
 *              H5HL package should include H5HLprivate.h instead.
 */
#ifndef H5HL_PACKAGE
#error "Do not include this file outside the H5HL package!"
#endif

#ifndef _H5HLpkg_H
#define _H5HLpkg_H

/* Get package's private header */
#include "H5HLprivate.h"

/* Other private headers needed by this file */
#include "H5FLprivate.h"	/* Free lists                           */


/*****************************/
/* Package Private Variables */
/*****************************/

/* The local heap prefix cache subclass */
H5_DLLVAR const H5AC_class_t H5AC_LHEAP_PRFX[1];

/* The local heap data block cache subclass */
H5_DLLVAR const H5AC_class_t H5AC_LHEAP_DBLK[1];

/* Declare extern the free list to manage the H5HL_free_t struct */
H5FL_EXTERN(H5HL_free_t);

/* Declare extern the PQ free list to manage the heap chunk information */
H5FL_BLK_EXTERN(lheap_chunk);


/**************************/
/* Package Private Macros */
/**************************/

#define H5HL_SIZEOF_HDR(F)						      \
    H5HL_ALIGN(H5_SIZEOF_MAGIC +	/*heap signature		*/    \
	       1 +			/*version			*/    \
	       3 +			/*reserved			*/    \
	       H5F_SIZEOF_SIZE(F) +	/*data size			*/    \
	       H5F_SIZEOF_SIZE(F) +	/*free list head		*/    \
	       H5F_SIZEOF_ADDR(F))	/*data address			*/

/* Value indicating end of free list on disk */
#define H5HL_FREE_NULL	1


/****************************/
/* Package Private Typedefs */
/****************************/

typedef struct H5HL_free_t {
    size_t		offset;		/*offset of free block		*/
    size_t		size;		/*size of free block		*/
    struct H5HL_free_t	*prev;		/*previous entry in free list	*/
    struct H5HL_free_t	*next;		/*next entry in free list	*/
} H5HL_free_t;

/* Forward declarations */
typedef struct H5HL_dblk_t H5HL_dblk_t;
typedef struct H5HL_prfx_t H5HL_prfx_t;

struct H5HL_t {
    /* General heap-management fields */
    size_t                  rc;         /* Ref. count for prefix & data block using this struct */
    size_t                  prots;      /* # of times the heap has been protected */
    size_t                  sizeof_size; /* Size of file sizes */
    size_t                  sizeof_addr; /* Size of file addresses */
    hbool_t                 single_cache_obj;   /* Indicate if the heap is a single object in the cache */
    H5HL_free_t		   *freelist;	/*the free list			*/

    /* Prefix-specific fields */
    H5HL_prfx_t            *prfx;       /* The prefix object for the heap */
    haddr_t                 prfx_addr;  /* address of heap prefix */
    size_t                  prfx_size;  /* size of heap prefix */
    hsize_t                 free_block; /* Address of first free block */

    /* Data block-specific fields */
    H5HL_dblk_t            *dblk;       /* The data block object for the heap */
    haddr_t		    dblk_addr;	/* address of data block	*/
    size_t		    dblk_size;	/* size of heap data block on disk and in mem */
    uint8_t		   *dblk_image;	/* The data block image */
};

/* Struct for heap data block */
struct H5HL_dblk_t {
    H5AC_info_t cache_info;    /* Information for H5AC cache functions, _must_ be */
                                /* first field in structure */
    H5HL_t                 *heap;       /* Pointer to heap for data block */
};

/* Struct for heap prefix */
struct H5HL_prfx_t {
    H5AC_info_t cache_info;    /* Information for H5AC cache functions, _must_ be */
                                /* first field in structure */
    H5HL_t                 *heap;       /* Pointer to heap for prefix */
};

/* Callback information for loading local heap prefix from disk */
typedef struct H5HL_cache_prfx_ud_t {
    /* Downwards */
    size_t sizeof_size;                 /* Size of file sizes */
    size_t sizeof_addr;                 /* Size of file addresses */
    haddr_t prfx_addr;                  /* Address of prefix */
    size_t sizeof_prfx;                 /* Size of heap prefix */

    /* Upwards */
} H5HL_cache_prfx_ud_t;

/* Callback information for loading local heap data block from disk */
typedef struct H5HL_cache_dblk_ud_t {
    /* Downwards */
    H5HL_t *heap;                       /* Local heap */

    /* Upwards */
    hbool_t loaded;                     /* Whether data block was loaded from file */
} H5HL_cache_dblk_ud_t;


/******************************/
/* Package Private Prototypes */
/******************************/

/* Heap routines */
H5_DLL H5HL_t *H5HL_new(size_t sizeof_size, size_t sizeof_addr, size_t prfx_size);
H5_DLL herr_t H5HL_dest(H5HL_t *heap);

/* Heap prefix routines */
H5_DLL H5HL_prfx_t *H5HL_prfx_new(H5HL_t *heap);
H5_DLL herr_t H5HL_prfx_dest(H5HL_prfx_t *prfx);

/* Heap data block routines */
H5_DLL H5HL_dblk_t *H5HL_dblk_new(H5HL_t *heap);
H5_DLL herr_t H5HL_dblk_dest(H5HL_dblk_t *dblk);

#endif /* _H5HLpkg_H */

