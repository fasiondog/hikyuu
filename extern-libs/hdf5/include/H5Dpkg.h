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
 * Programmer:	Quincey Koziol <koziol@ncsa.uiuc.edu>
 *		Monday, April 14, 2003
 *
 * Purpose:	This file contains declarations which are visible only within
 *		the H5D package.  Source files outside the H5D package should
 *		include H5Dprivate.h instead.
 */
#ifndef H5D_PACKAGE
#error "Do not include this file outside the H5D package!"
#endif

#ifndef _H5Dpkg_H
#define _H5Dpkg_H

/* Get package's private header */
#include "H5Dprivate.h"

/* Other private headers needed by this file */
#include "H5ACprivate.h"	/* Metadata cache			*/
#include "H5Gprivate.h"		/* Groups 			  	*/
#include "H5SLprivate.h"	/* Skip lists				*/
#include "H5Tprivate.h"		/* Datatypes         			*/

/**************************/
/* Package Private Macros */
/**************************/

/* Set the minimum object header size to create objects with */
#define H5D_MINHDR_SIZE 256

/* [Simple] Macro to construct a H5D_io_info_t from it's components */
#define H5D_BUILD_IO_INFO_WRT(io_info, ds, dxpl_c, dxpl_i, str, buf)    \
    (io_info)->dset = ds;                                               \
    (io_info)->dxpl_cache = dxpl_c;                                     \
    (io_info)->dxpl_id = dxpl_i;                                        \
    (io_info)->store = str;                                             \
    (io_info)->op_type = H5D_IO_OP_WRITE;                               \
    (io_info)->u.wbuf = buf
#define H5D_BUILD_IO_INFO_RD(io_info, ds, dxpl_c, dxpl_i, str, buf)     \
    (io_info)->dset = ds;                                               \
    (io_info)->dxpl_cache = dxpl_c;                                     \
    (io_info)->dxpl_id = dxpl_i;                                        \
    (io_info)->store = str;                                             \
    (io_info)->op_type = H5D_IO_OP_READ;                                \
    (io_info)->u.rbuf = buf

#define H5D_CHUNK_HASH(D, ADDR) H5F_addr_hash(ADDR, (D)->cache.chunk.nslots)

/* Flags for marking aspects of a dataset dirty */
#define H5D_MARK_SPACE  0x01
#define H5D_MARK_LAYOUT  0x02


/****************************/
/* Package Private Typedefs */
/****************************/

/* Typedef for datatype information for raw data I/O operation */
typedef struct H5D_type_info_t {
    /* Initial values */
    const H5T_t *mem_type;              /* Pointer to memory datatype */
    const H5T_t *dset_type;             /* Pointer to dataset datatype */
    H5T_path_t *tpath;                  /* Datatype conversion path */
    hid_t src_type_id;                  /* Source datatype ID */
    hid_t dst_type_id;                  /* Destination datatype ID */

    /* Computed/derived values */
    size_t src_type_size;		/* Size of source type	*/
    size_t dst_type_size;	        /* Size of destination type*/
    size_t max_type_size;	        /* Size of largest source/destination type */
    hbool_t is_conv_noop;               /* Whether the type conversion is a NOOP */
    hbool_t is_xform_noop;              /* Whether the data transform is a NOOP */
    const H5T_subset_info_t *cmpd_subset;   /* Info related to the compound subset conversion functions */
    H5T_bkg_t need_bkg;		        /* Type of background buf needed */
    size_t request_nelmts;		/* Requested strip mine	*/
    uint8_t *tconv_buf;	                /* Datatype conv buffer	*/
    hbool_t tconv_buf_allocated;        /* Whether the type conversion buffer was allocated */
    uint8_t *bkg_buf;	                /* Background buffer	*/
    hbool_t bkg_buf_allocated;          /* Whether the background buffer was allocated */
} H5D_type_info_t;

/* Forward declaration of structs used below */
struct H5D_io_info_t;
struct H5D_chunk_map_t;

/* Function pointers for I/O on particular types of dataset layouts */
typedef herr_t (*H5D_layout_construct_func_t)(H5F_t *f, H5D_t *dset);
typedef herr_t (*H5D_layout_init_func_t)(H5F_t *f, hid_t dxpl_id, const H5D_t *dset,
    hid_t dapl_id);
typedef hbool_t (*H5D_layout_is_space_alloc_func_t)(const H5O_storage_t *storage);
typedef herr_t (*H5D_layout_io_init_func_t)(const struct H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space,
    struct H5D_chunk_map_t *cm);
typedef herr_t (*H5D_layout_read_func_t)(struct H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, struct H5D_chunk_map_t *fm);
typedef herr_t (*H5D_layout_write_func_t)(struct H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, struct H5D_chunk_map_t *fm);
typedef ssize_t (*H5D_layout_readvv_func_t)(const struct H5D_io_info_t *io_info,
    size_t dset_max_nseq, size_t *dset_curr_seq, size_t dset_len_arr[], hsize_t dset_offset_arr[],
    size_t mem_max_nseq, size_t *mem_curr_seq, size_t mem_len_arr[], hsize_t mem_offset_arr[]);
typedef ssize_t (*H5D_layout_writevv_func_t)(const struct H5D_io_info_t *io_info,
    size_t dset_max_nseq, size_t *dset_curr_seq, size_t dset_len_arr[], hsize_t dset_offset_arr[],
    size_t mem_max_nseq, size_t *mem_curr_seq, size_t mem_len_arr[], hsize_t mem_offset_arr[]);
typedef herr_t (*H5D_layout_flush_func_t)(H5D_t *dataset, hid_t dxpl_id);
typedef herr_t (*H5D_layout_io_term_func_t)(const struct H5D_chunk_map_t *cm);

/* Typedef for grouping layout I/O routines */
typedef struct H5D_layout_ops_t {
    H5D_layout_construct_func_t construct;      /* Layout constructor for new datasets */
    H5D_layout_init_func_t init;        /* Layout initializer for dataset */
    H5D_layout_is_space_alloc_func_t is_space_alloc;    /* Query routine to determine if storage is allocated */
    H5D_layout_io_init_func_t io_init;  /* I/O initialization routine */
    H5D_layout_read_func_t ser_read;    /* High-level I/O routine for reading data in serial */
    H5D_layout_write_func_t ser_write;  /* High-level I/O routine for writing data in serial */
#ifdef H5_HAVE_PARALLEL
    H5D_layout_read_func_t par_read;    /* High-level I/O routine for reading data in parallel */
    H5D_layout_write_func_t par_write;  /* High-level I/O routine for writing data in parallel */
#endif /* H5_HAVE_PARALLEL */
    H5D_layout_readvv_func_t readvv;    /* Low-level I/O routine for reading data */
    H5D_layout_writevv_func_t writevv;  /* Low-level I/O routine for writing data */
    H5D_layout_flush_func_t flush;      /* Low-level I/O routine for flushing raw data */
    H5D_layout_io_term_func_t io_term;  /* I/O shutdown routine */
} H5D_layout_ops_t;

/* Function pointers for either multiple or single block I/O access */
typedef herr_t (*H5D_io_single_read_func_t)(const struct H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);
typedef herr_t (*H5D_io_single_write_func_t)(const struct H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);

/* Typedef for raw data I/O framework info */
typedef struct H5D_io_ops_t {
    H5D_layout_read_func_t multi_read;          /* High-level I/O routine for reading data */
    H5D_layout_write_func_t multi_write;        /* High-level I/O routine for writing data */
    H5D_io_single_read_func_t single_read;      /* I/O routine for reading single block */
    H5D_io_single_write_func_t single_write;    /* I/O routine for writing single block */
} H5D_io_ops_t;

/* Typedefs for dataset storage information */
typedef struct {
    haddr_t dset_addr;      /* Address of dataset in file */
    hsize_t dset_size;      /* Total size of dataset in file */
} H5D_contig_storage_t;

typedef struct {
    hsize_t index;          /* "Index" of chunk in dataset (must be first for TBBT routines) */
    hsize_t *offset;        /* Chunk's coordinates in elements */
} H5D_chunk_storage_t;

typedef struct {
    void *buf;              /* Buffer for compact dataset */
    hbool_t *dirty;         /* Pointer to dirty flag to mark */
} H5D_compact_storage_t;

typedef union H5D_storage_t {
    H5D_contig_storage_t contig; /* Contiguous information for dataset */
    H5D_chunk_storage_t chunk;  /* Chunk information for dataset */
    H5D_compact_storage_t compact; /* Compact information for dataset */
    H5O_efl_t   efl;            /* External file list information for dataset */
} H5D_storage_t;

/* Typedef for raw data I/O operation info */
typedef enum H5D_io_op_type_t {
    H5D_IO_OP_READ,         /* Read operation */
    H5D_IO_OP_WRITE         /* Write operation */
} H5D_io_op_type_t;

typedef struct H5D_io_info_t {
    H5D_t *dset;                /* Pointer to dataset being operated on */
#ifndef H5_HAVE_PARALLEL
    const
#endif /* H5_HAVE_PARALLEL */
        H5D_dxpl_cache_t *dxpl_cache; /* Pointer to cached DXPL info */
    hid_t dxpl_id;              /* Original DXPL ID */
#ifdef H5_HAVE_PARALLEL
    MPI_Comm comm;              /* MPI communicator for file */
    hbool_t using_mpi_vfd;      /* Whether the file is using an MPI-based VFD */
    struct {
        H5FD_mpio_xfer_t xfer_mode; /* Parallel transfer for this request (H5D_XFER_IO_XFER_MODE_NAME) */
        H5FD_mpio_collective_opt_t coll_opt_mode; /* Parallel transfer with independent IO or collective IO with this mode */
        H5D_io_ops_t io_ops;    /* I/O operation function pointers */
    } orig;
#endif /* H5_HAVE_PARALLEL */
    H5D_storage_t *store;       /* Dataset storage info */
    H5D_layout_ops_t layout_ops;    /* Dataset layout I/O operation function pointers */
    H5D_io_ops_t io_ops;        /* I/O operation function pointers */
    H5D_io_op_type_t op_type;
    union {
        void *rbuf;             /* Pointer to buffer for read */
        const void *wbuf;       /* Pointer to buffer to write */
    } u;
} H5D_io_info_t;


/******************/
/* Chunk typedefs */
/******************/

/* Typedef for chunked dataset index operation info */
typedef struct H5D_chk_idx_info_t {
    H5F_t *f;                   /* File pointer for operation */
    hid_t dxpl_id;              /* DXPL ID for operation */
    const H5O_pline_t *pline;   /* I/O pipeline info */
    H5O_layout_chunk_t *layout;           /* Chunk layout description */
    H5O_storage_chunk_t *storage;         /* Chunk storage description */
} H5D_chk_idx_info_t;

/*
 * "Generic" chunk record.  Each chunk is keyed by the minimum logical
 * N-dimensional coordinates and the datatype size of the chunk.
 * The fastest-varying dimension is assumed to reference individual bytes of
 * the array, so a 100-element 1-D array of 4-byte integers would really be a
 * 2-D array with the slow varying dimension of size 100 and the fast varying
 * dimension of size 4 (the storage dimensionality has very little to do with
 * the real dimensionality).
 *
 * The chunk's file address, filter mask and size on disk are not key values.
 */
typedef struct H5D_chunk_rec_t {
    uint32_t	nbytes;				/* Size of stored data	*/
    hsize_t	offset[H5O_LAYOUT_NDIMS];	/* Logical offset to start */
    unsigned	filter_mask;			/* Excluded filters	*/
    haddr_t     chunk_addr;                     /* Address of chunk in file */
} H5D_chunk_rec_t;

/*
 * Common data exchange structure for indexed storage nodes.  This structure is
 * passed through the indexing layer to the methods for the objects
 * to which the index points.
 */
typedef struct H5D_chunk_common_ud_t {
    /* downward */
    const H5O_layout_chunk_t *layout;           /* Chunk layout description */
    const H5O_storage_chunk_t *storage;         /* Chunk storage description */
    const hsize_t *offset;	                /* Logical offset of chunk */
    const struct H5D_rdcc_t *rdcc;              /* Chunk cache.  Only necessary if the index may
                                                 * be modified, and if any chunks in the dset
                                                 * may be cached */
} H5D_chunk_common_ud_t;

/* B-tree callback info for various operations */
typedef struct H5D_chunk_ud_t {
    H5D_chunk_common_ud_t common;       /* Common info for B-tree user data (must be first) */

    /* Upward */
    unsigned    idx_hint;               /*index of chunk in cache, if present */
    uint32_t	nbytes;			/*size of stored data	*/
    unsigned	filter_mask;		/*excluded filters	*/
    haddr_t	addr;			/*file address of chunk */
} H5D_chunk_ud_t;

/* Typedef for "generic" chunk callbacks */
typedef int (*H5D_chunk_cb_func_t)(const H5D_chunk_rec_t *chunk_rec,
    void *udata);

/* Typedefs for chunk operations */
typedef herr_t (*H5D_chunk_init_func_t)(const H5D_chk_idx_info_t *idx_info,
    const H5S_t *space, haddr_t dset_ohdr_addr);
typedef herr_t (*H5D_chunk_create_func_t)(const H5D_chk_idx_info_t *idx_info);
typedef hbool_t (*H5D_chunk_is_space_alloc_func_t)(const H5O_storage_chunk_t *storage);
typedef herr_t (*H5D_chunk_insert_func_t)(const H5D_chk_idx_info_t *idx_info,
    H5D_chunk_ud_t *udata);
typedef herr_t (*H5D_chunk_get_addr_func_t)(const H5D_chk_idx_info_t *idx_info,
    H5D_chunk_ud_t *udata);
typedef herr_t (*H5D_chunk_resize_func_t)(H5O_layout_chunk_t *layout);
typedef int (*H5D_chunk_iterate_func_t)(const H5D_chk_idx_info_t *idx_info,
    H5D_chunk_cb_func_t chunk_cb, void *chunk_udata);
typedef herr_t (*H5D_chunk_remove_func_t)(const H5D_chk_idx_info_t *idx_info,
    H5D_chunk_common_ud_t *udata);
typedef herr_t (*H5D_chunk_delete_func_t)(const H5D_chk_idx_info_t *idx_info);
typedef herr_t (*H5D_chunk_copy_setup_func_t)(const H5D_chk_idx_info_t *idx_info_src,
    const H5D_chk_idx_info_t *idx_info_dst);
typedef herr_t (*H5D_chunk_copy_shutdown_func_t)(H5O_storage_chunk_t *storage_src,
    H5O_storage_chunk_t *storage_dst, hid_t dxpl_id);
typedef herr_t (*H5D_chunk_size_func_t)(const H5D_chk_idx_info_t *idx_info,
    hsize_t *idx_size);
typedef herr_t (*H5D_chunk_reset_func_t)(H5O_storage_chunk_t *storage, hbool_t reset_addr);
typedef herr_t (*H5D_chunk_dump_func_t)(const H5O_storage_chunk_t *storage,
    FILE *stream);
typedef herr_t (*H5D_chunk_dest_func_t)(const H5D_chk_idx_info_t *idx_info);

/* Typedef for grouping chunk I/O routines */
typedef struct H5D_chunk_ops_t {
    H5D_chunk_init_func_t init;             /* Routine to initialize indexing information in memory */
    H5D_chunk_create_func_t create;         /* Routine to create chunk index */
    H5D_chunk_is_space_alloc_func_t is_space_alloc;    /* Query routine to determine if storage/index is allocated */
    H5D_chunk_insert_func_t insert;         /* Routine to insert a chunk into an index */
    H5D_chunk_get_addr_func_t get_addr;     /* Routine to retrieve address of chunk in file */
    H5D_chunk_resize_func_t resize;         /* Routine to update chunk index info after resizing dataset */
    H5D_chunk_iterate_func_t iterate;       /* Routine to iterate over chunks */
    H5D_chunk_remove_func_t remove;         /* Routine to remove a chunk from an index */
    H5D_chunk_delete_func_t idx_delete;     /* Routine to delete index & all chunks from file*/
    H5D_chunk_copy_setup_func_t copy_setup; /* Routine to perform any necessary setup for copying chunks */
    H5D_chunk_copy_shutdown_func_t copy_shutdown; /* Routine to perform any necessary shutdown for copying chunks */
    H5D_chunk_size_func_t size;             /* Routine to get size of indexing information */
    H5D_chunk_reset_func_t reset;           /* Routine to reset indexing information */
    H5D_chunk_dump_func_t dump;             /* Routine to dump indexing information */
    H5D_chunk_dest_func_t dest;             /* Routine to destroy indexing information in memory */
} H5D_chunk_ops_t;

/* Structure holding information about a chunk's selection for mapping */
typedef struct H5D_chunk_info_t {
    hsize_t index;              /* "Index" of chunk in dataset */
    uint32_t chunk_points;      /* Number of elements selected in chunk */
    hsize_t coords[H5O_LAYOUT_NDIMS];   /* Coordinates of chunk in file dataset's dataspace */
    H5S_t *fspace;              /* Dataspace describing chunk & selection in it */
    unsigned fspace_shared;     /* Indicate that the file space for a chunk is shared and shouldn't be freed */
    H5S_t *mspace;              /* Dataspace describing selection in memory corresponding to this chunk */
    unsigned mspace_shared;     /* Indicate that the memory space for a chunk is shared and shouldn't be freed */
} H5D_chunk_info_t;

/* Main structure holding the mapping between file chunks and memory */
typedef struct H5D_chunk_map_t {
    H5O_layout_t *layout;       /* Dataset layout information*/
    hsize_t nelmts;             /* Number of elements selected in file & memory dataspaces */

    const H5S_t *file_space;    /* Pointer to the file dataspace */
    unsigned f_ndims;           /* Number of dimensions for file dataspace */
    hsize_t f_dims[H5O_LAYOUT_NDIMS];   /* File dataspace dimensions */

    const H5S_t *mem_space;     /* Pointer to the memory dataspace */
    H5S_t *mchunk_tmpl;         /* Dataspace template for new memory chunks */
    H5S_sel_iter_t mem_iter;    /* Iterator for elements in memory selection */
    unsigned m_ndims;           /* Number of dimensions for memory dataspace */
    H5S_sel_type msel_type;     /* Selection type in memory */

    H5SL_t *sel_chunks;         /* Skip list containing information for each chunk selected */

    H5S_t  *single_space;       /* Dataspace for single chunk */
    H5D_chunk_info_t *single_chunk_info;  /* Pointer to single chunk's info */
    hbool_t use_single;         /* Whether I/O is on a single element */

    hsize_t last_index;         /* Index of last chunk operated on */
    H5D_chunk_info_t *last_chunk_info;  /* Pointer to last chunk's info */

    hsize_t chunk_dim[H5O_LAYOUT_NDIMS];    /* Size of chunk in each dimension */

#ifdef H5_HAVE_PARALLEL
    H5D_chunk_info_t **select_chunk;    /* Store the information about whether this chunk is selected or not */
#endif /* H5_HAVE_PARALLEL */
} H5D_chunk_map_t;

/* Cached information about a particular chunk */
typedef struct H5D_chunk_cached_t{
    hbool_t     valid;                          /*whether cache info is valid*/
    hsize_t	offset[H5O_LAYOUT_NDIMS];	/*logical offset to start*/
    uint32_t	nbytes;				/*size of stored data	*/
    unsigned	filter_mask;			/*excluded filters	*/
    haddr_t	addr;				/*file address of chunk */
} H5D_chunk_cached_t;

/* The raw data chunk cache */
typedef struct H5D_rdcc_t {
    struct {
        unsigned	ninits;	/* Number of chunk creations		*/
        unsigned	nhits;	/* Number of cache hits			*/
        unsigned	nmisses;/* Number of cache misses		*/
        unsigned	nflushes;/* Number of cache flushes		*/
    } stats;
    size_t		nbytes_max; /* Maximum cached raw data in bytes	*/
    size_t		nslots;	/* Number of chunk slots allocated	*/
    double		w0;     /* Chunk preemption policy          */
    struct H5D_rdcc_ent_t *head; /* Head of doubly linked list		*/
    struct H5D_rdcc_ent_t *tail; /* Tail of doubly linked list		*/
    size_t		nbytes_used; /* Current cached raw data in bytes */
    int			nused;	/* Number of chunk slots in use		*/
    H5D_chunk_cached_t last;    /* Cached copy of last chunk information */
    struct H5D_rdcc_ent_t **slot; /* Chunk slots, each points to a chunk*/
    H5SL_t		*sel_chunks; /* Skip list containing information for each chunk selected */
    H5S_t		*single_space; /* Dataspace for single element I/O on chunks */
    H5D_chunk_info_t *single_chunk_info;  /* Pointer to single chunk's info */
} H5D_rdcc_t;

/* The raw data contiguous data cache */
typedef struct H5D_rdcdc_t {
    unsigned char *sieve_buf;   /* Buffer to hold data sieve buffer */
    haddr_t sieve_loc;          /* File location (offset) of the data sieve buffer */
    size_t sieve_size;          /* Size of the data sieve buffer used (in bytes) */
    size_t sieve_buf_size;      /* Size of the data sieve buffer allocated (in bytes) */
    hbool_t sieve_dirty;        /* Flag to indicate that the data sieve buffer is dirty */
} H5D_rdcdc_t;

/*
 * A dataset is made of two layers, an H5D_t struct that is unique to
 * each instance of an opened datset, and a shared struct that is only
 * created once for a given dataset.  Thus, if a dataset is opened twice,
 * there will be two IDs and two H5D_t structs, both sharing one H5D_shared_t.
 */
typedef struct H5D_shared_t {
    size_t              fo_count;       /* Reference count */
    hid_t               type_id;        /* ID for dataset's datatype    */
    H5T_t              *type;           /* Datatype for this dataset     */
    H5S_t              *space;          /* Dataspace of this dataset    */
    hbool_t             space_dirty;    /* Whether the dataspace info needs to be flushed to the file */
    hbool_t             layout_dirty;   /* Whether the layout info needs to be flushed to the file */
    hid_t               dcpl_id;        /* Dataset creation property id */
    H5D_dcpl_cache_t    dcpl_cache;     /* Cached DCPL values */
    H5O_layout_t        layout;         /* Data layout                  */
    hbool_t             checked_filters;/* TRUE if dataset passes can_apply check */

    /* Buffered/cached information for types of raw data storage*/
    struct {
        H5D_rdcdc_t     contig;         /* Information about contiguous data */
                                        /* (Note that the "contig" cache
                                         * information can be used by a chunked
                                         * dataset in certain circumstances)
                                         */
        H5D_rdcc_t      chunk;          /* Information about chunked data */
    } cache;
} H5D_shared_t;

struct H5D_t {
    H5O_loc_t           oloc;           /* Object header location       */
    H5G_name_t          path;           /* Group hierarchy path         */
    H5D_shared_t        *shared;        /* cached information from file */
};

/* Enumerated type for allocating dataset's storage */
typedef enum {
    H5D_ALLOC_CREATE,           /* Dataset is being created */
    H5D_ALLOC_OPEN,             /* Dataset is being opened */
    H5D_ALLOC_EXTEND,           /* Dataset's dataspace is being extended */
    H5D_ALLOC_WRITE             /* Dataset is being extended */
} H5D_time_alloc_t;


/* Typedef for dataset creation operation */
typedef struct {
    hid_t type_id;              /* Datatype for dataset */
    const H5S_t *space;         /* Dataspace for dataset */
    hid_t dcpl_id;              /* Dataset creation property list */
    hid_t dapl_id;              /* Dataset access property list */
} H5D_obj_create_t;

/* Typedef for filling a buffer with a fill value */
typedef struct H5D_fill_buf_info_t {
    H5MM_allocate_t fill_alloc_func;    /* Routine to call for allocating fill buffer */
    void        *fill_alloc_info;       /* Extra info for allocation routine */
    H5MM_free_t fill_free_func;         /* Routine to call for freeing fill buffer */
    void        *fill_free_info;        /* Extra info for free routine */
    H5T_path_t *fill_to_mem_tpath;      /* Datatype conversion path for converting the fill value to the memory buffer */
    H5T_path_t *mem_to_dset_tpath;      /* Datatype conversion path for converting the memory buffer to the dataset elements */
    const H5O_fill_t *fill;             /* Pointer to fill value */
    void       *fill_buf;               /* Fill buffer */
    size_t      fill_buf_size;          /* Size of fill buffer */
    hbool_t     use_caller_fill_buf;    /* Whether the caller provided the fill buffer */
    void       *bkg_buf;                /* Background conversion buffer */
    size_t      bkg_buf_size;           /* Size of background buffer */
    H5T_t      *mem_type;               /* Pointer to memory datatype */
    const H5T_t *file_type;             /* Pointer to file datatype */
    hid_t       mem_tid;                /* ID for memory version of disk datatype */
    hid_t       file_tid;               /* ID for disk datatype */
    size_t      mem_elmt_size, file_elmt_size;       /* Size of element in memory and on disk */
    size_t      max_elmt_size;          /* Max. size of memory or file datatype */
    size_t      elmts_per_buf;          /* # of elements that fit into a buffer */
    hbool_t     has_vlen_fill_type;     /* Whether the datatype for the fill value has a variable-length component */
} H5D_fill_buf_info_t;

/* Internal data structure for computing variable-length dataset's total size */
typedef struct {
    hid_t dataset_id;   /* ID of the dataset we are working on */
    hid_t fspace_id;    /* ID of the file dataset's dataspace we are working on */
    hid_t mspace_id;    /* ID of the memory dataset's dataspace we are working on */
    void *fl_tbuf;      /* Ptr to the temporary buffer we are using for fixed-length data */
    void *vl_tbuf;      /* Ptr to the temporary buffer we are using for VL data */
    hid_t xfer_pid;     /* ID of the dataset xfer property list */
    hsize_t size;       /* Accumulated number of bytes for the selection */
} H5D_vlen_bufsize_t;

/* Raw data chunks are cached.  Each entry in the cache is: */
typedef struct H5D_rdcc_ent_t {
    hbool_t	locked;		/*entry is locked in cache		*/
    hbool_t	dirty;		/*needs to be written to disk?		*/
    hbool_t     deleted;        /*chunk about to be deleted (do not flush) */
    hsize_t	offset[H5O_LAYOUT_NDIMS]; /*chunk name			*/
    uint32_t	rd_count;	/*bytes remaining to be read		*/
    uint32_t	wr_count;	/*bytes remaining to be written		*/
    haddr_t     chunk_addr;     /*address of chunk in file		*/
    uint8_t	*chunk;		/*the unfiltered chunk data		*/
    unsigned	idx;		/*index in hash table			*/
    struct H5D_rdcc_ent_t *next;/*next item in doubly-linked list	*/
    struct H5D_rdcc_ent_t *prev;/*previous item in doubly-linked list	*/
} H5D_rdcc_ent_t;
typedef H5D_rdcc_ent_t *H5D_rdcc_ent_ptr_t; /* For free lists */


/*****************************/
/* Package Private Variables */
/*****************************/
extern H5D_dxpl_cache_t H5D_def_dxpl_cache;

/* Storage layout class I/O operations */
H5_DLLVAR const H5D_layout_ops_t H5D_LOPS_CONTIG[1];
H5_DLLVAR const H5D_layout_ops_t H5D_LOPS_EFL[1];
H5_DLLVAR const H5D_layout_ops_t H5D_LOPS_COMPACT[1];
H5_DLLVAR const H5D_layout_ops_t H5D_LOPS_CHUNK[1];

/* Chunked layout operations */
H5_DLLVAR const H5D_chunk_ops_t H5D_COPS_BTREE[1];


/******************************/
/* Package Private Prototypes */
/******************************/

H5_DLL H5D_t *H5D__create(H5F_t *file, hid_t type_id, const H5S_t *space,
    hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id);
H5_DLL H5D_t *H5D__create_named(const H5G_loc_t *loc, const char *name,
    hid_t type_id, const H5S_t *space, hid_t lcpl_id, hid_t dcpl_id,
    hid_t dapl_id, hid_t dxpl_id);
H5_DLL herr_t H5D__get_space_status(H5D_t *dset, H5D_space_status_t *allocation,
    hid_t dxpl_id);
H5_DLL herr_t H5D__alloc_storage(H5D_t *dset, hid_t dxpl_id, H5D_time_alloc_t time_alloc,
    hbool_t full_overwrite, hsize_t old_dim[]);
H5_DLL herr_t H5D__get_storage_size(H5D_t *dset, hid_t dxpl_id, hsize_t *storage_size);
H5_DLL haddr_t H5D__get_offset(const H5D_t *dset);
H5_DLL herr_t H5D__iterate(void *buf, hid_t type_id, const H5S_t *space,
    H5D_operator_t op, void *operator_data);
H5_DLL void *H5D__vlen_get_buf_size_alloc(size_t size, void *info);
H5_DLL herr_t H5D__vlen_get_buf_size(void *elem, hid_t type_id, unsigned ndim,
    const hsize_t *point, void *op_data);
H5_DLL herr_t H5D__check_filters(H5D_t *dataset);
H5_DLL herr_t H5D__set_extent(H5D_t *dataset, const hsize_t *size, hid_t dxpl_id);
H5_DLL herr_t H5D__get_dxpl_cache(hid_t dxpl_id, H5D_dxpl_cache_t **cache);
H5_DLL herr_t H5D__flush_sieve_buf(H5D_t *dataset, hid_t dxpl_id);
H5_DLL herr_t H5D__mark(H5D_t *dataset, hid_t dxpl_id, unsigned flags);
H5_DLL herr_t H5D__flush_real(H5D_t *dataset, hid_t dxpl_id);

/* Functions that perform direct serial I/O operations */
H5_DLL herr_t H5D__select_read(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);
H5_DLL herr_t H5D__select_write(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);

/* Functions that perform scatter-gather serial I/O operations */
H5_DLL herr_t H5D__scatter_mem(const void *_tscat_buf,
    const H5S_t *space, H5S_sel_iter_t *iter, size_t nelmts,
    const H5D_dxpl_cache_t *dxpl_cache, void *_buf);
H5_DLL herr_t H5D__scatgath_read(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);
H5_DLL herr_t H5D__scatgath_write(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);

/* Functions that operate on dataset's layout information */
H5_DLL herr_t H5D__layout_set_io_ops(const H5D_t *dataset);
H5_DLL size_t H5D__layout_meta_size(const H5F_t *f, const H5O_layout_t *layout,
    hbool_t include_compact_data);
H5_DLL herr_t H5D__layout_oh_create(H5F_t *file, hid_t dxpl_id, H5O_t *oh,
    H5D_t *dset, hid_t dapl_id);
H5_DLL herr_t H5D__layout_oh_read(H5D_t *dset, hid_t dxpl_id, hid_t dapl_id,
    H5P_genplist_t *plist);
H5_DLL herr_t H5D__layout_oh_write(H5D_t *dataset, hid_t dxpl_id, H5O_t *oh,
    unsigned update_flags);

/* Functions that operate on contiguous storage */
H5_DLL herr_t H5D__contig_alloc(H5F_t *f, hid_t dxpl_id,
    H5O_storage_contig_t *storage);
H5_DLL hbool_t H5D__contig_is_space_alloc(const H5O_storage_t *storage);
H5_DLL herr_t H5D__contig_fill(H5D_t *dset, hid_t dxpl_id);
H5_DLL herr_t H5D__contig_read(H5D_io_info_t *io_info, const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space,
    H5D_chunk_map_t *fm);
H5_DLL herr_t H5D__contig_write(H5D_io_info_t *io_info, const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space,
    H5D_chunk_map_t *fm);
H5_DLL herr_t H5D__contig_copy(H5F_t *f_src, const H5O_storage_contig_t *storage_src,
    H5F_t *f_dst, H5O_storage_contig_t *storage_dst, H5T_t *src_dtype,
    H5O_copy_t *cpy_info, hid_t dxpl_id);
H5_DLL herr_t H5D__contig_delete(H5F_t *f, hid_t dxpl_id,
    const H5O_storage_t *store);


/* Functions that operate on chunked dataset storage */
H5_DLL htri_t H5D__chunk_cacheable(const H5D_io_info_t *io_info, haddr_t caddr,
    hbool_t write_op);
H5_DLL herr_t H5D__chunk_create(H5D_t *dset /*in,out*/, hid_t dxpl_id);
H5_DLL herr_t H5D__chunk_set_info(const H5D_t *dset);
H5_DLL herr_t H5D__chunk_init(H5F_t *f, hid_t dxpl_id, const H5D_t *dset,
    hid_t dapl_id);
H5_DLL hbool_t H5D__chunk_is_space_alloc(const H5O_storage_t *storage);
H5_DLL herr_t H5D__chunk_lookup(const H5D_t *dset, hid_t dxpl_id,
    const hsize_t *chunk_offset, hsize_t chunk_idx, H5D_chunk_ud_t *udata);
H5_DLL void *H5D__chunk_lock(const H5D_io_info_t *io_info,
    H5D_chunk_ud_t *udata, hbool_t relax);
H5_DLL herr_t H5D__chunk_unlock(const H5D_io_info_t *io_info,
    const H5D_chunk_ud_t *udata, hbool_t dirty, void *chunk,
    uint32_t naccessed);
H5_DLL herr_t H5D__chunk_allocated(H5D_t *dset, hid_t dxpl_id, hsize_t *nbytes);
H5_DLL herr_t H5D__chunk_allocate(H5D_t *dset, hid_t dxpl_id,
    hbool_t full_overwrite, hsize_t old_dim[]);
H5_DLL herr_t H5D__chunk_prune_by_extent(H5D_t *dset, hid_t dxpl_id,
    const hsize_t *old_dim);
#ifdef H5_HAVE_PARALLEL
H5_DLL herr_t H5D__chunk_addrmap(const H5D_io_info_t *io_info, haddr_t chunk_addr[]);
#endif /* H5_HAVE_PARALLEL */
H5_DLL herr_t H5D__chunk_update_cache(H5D_t *dset, hid_t dxpl_id);
H5_DLL herr_t H5D__chunk_copy(H5F_t *f_src, H5O_storage_chunk_t *storage_src,
    H5O_layout_chunk_t *layout_src, H5F_t *f_dst, H5O_storage_chunk_t *storage_dst,
    const H5S_extent_t *ds_extent_src, const H5T_t *dt_src,
    const H5O_pline_t *pline_src, H5O_copy_t *cpy_info, hid_t dxpl_id);
H5_DLL herr_t H5D__chunk_bh_info(H5F_t *f, hid_t dxpl_id, H5O_layout_t *layout,
    const H5O_pline_t *pline, hsize_t *btree_size);
H5_DLL herr_t H5D__chunk_dump_index(H5D_t *dset, hid_t dxpl_id, FILE *stream);
H5_DLL herr_t H5D__chunk_dest(H5F_t *f, hid_t dxpl_id, H5D_t *dset);
H5_DLL herr_t H5D__chunk_delete(H5F_t *f, hid_t dxpl_id, H5O_t *oh,
    H5O_storage_t *store);
#ifdef H5D_CHUNK_DEBUG
H5_DLL herr_t H5D__chunk_stats(const H5D_t *dset, hbool_t headers);
#endif /* H5D_CHUNK_DEBUG */

/* Functions that operate on compact dataset storage */
H5_DLL herr_t H5D__compact_fill(H5D_t *dset, hid_t dxpl_id);
H5_DLL herr_t H5D__compact_copy(H5F_t *f_src, H5O_storage_compact_t *storage_src,
    H5F_t *f_dst, H5O_storage_compact_t *storage_dst, H5T_t *src_dtype,
    H5O_copy_t *cpy_info, hid_t dxpl_id);

/* Functions that operate on EFL (External File List)*/
H5_DLL hbool_t H5D__efl_is_space_alloc(const H5O_storage_t *storage);
H5_DLL herr_t H5D__efl_bh_info(H5F_t *f, hid_t dxpl_id, H5O_efl_t *efl,
    hsize_t *heap_size);

/* Functions that perform fill value operations on datasets */
H5_DLL herr_t H5D__fill(const void *fill, const H5T_t *fill_type, void *buf,
    const H5T_t *buf_type, const H5S_t *space, hid_t dxpl_id);
H5_DLL herr_t H5D__fill_init(H5D_fill_buf_info_t *fb_info, void *caller_fill_buf,
    H5MM_allocate_t alloc_func, void *alloc_info,
    H5MM_free_t free_func, void *free_info,
    const H5O_fill_t *fill, const H5T_t *dset_type, hid_t dset_type_id,
    size_t nelmts, size_t min_buf_size, hid_t dxpl_id);
H5_DLL herr_t H5D__fill_refill_vl(H5D_fill_buf_info_t *fb_info, size_t nelmts,
    hid_t dxpl_id);
H5_DLL herr_t H5D__fill_term(H5D_fill_buf_info_t *fb_info);

#ifdef H5_HAVE_PARALLEL

#ifdef H5S_DEBUG
#ifndef H5Dmpio_DEBUG
#define H5Dmpio_DEBUG
#endif /*H5Dmpio_DEBUG*/
#endif/*H5S_DEBUG*/
/* MPI-IO function to read, it will select either regular or irregular read */
H5_DLL herr_t H5D__mpio_select_read(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);

/* MPI-IO function to write, it will select either regular or irregular read */
H5_DLL herr_t H5D__mpio_select_write(const H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info,
    hsize_t nelmts, const H5S_t *file_space, const H5S_t *mem_space);

/* MPI-IO functions to handle contiguous collective IO */
H5_DLL herr_t H5D__contig_collective_read(H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, H5D_chunk_map_t *fm);
H5_DLL herr_t H5D__contig_collective_write(H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, H5D_chunk_map_t *fm);

/* MPI-IO functions to handle chunked collective IO */
H5_DLL herr_t H5D__chunk_collective_read(H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, H5D_chunk_map_t *fm);
H5_DLL herr_t H5D__chunk_collective_write(H5D_io_info_t *io_info,
    const H5D_type_info_t *type_info, hsize_t nelmts, const H5S_t *file_space,
    const H5S_t *mem_space, H5D_chunk_map_t *fm);

/* MPI-IO function to check if a direct I/O transfer is possible between
 * memory and the file */
H5_DLL htri_t H5D__mpio_opt_possible(const H5D_io_info_t *io_info,
    const H5S_t *file_space, const H5S_t *mem_space,
    const H5D_type_info_t *type_info, const H5D_chunk_map_t *fm,
    H5P_genplist_t *dx_plist);

#endif /* H5_HAVE_PARALLEL */

/* Testing functions */
#ifdef H5D_TESTING
H5_DLL herr_t H5D__layout_version_test(hid_t did, unsigned *version);
H5_DLL herr_t H5D__layout_contig_size_test(hid_t did, hsize_t *size);
H5_DLL herr_t H5D__current_cache_size_test(hid_t did, size_t *nbytes_used, int *nused);
#endif /* H5D_TESTING */

#endif /*_H5Dpkg_H*/

