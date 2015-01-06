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
 * This file contains private information about the H5S module
 */
#ifndef _H5Sprivate_H
#define _H5Sprivate_H

/* Include package's public header */
#include "H5Spublic.h"

/* Public headers needed by this file */
#include "H5Dpublic.h"		/* Datasets				*/

/* Private headers needed by this file */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Fprivate.h"		/* Files				*/
#include "H5Gprivate.h"		/* Groups				*/
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Pprivate.h"		/* Property lists			*/

/* Flags for H5S_find */
#define H5S_CONV_PAR_IO_POSSIBLE        0x0001
/* The storage options are mutually exclusive */
/* (2-bits reserved for storage type currently) */
#define H5S_CONV_STORAGE_COMPACT        0x0000  /* i.e. '0' */
#define H5S_CONV_STORAGE_CONTIGUOUS     0x0002  /* i.e. '1' */
#define H5S_CONV_STORAGE_CHUNKED        0x0004  /* i.e. '2' */
#define H5S_CONV_STORAGE_MASK           0x0006

/* Flags for "get_seq_list" methods */
#define H5S_GET_SEQ_LIST_SORTED         0x0001

/* Forward references of package typedefs */
typedef struct H5S_t H5S_t;
typedef struct H5S_extent_t H5S_extent_t;
typedef struct H5S_pnt_node_t H5S_pnt_node_t;
typedef struct H5S_hyper_span_t H5S_hyper_span_t;
typedef struct H5S_hyper_span_info_t H5S_hyper_span_info_t;

/* Information about one dimension in a hyperslab selection */
typedef struct H5S_hyper_dim_t {
    hsize_t start;
    hsize_t stride;
    hsize_t count;
    hsize_t block;
} H5S_hyper_dim_t;

/* Point selection iteration container */
typedef struct {
    H5S_pnt_node_t *curr;   /* Pointer to next node to output */
} H5S_point_iter_t;

/* Hyperslab selection iteration container */
typedef struct {
    /* Common fields for all hyperslab selections */
    hsize_t off[H5S_MAX_RANK];          /* Offset in span node (used as position for regular hyperslabs) */
    unsigned iter_rank;     /* Rank of iterator information */
                            /* (This should always be the same as the dataspace
                             * rank, except for regular hyperslab selections in
                             * which there are contiguous regions in the lower
                             * dimensions which have been "flattened" out
                             */
    hbool_t diminfo_valid;         /* Whether the dimension information is valid */

    /* "Flattened" regular hyperslab selection fields */
    H5S_hyper_dim_t diminfo[H5S_MAX_RANK];   /* "Flattened" regular selection information */
    hsize_t size[H5S_MAX_RANK];         /* "Flattened" dataspace extent information */
    hssize_t sel_off[H5S_MAX_RANK];     /* "Flattened" selection offset information */
    hbool_t flattened[H5S_MAX_RANK];    /* Whether this dimension has been flattened */

    /* Irregular hyperslab selection fields */
    H5S_hyper_span_info_t *spans;  /* Pointer to copy of the span tree */
    H5S_hyper_span_t *span[H5S_MAX_RANK];/* Array of pointers to span nodes */
} H5S_hyper_iter_t;

/* "All" selection iteration container */
typedef struct {
    hsize_t elmt_offset;         /* Next element to output */
    hsize_t byte_offset;         /* Next byte to output */
} H5S_all_iter_t;

/* Forward declaration of selection iteration class */
struct H5S_sel_iter_class_t;

/* Selection iteration container */
typedef struct H5S_sel_iter_t {
    /* Selection class */
    const struct H5S_sel_iter_class_t *type; /* Selection iteration class info */

    /* Information common to all iterators */
    unsigned rank;              /* Rank of dataspace the selection iterator is operating on */
    hsize_t *dims;              /* Dimensions of dataspace the selection is operating on */
    hsize_t elmt_left;          /* Number of elements left to iterate over */
    size_t elmt_size;           /* Size of elements to iterate over */

    /* Information specific to each type of iterator */
    union {
        H5S_point_iter_t pnt;   /* Point selection iteration information */
        H5S_hyper_iter_t hyp;   /* New Hyperslab selection iteration information */
        H5S_all_iter_t all;     /* "All" selection iteration information */
    } u;
} H5S_sel_iter_t;

/* If the module using this macro is allowed access to the private variables, access them directly */
#ifdef H5S_PACKAGE
#define H5S_GET_EXTENT_TYPE(S)          ((S)->extent.type)
#define H5S_GET_EXTENT_NDIMS(S)         ((S)->extent.rank)
#define H5S_GET_EXTENT_NPOINTS(S)       ((S)->extent.nelem)
#define H5S_GET_SELECT_NPOINTS(S)       ((S)->select.num_elem)
#define H5S_GET_SELECT_TYPE(S)          ((S)->select.type->type)
#define H5S_SELECT_GET_SEQ_LIST(S,FLAGS,ITER,MAXSEQ,MAXBYTES,NSEQ,NBYTES,OFF,LEN)             ((*(S)->select.type->get_seq_list)(S,FLAGS,ITER,MAXSEQ,MAXBYTES,NSEQ,NBYTES,OFF,LEN))
#define H5S_SELECT_VALID(S)             ((*(S)->select.type->is_valid)(S))
#define H5S_SELECT_RELEASE(S)           ((*(S)->select.type->release)(S))
#define H5S_SELECT_SERIAL_SIZE(S)       ((*(S)->select.type->serial_size)(S))
#define H5S_SELECT_SERIALIZE(S,BUF)     ((*(S)->select.type->serialize)(S,BUF))
#define H5S_SELECT_BOUNDS(S,START,END)  ((*(S)->select.type->bounds)(S,START,END))
#define H5S_SELECT_OFFSET(S, OFFSET)    ((*(S)->select.type->offset)(S, OFFSET))
#define H5S_SELECT_IS_CONTIGUOUS(S)     ((*(S)->select.type->is_contiguous)(S))
#define H5S_SELECT_IS_SINGLE(S)         ((*(S)->select.type->is_single)(S))
#define H5S_SELECT_IS_REGULAR(S)        ((*(S)->select.type->is_regular)(S))
#define H5S_SELECT_ADJUST_U(S,O)        ((*(S)->select.type->adjust_u)(S, O))
#define H5S_SELECT_PROJECT_SCALAR(S,O)  ((*(S)->select.type->project_scalar)(S, O))
#define H5S_SELECT_PROJECT_SIMPLE(S,NS, O) ((*(S)->select.type->project_simple)(S, NS, O))
#define H5S_SELECT_ITER_COORDS(ITER,COORDS)     ((*(ITER)->type->iter_coords)(ITER,COORDS))
#define H5S_SELECT_ITER_BLOCK(ITER,START,END)   ((*(ITER)->type->iter_block)(ITER,START,END))
#define H5S_SELECT_ITER_NELMTS(ITER)    ((*(ITER)->type->iter_nelmts)(ITER))
#define H5S_SELECT_ITER_HAS_NEXT_BLOCK(ITER)    ((*(ITER)->type->iter_has_next_block)(ITER))
#define H5S_SELECT_ITER_NEXT(ITER,NELEM)((*(ITER)->type->iter_next)(ITER,NELEM))
#define H5S_SELECT_ITER_NEXT_BLOCK(ITER)        ((*(ITER)->type->iter_next_block)(ITER))
#define H5S_SELECT_ITER_RELEASE(ITER)   ((*(ITER)->type->iter_release)(ITER))
#else /* H5S_PACKAGE */
#define H5S_GET_EXTENT_TYPE(S)          (H5S_get_simple_extent_type(S))
#define H5S_GET_EXTENT_NDIMS(S)         (H5S_get_simple_extent_ndims(S))
#define H5S_GET_EXTENT_NPOINTS(S)       (H5S_get_simple_extent_npoints(S))
#define H5S_GET_SELECT_NPOINTS(S)       (H5S_get_select_npoints(S))
#define H5S_GET_SELECT_TYPE(S)          (H5S_get_select_type(S))
#define H5S_SELECT_GET_SEQ_LIST(S,FLAGS,ITER,MAXSEQ,MAXBYTES,NSEQ,NBYTES,OFF,LEN)       (H5S_select_get_seq_list(S,FLAGS,ITER,MAXSEQ,MAXBYTES,NSEQ,NBYTES,OFF,LEN))
#define H5S_SELECT_VALID(S)             (H5S_select_valid(S))
#define H5S_SELECT_RELEASE(S)           (H5S_select_release(S))
#define H5S_SELECT_SERIAL_SIZE(S)       (H5S_select_serial_size(S))
#define H5S_SELECT_SERIALIZE(S,BUF)     (H5S_select_serialize(S,BUF))
#define H5S_SELECT_BOUNDS(S,START,END)  (H5S_get_select_bounds(S,START,END))
#define H5S_SELECT_OFFSET(S, OFFSET)    (H5S_get_select_offset(S, OFFSET))
#define H5S_SELECT_IS_CONTIGUOUS(S)     (H5S_select_is_contiguous(S))
#define H5S_SELECT_IS_SINGLE(S)         (H5S_select_is_single(S))
#define H5S_SELECT_IS_REGULAR(S)        (H5S_select_is_regular(S))
#define H5S_SELECT_ADJUST_U(S,O)        (H5S_select_adjust_u(S, O))
#define H5S_SELECT_PROJECT_SCALAR(S,O)  (H5S_select_project_scalar)(S, O))
#define H5S_SELECT_PROJECT_SIMPLE(S,NS,O) (H5S_select_project_simple)(S, NS, O))
#define H5S_SELECT_ITER_COORDS(ITER,COORDS)     (H5S_select_iter_coords(ITER,COORDS))
#define H5S_SELECT_ITER_BLOCK(ITER,START,END)   (H5S_select_iter_block(ITER,START,END))
#define H5S_SELECT_ITER_NELMTS(ITER)    (H5S_select_iter_nelmts(ITER))
#define H5S_SELECT_ITER_HAS_NEXT_BLOCK(ITER)    (H5S_select_iter_has_next_block(ITER))
#define H5S_SELECT_ITER_NEXT(ITER,NELEM)(H5S_select_iter_next(ITER,NELEM))
#define H5S_SELECT_ITER_NEXT_BLOCK(ITER)        (H5S_select_iter_next_block(ITER))
#define H5S_SELECT_ITER_RELEASE(ITER)   (H5S_select_iter_release(ITER))
#endif /* H5S_PACKAGE */
/* Handle these two callbacks in a special way, since they have prologs that need to be executed */
#define H5S_SELECT_COPY(DST,SRC,SHARE)  (H5S_select_copy(DST,SRC,SHARE))
#define H5S_SELECT_DESERIALIZE(S,BUF)   (H5S_select_deserialize(S,BUF))


/* Operations on dataspaces */
H5_DLL H5S_t *H5S_copy(const H5S_t *src, hbool_t share_selection, hbool_t copy_max);
H5_DLL herr_t H5S_close(H5S_t *ds);
H5_DLL H5S_class_t H5S_get_simple_extent_type(const H5S_t *ds);
H5_DLL hssize_t H5S_get_simple_extent_npoints(const H5S_t *ds);
H5_DLL hsize_t H5S_get_npoints_max(const H5S_t *ds);
H5_DLL hbool_t H5S_has_extent(const H5S_t *ds);
H5_DLL int H5S_get_simple_extent_ndims(const H5S_t *ds);
H5_DLL int H5S_get_simple_extent_dims(const H5S_t *ds, hsize_t dims[]/*out*/,
    hsize_t max_dims[]/*out*/);
H5_DLL herr_t H5S_write(H5F_t *f, hid_t dxpl_id, H5O_t *oh, unsigned update_flags,
    H5S_t *ds);
H5_DLL herr_t H5S_append(H5F_t *f, hid_t dxpl_id, struct H5O_t *oh, H5S_t *ds);
H5_DLL H5S_t *H5S_read(const struct H5O_loc_t *loc, hid_t dxpl_id);
H5_DLL htri_t H5S_set_extent(H5S_t *space, const hsize_t *size);
H5_DLL herr_t H5S_set_extent_real(H5S_t *space, const hsize_t *size);
H5_DLL H5S_t *H5S_create(H5S_class_t type);
H5_DLL H5S_t *H5S_create_simple(unsigned rank, const hsize_t dims[/*rank*/],
    const hsize_t maxdims[/*rank*/]);
H5_DLL herr_t H5S_set_latest_version(H5S_t *ds);
H5_DLL herr_t H5S_debug(H5F_t *f, hid_t dxpl_id, const void *_mesg, FILE *stream,
    int indent, int fwidth);
#ifndef H5_NO_DEPRECATED_SYMBOLS
H5_DLL int H5S_extend(H5S_t *space, const hsize_t *size);
#endif /* H5_NO_DEPRECATED_SYMBOLS */

/* Operations on dataspace extents */
H5_DLL hsize_t H5S_extent_nelem(const H5S_extent_t *ext);
H5_DLL int H5S_extent_get_dims(const H5S_extent_t *ext, hsize_t dims[], hsize_t max_dims[]);
H5_DLL htri_t H5S_extent_equal(const H5S_t *ds1, const H5S_t *ds2);

/* Operations on selections */
H5_DLL herr_t H5S_select_deserialize(H5S_t *space, const uint8_t *buf);
H5_DLL H5S_sel_type H5S_get_select_type(const H5S_t *space);
H5_DLL herr_t H5S_select_iterate(void *buf, hid_t type_id, const H5S_t *space,
    H5D_operator_t op, void *operator_data);
H5_DLL herr_t H5S_select_fill(const void *fill, size_t fill_size,
    const H5S_t *space, void *buf);
H5_DLL htri_t H5S_select_valid(const H5S_t *space);
H5_DLL hssize_t H5S_get_select_npoints(const H5S_t *space);
H5_DLL herr_t H5S_get_select_bounds(const H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL herr_t H5S_get_select_offset(const H5S_t *space, hsize_t *offset);
H5_DLL herr_t H5S_select_offset(H5S_t *space, const hssize_t *offset);
H5_DLL herr_t H5S_select_copy(H5S_t *dst, const H5S_t *src, hbool_t share_selection);
H5_DLL htri_t H5S_select_shape_same(const H5S_t *space1, const H5S_t *space2);
H5_DLL herr_t H5S_select_construct_projection(const H5S_t *base_space,
    H5S_t **new_space_ptr, unsigned new_space_rank, const void *buf,
    void const **adj_buf_ptr, hsize_t element_size);
H5_DLL herr_t H5S_select_release(H5S_t *ds);
H5_DLL herr_t H5S_select_get_seq_list(const H5S_t *space, unsigned flags,
    H5S_sel_iter_t *iter, size_t maxseq, size_t maxbytes,
    size_t *nseq, size_t *nbytes, hsize_t *off, size_t *len);
H5_DLL hssize_t H5S_select_serial_size(const H5S_t *space);
H5_DLL herr_t H5S_select_serialize(const H5S_t *space, uint8_t *buf);
H5_DLL htri_t H5S_select_is_contiguous(const H5S_t *space);
H5_DLL htri_t H5S_select_is_single(const H5S_t *space);
H5_DLL htri_t H5S_select_is_regular(const H5S_t *space);
H5_DLL herr_t H5S_select_adjust_u(H5S_t *space, const hsize_t *offset);
H5_DLL herr_t H5S_select_project_scalar(const H5S_t *space, hsize_t *offset);
H5_DLL herr_t H5S_select_project_simple(const H5S_t *space, H5S_t *new_space, hsize_t *offset);

/* Operations on all selections */
H5_DLL herr_t H5S_select_all(H5S_t *space, hbool_t rel_prev);

/* Operations on none selections */
H5_DLL herr_t H5S_select_none(H5S_t *space);

/* Operations on point selections */
H5_DLL herr_t H5S_select_elements(H5S_t *space, H5S_seloper_t op,
    size_t num_elem, const hsize_t *coord);

/* Operations on hyperslab selections */
H5_DLL herr_t H5S_select_hyperslab (H5S_t *space, H5S_seloper_t op, const hsize_t start[],
    const hsize_t *stride, const hsize_t count[], const hsize_t *block);
H5_DLL herr_t H5S_hyper_add_span_element(H5S_t *space, unsigned rank,
    hsize_t *coords);
H5_DLL herr_t H5S_hyper_reset_scratch(H5S_t *space);
H5_DLL herr_t H5S_hyper_convert(H5S_t *space);
#ifdef LATER
H5_DLL htri_t H5S_hyper_intersect (H5S_t *space1, H5S_t *space2);
#endif /* LATER */
H5_DLL htri_t H5S_hyper_intersect_block (H5S_t *space, hsize_t *start, hsize_t *end);
H5_DLL herr_t H5S_hyper_adjust_s(H5S_t *space, const hssize_t *offset);
H5_DLL htri_t H5S_hyper_normalize_offset(H5S_t *space, hssize_t *old_offset);
H5_DLL herr_t H5S_hyper_denormalize_offset(H5S_t *space, const hssize_t *old_offset);

/* Operations on selection iterators */
H5_DLL herr_t H5S_select_iter_init(H5S_sel_iter_t *iter, const H5S_t *space, size_t elmt_size);
H5_DLL herr_t H5S_select_iter_coords(const H5S_sel_iter_t *sel_iter, hsize_t *coords);
H5_DLL hsize_t H5S_select_iter_nelmts(const H5S_sel_iter_t *sel_iter);
H5_DLL herr_t H5S_select_iter_next(H5S_sel_iter_t *sel_iter, size_t nelem);
H5_DLL herr_t H5S_select_iter_release(H5S_sel_iter_t *sel_iter);

#ifdef H5_HAVE_PARALLEL
/* Global vars whose value comes from environment variable */
/* (Defined in H5S.c) */
H5_DLLVAR hbool_t		H5S_mpi_opt_types_g;

H5_DLL herr_t
H5S_mpio_space_type( const H5S_t *space, size_t elmt_size,
     /* out: */
     MPI_Datatype *new_type,
     int *count,
     hbool_t *is_derived_type );
#endif /* H5_HAVE_PARALLEL */

#endif /* _H5Sprivate_H */

