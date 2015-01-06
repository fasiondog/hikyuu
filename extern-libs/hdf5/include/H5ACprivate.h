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
 * Created:		H5ACprivate.h
 *			Jul  9 1997
 *			Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:		Constants and typedefs available to the rest of the
 *			library.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5ACprivate_H
#define _H5ACprivate_H

#include "H5ACpublic.h"		/*public prototypes			*/

/* Pivate headers needed by this header */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Cprivate.h"		/* Cache				*/
#include "H5Fprivate.h"		/* File access				*/

#ifdef H5_METADATA_TRACE_FILE
#define H5AC__TRACE_FILE_ENABLED	1
#else /* H5_METADATA_TRACE_FILE */
#define H5AC__TRACE_FILE_ENABLED	0
#endif /* H5_METADATA_TRACE_FILE */

/* Types of metadata objects cached */
typedef enum {
    H5AC_BT_ID = 0, 	/*B-tree nodes				     */
    H5AC_SNODE_ID,	/*symbol table nodes			     */
    H5AC_LHEAP_PRFX_ID, /*local heap prefix			     */
    H5AC_LHEAP_DBLK_ID, /*local heap data block			     */
    H5AC_GHEAP_ID,	/*global heap				     */
    H5AC_OHDR_ID,	/*object header				     */
    H5AC_OHDR_CHK_ID,	/*object header chunk			     */
    H5AC_BT2_HDR_ID,	/*v2 B-tree header			     */
    H5AC_BT2_INT_ID,	/*v2 B-tree internal node		     */
    H5AC_BT2_LEAF_ID,	/*v2 B-tree leaf node			     */
    H5AC_FHEAP_HDR_ID,	/*fractal heap header			     */
    H5AC_FHEAP_DBLOCK_ID, /*fractal heap direct block		     */
    H5AC_FHEAP_IBLOCK_ID, /*fractal heap indirect block		     */
    H5AC_FSPACE_HDR_ID,	/*free space header			     */
    H5AC_FSPACE_SINFO_ID,/*free space sections			     */
    H5AC_SOHM_TABLE_ID, /*shared object header message master table  */
    H5AC_SOHM_LIST_ID,  /*shared message index stored as a list      */
    H5AC_SUPERBLOCK_ID, /* file superblock                           */
    H5AC_TEST_ID,	/*test entry -- not used for actual files    */
    H5AC_NTYPES		/* Number of types, must be last             */
} H5AC_type_t;

/* H5AC_DUMP_STATS_ON_CLOSE should always be FALSE when
 * H5C_COLLECT_CACHE_STATS is FALSE.
 *
 * When H5C_COLLECT_CACHE_STATS is TRUE, H5AC_DUMP_STATS_ON_CLOSE must
 * be FALSE for "make check" to succeed, but may be set to TRUE at other
 * times for debugging purposes.
 *
 * Hence the following, somewhat odd set of #defines.
 */
#if H5C_COLLECT_CACHE_STATS

#define H5AC_DUMP_STATS_ON_CLOSE	0

#else /* H5C_COLLECT_CACHE_STATS */

#define H5AC_DUMP_STATS_ON_CLOSE	0

#endif /* H5C_COLLECT_CACHE_STATS */

/* Default max metadata cache size and min clean size are give here.
 * At present, these are the same as those given in H5Cprivate.h.
 */

#define H5AC__DEFAULT_MAX_CACHE_SIZE	H5C__DEFAULT_MAX_CACHE_SIZE
#define H5AC__DEFAULT_MIN_CLEAN_SIZE	H5C__DEFAULT_MIN_CLEAN_SIZE


/*
 * Class methods pertaining to caching.	 Each type of cached object will
 * have a constant variable with permanent life-span that describes how
 * to cache the object.	 That variable will be of type H5AC_class_t and
 * have the following required fields...
 *
 * LOAD:	Loads an object from disk to memory.  The function
 *		should allocate some data structure and return it.
 *
 * FLUSH:	Writes some data structure back to disk.  It would be
 *		wise for the data structure to include dirty flags to
 *		indicate whether it really needs to be written.	 This
 *		function is also responsible for freeing memory allocated
 *		by the LOAD method if the DEST argument is non-zero (by
 *              calling the DEST method).
 *
 * DEST:	Just frees memory allocated by the LOAD method.
 *
 * CLEAR:	Just marks object as non-dirty.
 *
 * SIZE:	Report the size (on disk) of the specified cache object.
 *		Note that the space allocated on disk may not be contiguous.
 */

#define H5AC_CALLBACK__NO_FLAGS_SET             H5C_CALLBACK__NO_FLAGS_SET
#define H5AC_CALLBACK__SIZE_CHANGED_FLAG	H5C_CALLBACK__SIZE_CHANGED_FLAG
#define H5AC_CALLBACK__MOVED_FLAG             H5C_CALLBACK__MOVED_FLAG

typedef H5C_load_func_t		H5AC_load_func_t;
typedef H5C_flush_func_t	H5AC_flush_func_t;
typedef H5C_dest_func_t		H5AC_dest_func_t;
typedef H5C_clear_func_t	H5AC_clear_func_t;
typedef H5C_size_func_t		H5AC_size_func_t;

typedef H5C_class_t			H5AC_class_t;


/* The H5AC_NSLOTS #define is now obsolete, as the metadata cache no longer
 * uses slots.  However I am leaving it in for now to avoid modifying the
 * interface between the metadata cache and the rest of HDF.  It should
 * be removed when we get to dealing with the size_hint parameter in
 * H5AC_create().
 *						JRM - 5/20/04
 *
 * Old comment on H5AC_NSLOTS follows:
 *
 * A cache has a certain number of entries.  Objects are mapped into a
 * cache entry by hashing the object's file address.  Each file has its
 * own cache, an array of slots.
 */
#define H5AC_NSLOTS     10330           /* The library "likes" this number... */


typedef H5C_cache_entry_t		H5AC_info_t;


/*===----------------------------------------------------------------------===
 *                             Protect Types
 *===----------------------------------------------------------------------===
 *
 * These are for the wrapper functions to H5AC_protect. They specify what
 * type of operation you're planning on doing to the metadata. The
 * Flexible Parallel HDF5 locking can then act accordingly.
 */

typedef enum H5AC_protect_t {
    H5AC_WRITE,                 /* Protect object for writing                */
    H5AC_READ                   /* Protect object for reading                */
} H5AC_protect_t;


/* Typedef for metadata cache (defined in H5Cpkg.h) */
typedef H5C_t	H5AC_t;

/* Metadata specific properties for FAPL */
/* (Only used for parallel I/O) */
#ifdef H5_HAVE_PARALLEL
/* Definitions for "block before metadata write" property */
#define H5AC_BLOCK_BEFORE_META_WRITE_NAME       "H5AC_block_before_meta_write"
#define H5AC_BLOCK_BEFORE_META_WRITE_SIZE       sizeof(unsigned)
#define H5AC_BLOCK_BEFORE_META_WRITE_DEF        0

/* Definitions for "library internal" property */
#define H5AC_LIBRARY_INTERNAL_NAME       "H5AC_library_internal"
#define H5AC_LIBRARY_INTERNAL_SIZE       sizeof(unsigned)
#define H5AC_LIBRARY_INTERNAL_DEF        0
#endif /* H5_HAVE_PARALLEL */

/* Dataset transfer property list for flush calls */
/* (Collective set, "block before metadata write" set and "library internal" set) */
/* (Global variable declaration, definition is in H5AC.c) */
extern hid_t H5AC_dxpl_id;

/* Dataset transfer property list for independent metadata I/O calls */
/* (just "library internal" set - i.e. independent transfer mode) */
/* (Global variable declaration, definition is in H5AC.c) */
extern hid_t H5AC_ind_dxpl_id;


/* Default cache configuration. */

#define H5AC__DEFAULT_METADATA_WRITE_STRATEGY   \
                                H5AC_METADATA_WRITE_STRATEGY__DISTRIBUTED

#ifdef H5_HAVE_PARALLEL
#define H5AC__DEFAULT_CACHE_CONFIG                                            \
{                                                                             \
  /* int         version                = */ H5AC__CURR_CACHE_CONFIG_VERSION, \
  /* hbool_t     rpt_fcn_enabled        = */ FALSE,                           \
  /* hbool_t     open_trace_file        = */ FALSE,                           \
  /* hbool_t     close_trace_file       = */ FALSE,                           \
  /* char        trace_file_name[]      = */ "",                              \
  /* hbool_t     evictions_enabled      = */ TRUE,                            \
  /* hbool_t     set_initial_size       = */ TRUE,                            \
  /* size_t      initial_size           = */ ( 2 * 1024 * 1024),              \
  /* double      min_clean_fraction     = */ 0.3,                             \
  /* size_t      max_size               = */ (32 * 1024 * 1024),              \
  /* size_t      min_size               = */ (1 * 1024 * 1024),               \
  /* long int    epoch_length           = */ 50000,                           \
  /* enum H5C_cache_incr_mode incr_mode = */ H5C_incr__threshold,             \
  /* double      lower_hr_threshold     = */ 0.9,                             \
  /* double      increment              = */ 2.0,                             \
  /* hbool_t     apply_max_increment    = */ TRUE,                            \
  /* size_t      max_increment          = */ (4 * 1024 * 1024),               \
  /* enum H5C_cache_flash_incr_mode       */                                  \
  /*                    flash_incr_mode = */ H5C_flash_incr__add_space,       \
  /* double      flash_multiple         = */ 1.0,                             \
  /* double      flash_threshold        = */ 0.25,                            \
  /* enum H5C_cache_decr_mode decr_mode = */ H5C_decr__age_out_with_threshold, \
  /* double      upper_hr_threshold     = */ 0.999,                           \
  /* double      decrement              = */ 0.9,                             \
  /* hbool_t     apply_max_decrement    = */ TRUE,                            \
  /* size_t      max_decrement          = */ (1 * 1024 * 1024),               \
  /* int         epochs_before_eviction = */ 3,                               \
  /* hbool_t     apply_empty_reserve    = */ TRUE,                            \
  /* double      empty_reserve          = */ 0.1,                             \
  /* int	 dirty_bytes_threshold  = */ (256 * 1024),                    \
  /* int	metadata_write_strategy = */                                  \
				       H5AC__DEFAULT_METADATA_WRITE_STRATEGY  \
}
#else /* H5_HAVE_PARALLEL */
#define H5AC__DEFAULT_CACHE_CONFIG                                            \
{                                                                             \
  /* int         version                = */ H5C__CURR_AUTO_SIZE_CTL_VER,     \
  /* hbool_t     rpt_fcn_enabled        = */ FALSE,                           \
  /* hbool_t     open_trace_file        = */ FALSE,                           \
  /* hbool_t     close_trace_file       = */ FALSE,                           \
  /* char        trace_file_name[]      = */ "",                              \
  /* hbool_t     evictions_enabled      = */ TRUE,                            \
  /* hbool_t     set_initial_size       = */ TRUE,                            \
  /* size_t      initial_size           = */ ( 2 * 1024 * 1024),              \
  /* double      min_clean_fraction     = */ 0.01,                            \
  /* size_t      max_size               = */ (32 * 1024 * 1024),              \
  /* size_t      min_size               = */ ( 1 * 1024 * 1024),              \
  /* long int    epoch_length           = */ 50000,                           \
  /* enum H5C_cache_incr_mode incr_mode = */ H5C_incr__threshold,             \
  /* double      lower_hr_threshold     = */ 0.9,                             \
  /* double      increment              = */ 2.0,                             \
  /* hbool_t     apply_max_increment    = */ TRUE,                            \
  /* size_t      max_increment          = */ (4 * 1024 * 1024),               \
  /* enum H5C_cache_flash_incr_mode       */                                  \
  /*                    flash_incr_mode = */ H5C_flash_incr__add_space,       \
  /* double      flash_multiple         = */ 1.4,                             \
  /* double      flash_threshold        = */ 0.25,                            \
  /* enum H5C_cache_decr_mode decr_mode = */ H5C_decr__age_out_with_threshold,\
  /* double      upper_hr_threshold     = */ 0.999,                           \
  /* double      decrement              = */ 0.9,                             \
  /* hbool_t     apply_max_decrement    = */ TRUE,                            \
  /* size_t      max_decrement          = */ (1 * 1024 * 1024),               \
  /* int         epochs_before_eviction = */ 3,                               \
  /* hbool_t     apply_empty_reserve    = */ TRUE,                            \
  /* double      empty_reserve          = */ 0.1,                             \
  /* int	 dirty_bytes_threshold  = */ (256 * 1024),                    \
  /* int	metadata_write_strategy = */                                  \
				       H5AC__DEFAULT_METADATA_WRITE_STRATEGY  \
}
#endif /* H5_HAVE_PARALLEL */


/*
 * Library prototypes.
 */

/* #defines of flags used in the flags parameters in some of the
 * following function calls.  Note that they are just copies of
 * the equivalent flags from H5Cprivate.h.
 */

#define H5AC__NO_FLAGS_SET		  H5C__NO_FLAGS_SET
#define H5AC__SET_FLUSH_MARKER_FLAG	  H5C__SET_FLUSH_MARKER_FLAG
#define H5AC__DELETED_FLAG		  H5C__DELETED_FLAG
#define H5AC__DIRTIED_FLAG		  H5C__DIRTIED_FLAG
#define H5AC__PIN_ENTRY_FLAG		  H5C__PIN_ENTRY_FLAG
#define H5AC__UNPIN_ENTRY_FLAG		  H5C__UNPIN_ENTRY_FLAG
#define H5AC__FLUSH_INVALIDATE_FLAG	  H5C__FLUSH_INVALIDATE_FLAG
#define H5AC__FLUSH_CLEAR_ONLY_FLAG	  H5C__FLUSH_CLEAR_ONLY_FLAG
#define H5AC__FLUSH_MARKED_ENTRIES_FLAG   H5C__FLUSH_MARKED_ENTRIES_FLAG
#define H5AC__FLUSH_IGNORE_PROTECTED_FLAG H5C__FLUSH_IGNORE_PROTECTED_FLAG
#define H5AC__FREE_FILE_SPACE_FLAG	  H5C__FREE_FILE_SPACE_FLAG
#define H5AC__TAKE_OWNERSHIP_FLAG         H5C__TAKE_OWNERSHIP_FLAG


/* #defines of flags used to report entry status in the
 * H5AC_get_entry_status() call.
 */

#define H5AC_ES__IN_CACHE	0x0001
#define H5AC_ES__IS_DIRTY	0x0002
#define H5AC_ES__IS_PROTECTED	0x0004
#define H5AC_ES__IS_PINNED	0x0008


/* external function declarations: */

H5_DLL herr_t H5AC_init(void);
H5_DLL herr_t H5AC_create(const H5F_t *f, H5AC_cache_config_t *config_ptr);
H5_DLL herr_t H5AC_get_entry_status(const H5F_t *f, haddr_t addr,
				    unsigned * status_ptr);
H5_DLL herr_t H5AC_insert_entry(H5F_t *f, hid_t dxpl_id, const H5AC_class_t *type,
                       haddr_t addr, void *thing, unsigned int flags);
H5_DLL herr_t H5AC_pin_protected_entry(void *thing);
H5_DLL void * H5AC_protect(H5F_t *f, hid_t dxpl_id, const H5AC_class_t *type,
                           haddr_t addr, void *udata,
                           H5AC_protect_t rw);
H5_DLL herr_t H5AC_resize_entry(void *thing, size_t new_size);
H5_DLL herr_t H5AC_unpin_entry(void *thing);
H5_DLL herr_t H5AC_unprotect(H5F_t *f, hid_t dxpl_id,
                             const H5AC_class_t *type, haddr_t addr,
			     void *thing, unsigned flags);
H5_DLL herr_t H5AC_flush(H5F_t *f, hid_t dxpl_id);
H5_DLL herr_t H5AC_mark_entry_dirty(void *thing);
H5_DLL herr_t H5AC_move_entry(H5F_t *f, const H5AC_class_t *type,
			   haddr_t old_addr, haddr_t new_addr);

H5_DLL herr_t H5AC_dest(H5F_t *f, hid_t dxpl_id);

H5_DLL herr_t H5AC_expunge_entry(H5F_t *f, hid_t dxpl_id,
                                 const H5AC_class_t *type, haddr_t addr,
                                 unsigned flags);

H5_DLL herr_t H5AC_set_sync_point_done_callback(H5C_t *cache_ptr,
    void (*sync_point_done)(int num_writes, haddr_t *written_entries_tbl));

H5_DLL herr_t H5AC_set_write_done_callback(H5C_t * cache_ptr,
                                           void (* write_done)(void));
H5_DLL herr_t H5AC_stats(const H5F_t *f);

H5_DLL herr_t H5AC_dump_cache(const H5F_t *f);

H5_DLL herr_t H5AC_get_cache_auto_resize_config(const H5AC_t * cache_ptr,
                                               H5AC_cache_config_t *config_ptr);

H5_DLL herr_t H5AC_get_cache_size(H5AC_t * cache_ptr,
                                  size_t * max_size_ptr,
                                  size_t * min_clean_size_ptr,
                                  size_t * cur_size_ptr,
                                  int32_t * cur_num_entries_ptr);

H5_DLL herr_t H5AC_get_cache_hit_rate(H5AC_t * cache_ptr,
                                      double * hit_rate_ptr);

H5_DLL herr_t H5AC_reset_cache_hit_rate_stats(H5AC_t * cache_ptr);

H5_DLL herr_t H5AC_set_cache_auto_resize_config(H5AC_t *cache_ptr,
                                               H5AC_cache_config_t *config_ptr);

H5_DLL herr_t H5AC_validate_config(H5AC_cache_config_t * config_ptr);

H5_DLL herr_t H5AC_close_trace_file( H5AC_t * cache_ptr);

H5_DLL herr_t H5AC_open_trace_file(H5AC_t * cache_ptr,
		                   const char * trace_file_name);

#ifdef H5_HAVE_PARALLEL
H5_DLL herr_t H5AC_add_candidate(H5AC_t * cache_ptr, haddr_t addr);
#endif /* H5_HAVE_PARALLEL */

#endif /* !_H5ACprivate_H */

