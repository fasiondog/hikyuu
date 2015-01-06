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


#ifndef _H5f90proto_H
#define _H5f90proto_H

#include "H5public.h"

H5_FCDLL char * HD5f2cstring (_fcd fdesc, size_t len);
H5_FCDLL void HD5packFstring(char *src, char *dest, size_t len);

#ifdef H5_VMS
#define H5_FC_FUNC_(name, NAME) NAME
#endif /*H5_VMS*/

/*
 * Storage info struct used by H5O_info_t and H5F_info_t 
 * interoperable with Fortran.
 */
typedef struct H5_ih_info_t_f {
    hsize_t     index_size;     /* btree and/or list */
    hsize_t     heap_size;
} H5_ih_info_t_f;

/* Information struct for object header metadata (for H5Oget_info/H5Oget_info_by_name/H5Oget_info_by_idx) 
 *  interoperable with Fortran.
 */
typedef struct H5O_hdr_info_t_f {
    int_f version;		/* Version number of header format in file */
    int_f nmesgs;		/* Number of object header messages */
    int_f nchunks;		/* Number of object header chunks */
    int_f flags;                /* Object header status flags */
    struct {
        hsize_t total;		/* Total space for storing object header in file */
        hsize_t meta;		/* Space within header for object header metadata information */
        hsize_t mesg;		/* Space within header for actual message information */
        hsize_t free;		/* Free space within object header */
    } space;
    struct {
        uint64_t present;	/* Flags to indicate presence of message type in header */
        uint64_t shared;	/* Flags to indicate message type is shared in header */
    } mesg;
} H5O_hdr_info_t_f;

/* Information struct for object (for H5Oget_info/H5Oget_info_by_name/H5Oget_info_by_idx) 
 *  interoperable with Fortran.
 */
typedef struct H5O_info_t_f {
    unsigned long 	fileno;		/* File number that object is located in */
    haddr_t_f 		addr;		/* Object address in file	*/
    int_f 		type;		/* Basic object type (group, dataset, etc.) */
    int_f 		rc;		/* Reference count of object    */
    int_f	        atime[8];	/* Access time			*/
    int_f		mtime[8];	/* Modification time		*/
    int_f		ctime[8];	/* Change time			*/
    int_f		btime[8];	/* Birth time			*/
    hsize_t 		num_attrs;	/* # of attributes attached to object */
    H5O_hdr_info_t_f    hdr;            /* Object header information */
    /* Extra metadata storage for obj & attributes */
    struct {
        H5_ih_info_t_f   obj;             /* v1/v2 B-tree & local/fractal heap for groups, B-tree for chunked datasets */
        H5_ih_info_t_f   attr;            /* v2 B-tree & heap for attributes */
    } meta_size;
} H5O_info_t_f;


/*
 *  Functions from H5Ff.c
 */
#define nh5fcreate_c              H5_FC_FUNC_(h5fcreate_c, H5FCREATE_C)
#define nh5fflush_c               H5_FC_FUNC_(h5fflush_c, H5FFLUSH_C)
#define nh5fclose_c               H5_FC_FUNC_(h5fclose_c, H5FCLOSE_C)
#define nh5fopen_c                H5_FC_FUNC_(h5fopen_c, H5FOPEN_C)
#define nh5fis_hdf5_c             H5_FC_FUNC_(h5fis_hdf5_c, H5FIS_HDF5_C)
#define nh5fmount_c               H5_FC_FUNC_(h5fmount_c, H5FMOUNT_C)
#define nh5funmount_c             H5_FC_FUNC_(h5funmount_c, H5FUNMOUNT_C)
#define nh5freopen_c              H5_FC_FUNC_(h5freopen_c, H5FREOPEN_C)
#define nh5fget_create_plist_c    H5_FC_FUNC_(h5fget_create_plist_c, H5FGET_CREATE_PLIST_C)
#define nh5fget_access_plist_c    H5_FC_FUNC_(h5fget_access_plist_c, H5FGET_ACCESS_PLIST_C)
#define nh5fget_obj_count_c       H5_FC_FUNC_(h5fget_obj_count_c, H5FGET_OBJ_COUNT_C)
#define nh5fget_obj_ids_c         H5_FC_FUNC_(h5fget_obj_ids_c, H5FGET_OBJ_IDS_C)
#define nh5fget_freespace_c       H5_FC_FUNC_(h5fget_freespace_c, H5FGET_FREESPACE_C)
#define nh5fget_name_c            H5_FC_FUNC_(h5fget_name_c, H5FGET_NAME_C)
#define nh5fget_filesize_c        H5_FC_FUNC_(h5fget_filesize_c, H5FGET_FILESIZE_C)

H5_FCDLL int_f nh5fcreate_c (_fcd name, int_f *namelen, int_f *access_flags, hid_t_f *crt_prp, hid_t_f *acc_prp, hid_t_f *file_id);
H5_FCDLL int_f nh5fopen_c (_fcd name, int_f *namelen, int_f *access_flags, hid_t_f *acc_prp, hid_t_f *file_id);
H5_FCDLL int_f nh5fis_hdf5_c (_fcd name, int_f *namelen, int_f *flag);
H5_FCDLL int_f nh5fclose_c (hid_t_f *file_id);
H5_FCDLL int_f nh5fmount_c (hid_t_f *loc_id, _fcd dsetname, int_f *namelen, hid_t_f *file_id, hid_t_f *acc_prp);
H5_FCDLL int_f nh5funmount_c (hid_t_f *loc_id, _fcd dsetname, int_f *namelen);
H5_FCDLL int_f nh5freopen_c (hid_t_f *file_id1, hid_t_f *file_id2);
H5_FCDLL int_f nh5fget_create_plist_c (hid_t_f *file_id, hid_t_f *prop_id);
H5_FCDLL int_f nh5fget_access_plist_c (hid_t_f *file_id, hid_t_f *access_id);
H5_FCDLL int_f nh5fget_obj_count_c (hid_t_f *file_id, int_f *obj_type, size_t_f *obj_count);
H5_FCDLL int_f nh5fget_obj_ids_c (hid_t_f *file_id, int_f *obj_type, size_t_f *max_objs, hid_t_f *obj_ids, size_t_f *num_objs);
H5_FCDLL int_f nh5fget_freespace_c (hid_t_f *file_id, hssize_t_f *free_space);
H5_FCDLL int_f nh5fflush_c (hid_t_f *obj_id, int_f *scope);
H5_FCDLL int_f nh5fget_name_c(hid_t_f *obj_id, size_t_f *size, _fcd buf, size_t_f *buflen);
H5_FCDLL int_f nh5fget_filesize_c(hid_t_f *file_id, hsize_t_f *size);

/*
 * Functions from H5Sf.c
 */
#define nh5screate_simple_c      H5_FC_FUNC_(h5screate_simple_c, H5SCREATE_SIMPLE_C)
#define nh5sclose_c              H5_FC_FUNC_(h5sclose_c, H5SCLOSE_C)
#define nh5screate_c             H5_FC_FUNC_(h5screate_c, H5SCREATE_C)
#define nh5scopy_c               H5_FC_FUNC_(h5scopy_c, H5SCOPY_C)
#define nh5sget_select_hyper_nblocks_c H5_FC_FUNC_(h5sget_select_hyper_nblocks_c, H5SGET_SELECT_HYPER_NBLOCKS_C)
#define nh5sget_select_hyper_blocklist_c H5_FC_FUNC_(h5sget_select_hyper_blocklist_c, H5SGET_SELECT_HYPER_BLOCKLIST_C)
#define nh5sget_select_elem_npoints_c H5_FC_FUNC_(h5sget_select_elem_npoints_c, H5SGET_SELECT_ELEM_NPOINTS_C)
#define nh5sget_select_elem_pointlist_c H5_FC_FUNC_(h5sget_select_elem_pointlist_c, H5SGET_SELECT_ELEM_POINTLIST_C)
#define nh5sget_select_bounds_c  H5_FC_FUNC_(h5sget_select_bounds_c, H5SGET_SELECT_BOUNDS_C)
#define nh5sselect_all_c         H5_FC_FUNC_(h5sselect_all_c, H5SSELECT_ALL_C)
#define nh5sselect_none_c        H5_FC_FUNC_(h5sselect_none_c, H5SSELECT_NONE_C)
#define nh5sselect_valid_c       H5_FC_FUNC_(h5sselect_valid_c, H5SSELECT_VALID_C)
#define nh5sget_simple_extent_npoints_c H5_FC_FUNC_(h5sget_simple_extent_npoints_c, H5SGET_SIMPLE_EXTENT_NPOINTS_C)
#define nh5sget_select_npoints_c H5_FC_FUNC_(h5sget_select_npoints_c, H5SGET_SELECT_NPOINTS_C)
#define nh5sget_simple_extent_ndims_c H5_FC_FUNC_(h5sget_simple_extent_ndims_c, H5SGET_SIMPLE_EXTENT_NDIMS_C)
#define nh5sget_simple_extent_type_c  H5_FC_FUNC_(h5sget_simple_extent_type_c, H5SGET_SIMPLE_EXTENT_TYPE_C)
#define nh5soffset_simple_c      H5_FC_FUNC_(h5soffset_simple_c, H5SOFFSET_SIMPLE_C)
#define nh5sset_extent_simple_c  H5_FC_FUNC_(h5sset_extent_simple_c, H5SSET_EXTENT_SIMPLE_C)
#define nh5sis_simple_c          H5_FC_FUNC_(h5sis_simple_c, H5SIS_SIMPLE_C)
#define nh5sextent_class_c       H5_FC_FUNC_(h5sextent_class_c, H5SEXTENT_CLASS_C)
#define nh5sget_simple_extent_dims_c H5_FC_FUNC_(h5sget_simple_extent_dims_c, H5SGET_SIMPLE_EXTENT_DIMS_C)
#define nh5sextent_copy_c        H5_FC_FUNC_(h5sextent_copy_c, H5SEXTENT_COPY_C)
#define nh5sset_extent_none_c    H5_FC_FUNC_(h5sset_extent_none_c, H5SSET_EXTENT_NONE_C)
#define nh5sselect_hyperslab_c   H5_FC_FUNC_(h5sselect_hyperslab_c, H5SSELECT_HYPERSLAB_C)
#define nh5scombine_hyperslab_c   H5_FC_FUNC_(h5scombine_hyperslab_c, H5SCOMBINE_HYPERSLAB_C)
#define nh5scombine_select_c   H5_FC_FUNC_(h5scombine_select_c, H5SCOMBINE_SELECT_C)
#define nh5sselect_select_c   H5_FC_FUNC_(h5sselect_select_c, H5SSELECT_SELECT_C)
#define nh5sget_select_type_c   H5_FC_FUNC_(h5sget_select_type_c, H5SGET_SELECT_TYPE_C)
#define nh5sselect_elements_c    H5_FC_FUNC_(h5sselect_elements_c, H5SSELECT_ELEMENTS_C)
#define nh5sdecode_c    H5_FC_FUNC_(h5sdecode_c, H5SDECODE_C)
#define nh5sencode_c    H5_FC_FUNC_(h5sencode_c, H5SENCODE_C)
#define nh5sextent_equal_c    H5_FC_FUNC_(h5sextent_equal_c, H5SEXTENT_EQUAL_C)

H5_FCDLL int_f nh5screate_simple_c ( int_f *rank, hsize_t_f *dims, hsize_t_f *maxdims, hid_t_f *space_id );
H5_FCDLL int_f nh5sclose_c ( hid_t_f *space_id );
H5_FCDLL int_f nh5screate_c ( int_f *classtype, hid_t_f *space_id );
H5_FCDLL int_f nh5scopy_c ( hid_t_f *space_id , hid_t_f *new_space_id);
H5_FCDLL int_f nh5sget_select_hyper_nblocks_c( hid_t_f *space_id , hssize_t_f * num_blocks);
H5_FCDLL int_f nh5sget_select_hyper_blocklist_c( hid_t_f *space_id ,hsize_t_f * startblock, hsize_t_f * num_blocks, hsize_t_f * buf);
H5_FCDLL int_f nh5sget_select_bounds_c( hid_t_f *space_id , hsize_t_f * start, hsize_t_f * end);
H5_FCDLL int_f nh5sget_select_elem_npoints_c( hid_t_f *space_id , hssize_t_f * num_points);
H5_FCDLL int_f nh5sget_select_elem_pointlist_c( hid_t_f *space_id ,hsize_t_f * startpoint, hsize_t_f * numpoints, hsize_t_f * buf);
H5_FCDLL int_f nh5sselect_all_c ( hid_t_f *space_id );
H5_FCDLL int_f nh5sselect_none_c ( hid_t_f *space_id );
H5_FCDLL int_f nh5sselect_valid_c ( hid_t_f *space_id , int_f *flag );
H5_FCDLL int_f nh5sget_simple_extent_npoints_c ( hid_t_f *space_id , hsize_t_f *npoints );
H5_FCDLL int_f nh5sget_select_npoints_c ( hid_t_f *space_id , hssize_t_f *npoints );
H5_FCDLL int_f nh5sget_simple_extent_ndims_c ( hid_t_f *space_id , int_f *ndims );
H5_FCDLL int_f nh5sget_simple_extent_type_c ( hid_t_f *space_id , int_f *classtype);
H5_FCDLL int_f nh5soffset_simple_c ( hid_t_f *space_id , hssize_t_f *offset);
H5_FCDLL int_f nh5sset_extent_simple_c ( hid_t_f *space_id , int_f *rank, hsize_t_f * current_size, hsize_t_f *maximum_size);
H5_FCDLL int_f nh5sis_simple_c ( hid_t_f *space_id , int_f *flag );
H5_FCDLL int_f nh5sextent_class_c ( hid_t_f *space_id , int_f *classtype);
H5_FCDLL int_f nh5sget_simple_extent_dims_c ( hid_t_f *space_id , hsize_t_f *dims, hsize_t_f *maxdims);
H5_FCDLL int_f nh5sextent_copy_c ( hid_t_f *dest_space_id , hid_t_f *source_space_id);
H5_FCDLL int_f nh5sset_extent_none_c ( hid_t_f *space_id );
H5_FCDLL int_f nh5sselect_hyperslab_c ( hid_t_f *space_id , int_f *op, hsize_t_f *start, hsize_t_f *count, hsize_t_f *stride, hsize_t_f *block);
H5_FCDLL int_f nh5sget_select_type_c ( hid_t_f *space_id , int_f *op);
H5_FCDLL int_f nh5sselect_elements_c ( hid_t_f *space_id , int_f *op, size_t_f *nelements, hsize_t_f *coord);
H5_FCDLL int_f nh5scombine_hyperslab_c ( hid_t_f *space_id , int_f *op, hsize_t_f *start, hsize_t_f *count, hsize_t_f *stride, hsize_t_f *block, hid_t_f *hyper_id);
H5_FCDLL int_f nh5scombine_select_c ( hid_t_f *space1_id , int_f *op, hid_t_f *space2_id, hid_t_f *ds_id);
H5_FCDLL int_f nh5sselect_select_c ( hid_t_f *space1_id , int_f *op, hid_t_f *space2_id);
H5_FCDLL int_f nh5sdecode_c ( _fcd buf, hid_t_f *obj_id );
H5_FCDLL int_f nh5sencode_c (_fcd buf, hid_t_f *obj_id, size_t_f *nalloc );
H5_FCDLL int_f nh5sextent_equal_c ( hid_t_f * space1_id, hid_t_f *space2_id, hid_t_f *c_equal);

/*
 * Functions from H5Df.c
 */
#define nh5dcreate_c                H5_FC_FUNC_(h5dcreate_c, H5DCREATE_C)
#define nh5dclose_c                 H5_FC_FUNC_(h5dclose_c, H5DCLOSE_C)
#define nh5dopen_c                  H5_FC_FUNC_(h5dopen_c, H5DOPEN_C)
#define nh5dwrite_c                 H5_FC_FUNC_(h5dwrite_c, H5DWRITE_C)
#define nh5dwrite_integer_s_c       H5_FC_FUNC_(h5dwrite_integer_s_c, H5DWRITE_INTEGER_S_C)
#define nh5dwrite_integer_1_c       H5_FC_FUNC_(h5dwrite_integer_1_c, H5DWRITE_INTEGER_1_C)
#define nh5dwrite_integer_2_c       H5_FC_FUNC_(h5dwrite_integer_2_c, H5DWRITE_INTEGER_2_C)
#define nh5dwrite_integer_3_c       H5_FC_FUNC_(h5dwrite_integer_3_c, H5DWRITE_INTEGER_3_C)
#define nh5dwrite_integer_4_c       H5_FC_FUNC_(h5dwrite_integer_4_c, H5DWRITE_INTEGER_4_C)
#define nh5dwrite_integer_5_c       H5_FC_FUNC_(h5dwrite_integer_5_c, H5DWRITE_INTEGER_5_C)
#define nh5dwrite_integer_6_c       H5_FC_FUNC_(h5dwrite_integer_6_c, H5DWRITE_INTEGER_6_C)
#define nh5dwrite_integer_7_c       H5_FC_FUNC_(h5dwrite_integer_7_c, H5DWRITE_INTEGER_7_C)
#define nh5dwrite_real_s_c          H5_FC_FUNC_(h5dwrite_real_s_c, H5DWRITE_REAL_S_C)
#define nh5dwrite_real_1_c          H5_FC_FUNC_(h5dwrite_real_1_c, H5DWRITE_REAL_1_C)
#define nh5dwrite_real_2_c          H5_FC_FUNC_(h5dwrite_real_2_c, H5DWRITE_REAL_2_C)
#define nh5dwrite_real_3_c          H5_FC_FUNC_(h5dwrite_real_3_c, H5DWRITE_REAL_3_C)
#define nh5dwrite_real_4_c          H5_FC_FUNC_(h5dwrite_real_4_c, H5DWRITE_REAL_4_C)
#define nh5dwrite_real_5_c          H5_FC_FUNC_(h5dwrite_real_5_c, H5DWRITE_REAL_5_C)
#define nh5dwrite_real_6_c          H5_FC_FUNC_(h5dwrite_real_6_c, H5DWRITE_REAL_6_C)
#define nh5dwrite_real_7_c          H5_FC_FUNC_(h5dwrite_real_7_c, H5DWRITE_REAL_7_C)
#define nh5dwrite_double_s_c        H5_FC_FUNC_(h5dwrite_double_s_c, H5DWRITE_DOUBLE_S_C)
#define nh5dwrite_double_1_c        H5_FC_FUNC_(h5dwrite_double_1_c, H5DWRITE_DOUBLE_1_C)
#define nh5dwrite_double_2_c        H5_FC_FUNC_(h5dwrite_double_2_c, H5DWRITE_DOUBLE_2_C)
#define nh5dwrite_double_3_c        H5_FC_FUNC_(h5dwrite_double_3_c, H5DWRITE_DOUBLE_3_C)
#define nh5dwrite_double_4_c        H5_FC_FUNC_(h5dwrite_double_4_c, H5DWRITE_DOUBLE_4_C)
#define nh5dwrite_double_5_c        H5_FC_FUNC_(h5dwrite_double_5_c, H5DWRITE_DOUBLE_5_C)
#define nh5dwrite_double_6_c        H5_FC_FUNC_(h5dwrite_double_6_c, H5DWRITE_DOUBLE_6_C)
#define nh5dwrite_double_7_c        H5_FC_FUNC_(h5dwrite_double_7_c, H5DWRITE_DOUBLE_7_C)
#define nh5dwrite_ref_obj_c         H5_FC_FUNC_(h5dwrite_ref_obj_c, H5DWRITE_REF_OBJ_C)
#define nh5dwrite_ref_reg_c         H5_FC_FUNC_(h5dwrite_ref_reg_c, H5DWRITE_REF_REG_C)
#define nh5dwritec_c                H5_FC_FUNC_(h5dwritec_c, H5DWRITEC_C)
#define nh5dwritec_s_c                H5_FC_FUNC_(h5dwritec_s_c, H5DWRITEC_S_C)
#define nh5dwritec_1_c                H5_FC_FUNC_(h5dwritec_1_c, H5DWRITEC_1_C)
#define nh5dwritec_2_c                H5_FC_FUNC_(h5dwritec_2_c, H5DWRITEC_2_C)
#define nh5dwritec_3_c                H5_FC_FUNC_(h5dwritec_3_c, H5DWRITEC_3_C)
#define nh5dwritec_4_c                H5_FC_FUNC_(h5dwritec_4_c, H5DWRITEC_4_C)
#define nh5dwritec_5_c                H5_FC_FUNC_(h5dwritec_5_c, H5DWRITEC_5_C)
#define nh5dwritec_6_c                H5_FC_FUNC_(h5dwritec_6_c, H5DWRITEC_6_C)
#define nh5dwritec_7_c                H5_FC_FUNC_(h5dwritec_7_c, H5DWRITEC_7_C)
#define nh5dread_c                  H5_FC_FUNC_(h5dread_c, H5DREAD_C)
#define nh5dread_integer_s_c                 H5_FC_FUNC_(h5dread_integer_s_c, H5DREAD_INTEGER_S_C)
#define nh5dread_integer_1_c                 H5_FC_FUNC_(h5dread_integer_1_c, H5DREAD_INTEGER_1_C)
#define nh5dread_integer_2_c                 H5_FC_FUNC_(h5dread_integer_2_c, H5DREAD_INTEGER_2_C)
#define nh5dread_integer_3_c                 H5_FC_FUNC_(h5dread_integer_3_c, H5DREAD_INTEGER_3_C)
#define nh5dread_integer_4_c                 H5_FC_FUNC_(h5dread_integer_4_c, H5DREAD_INTEGER_4_C)
#define nh5dread_integer_5_c                 H5_FC_FUNC_(h5dread_integer_5_c, H5DREAD_INTEGER_5_C)
#define nh5dread_integer_6_c                 H5_FC_FUNC_(h5dread_integer_6_c, H5DREAD_INTEGER_6_C)
#define nh5dread_integer_7_c                 H5_FC_FUNC_(h5dread_integer_7_c, H5DREAD_INTEGER_7_C)
#define nh5dread_real_s_c                 H5_FC_FUNC_(h5dread_real_s_c, H5DREAD_REAL_S_C)
#define nh5dread_real_1_c                 H5_FC_FUNC_(h5dread_real_1_c, H5DREAD_REAL_1_C)
#define nh5dread_real_2_c                 H5_FC_FUNC_(h5dread_real_2_c, H5DREAD_REAL_2_C)
#define nh5dread_real_3_c                 H5_FC_FUNC_(h5dread_real_3_c, H5DREAD_REAL_3_C)
#define nh5dread_real_4_c                 H5_FC_FUNC_(h5dread_real_4_c, H5DREAD_REAL_4_C)
#define nh5dread_real_5_c                 H5_FC_FUNC_(h5dread_real_5_c, H5DREAD_REAL_5_C)
#define nh5dread_real_6_c                 H5_FC_FUNC_(h5dread_real_6_c, H5DREAD_REAL_6_C)
#define nh5dread_real_7_c                 H5_FC_FUNC_(h5dread_real_7_c, H5DREAD_REAL_7_C)
#define nh5dread_double_s_c                 H5_FC_FUNC_(h5dread_double_s_c, H5DREAD_DOUBLE_S_C)
#define nh5dread_double_1_c                 H5_FC_FUNC_(h5dread_double_1_c, H5DREAD_DOUBLE_1_C)
#define nh5dread_double_2_c                 H5_FC_FUNC_(h5dread_double_2_c, H5DREAD_DOUBLE_2_C)
#define nh5dread_double_3_c                 H5_FC_FUNC_(h5dread_double_3_c, H5DREAD_DOUBLE_3_C)
#define nh5dread_double_4_c                 H5_FC_FUNC_(h5dread_double_4_c, H5DREAD_DOUBLE_4_C)
#define nh5dread_double_5_c                 H5_FC_FUNC_(h5dread_double_5_c, H5DREAD_DOUBLE_5_C)
#define nh5dread_double_6_c                 H5_FC_FUNC_(h5dread_double_6_c, H5DREAD_DOUBLE_6_C)
#define nh5dread_double_7_c                 H5_FC_FUNC_(h5dread_double_7_c, H5DREAD_DOUBLE_7_C)
#define nh5dread_c_b                H5_FC_FUNC_(h5dread_c_b, H5DREAD_C_B)
#define nh5dread_ref_reg_c          H5_FC_FUNC_(h5dread_ref_reg_c, H5DREAD_REF_REG_C)
#define nh5dread_ref_obj_c          H5_FC_FUNC_(h5dread_ref_obj_c, H5DREAD_REF_OBJ_C)
#define nh5dreadc_c                 H5_FC_FUNC_(h5dreadc_c, H5DREADC_C)
#define nh5dreadc_s_c                 H5_FC_FUNC_(h5dreadc_s_c, H5DREADC_S_C)
#define nh5dreadc_1_c                 H5_FC_FUNC_(h5dreadc_1_c, H5DREADC_1_C)
#define nh5dreadc_2_c                 H5_FC_FUNC_(h5dreadc_2_c, H5DREADC_2_C)
#define nh5dreadc_3_c                 H5_FC_FUNC_(h5dreadc_3_c, H5DREADC_3_C)
#define nh5dreadc_4_c                 H5_FC_FUNC_(h5dreadc_4_c, H5DREADC_4_C)
#define nh5dreadc_5_c                 H5_FC_FUNC_(h5dreadc_5_c, H5DREADC_5_C)
#define nh5dreadc_6_c                 H5_FC_FUNC_(h5dreadc_6_c, H5DREADC_6_C)
#define nh5dreadc_7_c                 H5_FC_FUNC_(h5dreadc_7_c, H5DREADC_7_C)
#define nh5dreadc_c_b               H5_FC_FUNC_(h5dreadc_c_b, H5DREADC_C_B)
#define nh5dget_space_c             H5_FC_FUNC_(h5dget_space_c, H5DGET_SPACE_C)
#define nh5dget_type_c              H5_FC_FUNC_(h5dget_type_c, H5DGET_TYPE_C)
#define nh5dget_create_plist_c      H5_FC_FUNC_(h5dget_create_plist_c, H5DGET_CREATE_PLIST_C)
#define nh5dset_extent_c            H5_FC_FUNC_(h5dset_extent_c, H5DSET_EXTENT_C)
#define nh5dget_storage_size_c      H5_FC_FUNC_(h5dget_storage_size_c, H5DGET_STORAGE_SIZE_C)
#define nh5dvlen_get_max_len_c      H5_FC_FUNC_(h5dvlen_get_max_len_c, H5DVLEN_GET_MAX_LEN_C)
#define nh5dwrite_vl_integer_c      H5_FC_FUNC_(h5dwrite_vl_integer_c, H5DWRITE_VL_INTEGER_C)
#define nh5dread_vl_integer_c       H5_FC_FUNC_(h5dread_vl_integer_c, H5DREAD_VL_INTEGER_C)
#define nh5dwrite_vl_real_c         H5_FC_FUNC_(h5dwrite_vl_real_c, H5DWRITE_VL_REAL_C)
#define nh5dread_vl_real_c          H5_FC_FUNC_(h5dread_vl_real_c, H5DREAD_VL_REAL_C)
#define nh5dwrite_vl_string_c       H5_FC_FUNC_(h5dwrite_vl_string_c, H5DWRITE_VL_STRING_C)
#define nh5dread_vl_string_c        H5_FC_FUNC_(h5dread_vl_string_c, H5DREAD_VL_STRING_C)
#define nh5dfillc_c                 H5_FC_FUNC_(h5dfillc_c, H5DFILLC_C)
#define nh5dfill_c                  H5_FC_FUNC_(h5dfill_c, H5DFILL_C)
#define nh5dfill_integer_c                  H5_FC_FUNC_(h5dfill_integer_c, H5DFILL_INTEGER_C)
#define nh5dfill_real_c                  H5_FC_FUNC_(h5dfill_real_c, H5DFILL_REAL_C)
#define nh5dfill_double_c                  H5_FC_FUNC_(h5dfill_double_c, H5DFILL_DOUBLE_C)
#define nh5dget_space_status_c    H5_FC_FUNC_(h5dget_space_status_c, H5DGET_SPACE_STATUS_C)
#define nh5dcreate_anon_c         H5_FC_FUNC_(h5dcreate_anon_c, H5DCREATE_ANON_C)
#define nh5dwrite_f_c             H5_FC_FUNC_(h5dwrite_f_c, H5DWRITE_F_C)
#define nh5dread_f_c              H5_FC_FUNC_(h5dread_f_c, H5DREAD_F_C)
#define nh5dget_access_plist_c    H5_FC_FUNC_(h5dget_access_plist_c, H5DGET_ACCESS_PLIST_C)
#define nh5dvlen_reclaim_c        H5_FC_FUNC_(h5dvlen_reclaim_c, H5DVLEN_RECLAIM_C)


H5_FCDLL int_f nh5dcreate_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *type_id, hid_t_f *space_id,
			     hid_t_f *lcpl_id, hid_t_f *dcpl_id, hid_t_f *dapl_id, hid_t_f *dset_id);
H5_FCDLL int_f nh5dopen_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *dapl_id, hid_t_f *dset_id);
H5_FCDLL int_f nh5dclose_c ( hid_t_f *dset_id );
H5_FCDLL int_f nh5dwrite_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_integer_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dwrite_real_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_real_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dwrite_double_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_double_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dwrite_vl_integer_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dwrite_vl_real_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, real_f *buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dwrite_vl_string_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dwrite_ref_obj_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, haddr_t_f *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwrite_ref_reg_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dwritec_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dwritec_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dwritec_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dread_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dread_integer_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_integer_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dread_real_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_real_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dread_double_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_double_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dread_vl_integer_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f *buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dread_vl_real_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, real_f *buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dread_vl_string_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims, size_t_f *len);
H5_FCDLL int_f nh5dread_ref_obj_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, haddr_t_f * buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dread_ref_reg_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, int_f * buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dreadc_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);

H5_FCDLL int_f nh5dreadc_s_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_1_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_2_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_3_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_4_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_5_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_6_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dreadc_7_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, hid_t_f *file_space_id, hid_t_f *xfer_prp, _fcd buf, hsize_t_f *dims);
H5_FCDLL int_f nh5dget_access_plist_c (hid_t_f *dset_id, hid_t_f *plist_id);


H5_FCDLL int_f nh5dget_space_c ( hid_t_f *dset_id , hid_t_f *space_id);
H5_FCDLL int_f nh5dget_type_c ( hid_t_f *dset_id , hid_t_f *type_id);
H5_FCDLL int_f nh5dget_create_plist_c ( hid_t_f *dset_id , hid_t_f *plist_id);
H5_FCDLL int_f nh5dset_extent_c ( hid_t_f *dset_id , hsize_t_f *dims);
H5_FCDLL int_f nh5dvlen_get_max_len_c(hid_t_f *dataset_id, hid_t_f *type_id, hid_t_f *space_id, size_t_f *len);
H5_FCDLL int_f nh5dget_storage_size_c(hid_t_f *dataset_id, hsize_t_f *size);
H5_FCDLL int_f nh5dfillc_c(_fcd fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, _fcd buf, hid_t_f *mem_type_id);
H5_FCDLL int_f nh5dfill_c(void * fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, void * buf, hid_t_f *mem_type_id);
H5_FCDLL int_f nh5dfill_integer_c(void * fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, void * buf, hid_t_f *mem_type_id);
H5_FCDLL int_f nh5dfill_real_c(void * fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, void * buf, hid_t_f *mem_type_id);
H5_FCDLL int_f nh5dfill_double_c(void * fill_value, hid_t_f *fill_type_id, hid_t_f *space_id, void * buf, hid_t_f *mem_type_id);
H5_FCDLL int_f nh5dget_space_status_c ( hid_t_f *dset_id, int_f *flag);
H5_FCDLL int_f nh5dcreate_anon_c (hid_t_f *loc_id, hid_t_f *type_id, hid_t_f *space_id,
				  hid_t_f *dcpl_id, hid_t_f *dapl_id, hid_t_f *dset_id);
H5_FCDLL int_f nh5dwrite_f_c (hid_t_f *dset_id, hid_t_f *mem_type_id, hid_t_f *mem_space_id, 
				   hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf);
H5_FCDLL int_f nh5dread_f_c ( hid_t_f *dset_id ,  hid_t_f *mem_type_id, hid_t_f *mem_space_id, 
				   hid_t_f *file_space_id, hid_t_f *xfer_prp, void *buf);
H5_FCDLL int_f nh5dvlen_reclaim_c (hid_t_f *type_id ,  hid_t_f *space_id, hid_t_f *plist_id, void *buf);
/*
 * Functions from H5Gf.c
 */
#define nh5gcreate_c      H5_FC_FUNC_(h5gcreate_c, H5GCREATE_C)
#define nh5gclose_c       H5_FC_FUNC_(h5gclose_c, H5GCLOSE_C)
#define nh5gopen_c        H5_FC_FUNC_(h5gopen_c, H5GOPEN_C)
#define nh5gget_obj_info_idx_c H5_FC_FUNC_(h5gget_obj_info_idx_c, H5GGET_OBJ_INFO_IDX_C)
#define nh5gn_members_c   H5_FC_FUNC_(h5gn_members_c, H5GN_MEMBERS_C)
#define nh5glink_c        H5_FC_FUNC_(h5glink_c, H5GLINK_C)
#define nh5glink2_c        H5_FC_FUNC_(h5glink2_c, H5GLINK2_C)
#define nh5gunlink_c      H5_FC_FUNC_(h5gunlink_c, H5GUNLINK_C)
#define nh5gmove_c        H5_FC_FUNC_(h5gmove_c, H5GMOVE_C)
#define nh5gmove2_c        H5_FC_FUNC_(h5gmove2_c, H5GMOVE2_C)
#define nh5gget_linkval_c   H5_FC_FUNC_(h5gget_linkval_c, H5GGET_LINKVAL_C)
#define nh5gset_comment_c   H5_FC_FUNC_(h5gset_comment_c, H5GSET_COMMENT_C)
#define nh5gget_comment_c   H5_FC_FUNC_(h5gget_comment_c, H5GGET_COMMENT_C)
#define nh5gcreate_anon_c      H5_FC_FUNC_(h5gcreate_anon_c, H5GCREATE_ANON_C)
#define nh5gget_create_plist_c H5_FC_FUNC_(h5gget_create_plist_c, H5GGET_CREATE_PLIST_C)
#define nh5gget_info_c  H5_FC_FUNC_(h5gget_info_c, H5GGET_INFO_C)
#define nh5gget_info_by_idx_c  H5_FC_FUNC_(h5gget_info_by_idx_c, H5GGET_INFO_BY_IDX_C)
#define nh5gget_info_by_name_c  H5_FC_FUNC_(h5gget_info_by_name_c, H5GGET_INFO_BY_NAME_C)


H5_FCDLL int_f nh5gcreate_c (hid_t_f *loc_id, _fcd name, int_f *namelen, size_t_f *size_hint,  hid_t_f *grp_id,
			     hid_t_f *lcpl_id, hid_t_f *gcpl_id, hid_t_f *gapl_id);
H5_FCDLL int_f nh5gopen_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *gapl_id, hid_t_f *grp_id);
H5_FCDLL int_f nh5gclose_c ( hid_t_f *grp_id );
H5_FCDLL int_f nh5gget_obj_info_idx_c (hid_t_f *loc_id, _fcd name, int_f *namelen, int_f *idx, _fcd obj_name, int_f *obj_namelen, int_f *obj_type);
H5_FCDLL int_f nh5gn_members_c (hid_t_f *loc_id, _fcd name, int_f *namelen, int_f *nmembers);
H5_FCDLL int_f nh5glink_c (hid_t_f *loc_id, int_f *link_type, _fcd current_name, int_f *current_namelen, _fcd new_name, int_f *new_namelen);
H5_FCDLL int_f nh5glink2_c (hid_t_f *cur_loc_id, _fcd cur_name, int_f *cur_namelen, int_f *link_type, hid_t_f *new_loc_id, _fcd new_name, int_f *new_namelen);
H5_FCDLL int_f nh5gunlink_c (hid_t_f *loc_id, _fcd name, int_f *namelen);
H5_FCDLL int_f nh5gmove_c (hid_t_f *loc_id, _fcd src_name, int_f *src_namelen, _fcd dst_name, int_f *dst_namelen);
H5_FCDLL int_f nh5gmove2_c (hid_t_f *src_loc_id, _fcd src_name, int_f *src_namelen, hid_t_f *dst_loc_id,_fcd dst_name, int_f *dst_namelen);
H5_FCDLL int_f nh5gget_linkval_c (hid_t_f *loc_id, _fcd name, int_f *namelen, size_t_f *size, _fcd value );
H5_FCDLL int_f nh5gset_comment_c (hid_t_f *loc_id, _fcd name, int_f *namelen, _fcd comment, int_f *commentlen);
H5_FCDLL int_f nh5gget_comment_c (hid_t_f *loc_id, _fcd name, int_f *namelen, size_t_f *bufsize, _fcd comment);
H5_FCDLL int_f nh5gcreate_anon_c (hid_t_f *loc_id, hid_t_f *gcpl_id, hid_t_f *gapl_id, hid_t_f *grp_id);
H5_FCDLL int_f nh5gget_create_plist_c(hid_t_f *grp_id, hid_t_f *gcpl_id );
H5_FCDLL int_f nh5gget_info_c (hid_t_f *group_id, int_f *storage_type, int_f *nlinks, int_f *max_corder, int_f *mounted);
H5_FCDLL int_f nh5gget_info_by_idx_c(hid_t_f *loc_id, _fcd group_name, size_t_f *group_namelen,
				     int_f *index_type, int_f *order, hsize_t_f *n, hid_t_f *lapl_id,
				     int_f *storage_type, int_f *nlinks, int_f *max_corder, int_f *mounted);
H5_FCDLL int_f nh5gget_info_by_name_c(hid_t_f *loc_id, _fcd group_name, size_t_f *group_namelen, hid_t_f *lapl_id,
				      int_f *storage_type, int_f *nlinks, int_f *max_corder, int_f *mounted);

/*
 * Functions from H5Af.c
 */
#define nh5acreate_c      H5_FC_FUNC_(h5acreate_c, H5ACREATE_C)
#define nh5aclose_c       H5_FC_FUNC_(h5aclose_c, H5ACLOSE_C)
#define nh5aopen_name_c   H5_FC_FUNC_(h5aopen_name_c, H5AOPEN_NAME_C)
#define nh5awrite_c       H5_FC_FUNC_(h5awrite_c, H5AWRITE_C)
#define nh5awrite_integer_s_c       H5_FC_FUNC_(h5awrite_integer_s_c, H5AWRITE_INTEGER_S_C)
#define nh5awrite_integer_1_c       H5_FC_FUNC_(h5awrite_integer_1_c, H5AWRITE_INTEGER_1_C)
#define nh5awrite_integer_2_c       H5_FC_FUNC_(h5awrite_integer_2_c, H5AWRITE_INTEGER_2_C)
#define nh5awrite_integer_3_c       H5_FC_FUNC_(h5awrite_integer_3_c, H5AWRITE_INTEGER_3_C)
#define nh5awrite_integer_4_c       H5_FC_FUNC_(h5awrite_integer_4_c, H5AWRITE_INTEGER_4_C)
#define nh5awrite_integer_5_c       H5_FC_FUNC_(h5awrite_integer_5_c, H5AWRITE_INTEGER_5_C)
#define nh5awrite_integer_6_c       H5_FC_FUNC_(h5awrite_integer_6_c, H5AWRITE_INTEGER_6_C)
#define nh5awrite_integer_7_c       H5_FC_FUNC_(h5awrite_integer_7_c, H5AWRITE_INTEGER_7_C)
#define nh5awrite_real_s_c       H5_FC_FUNC_(h5awrite_real_s_c, H5AWRITE_REAL_S_C)
#define nh5awrite_real_1_c       H5_FC_FUNC_(h5awrite_real_1_c, H5AWRITE_REAL_1_C)
#define nh5awrite_real_2_c       H5_FC_FUNC_(h5awrite_real_2_c, H5AWRITE_REAL_2_C)
#define nh5awrite_real_3_c       H5_FC_FUNC_(h5awrite_real_3_c, H5AWRITE_REAL_3_C)
#define nh5awrite_real_4_c       H5_FC_FUNC_(h5awrite_real_4_c, H5AWRITE_REAL_4_C)
#define nh5awrite_real_5_c       H5_FC_FUNC_(h5awrite_real_5_c, H5AWRITE_REAL_5_C)
#define nh5awrite_real_6_c       H5_FC_FUNC_(h5awrite_real_6_c, H5AWRITE_REAL_6_C)
#define nh5awrite_real_7_c       H5_FC_FUNC_(h5awrite_real_7_c, H5AWRITE_REAL_7_C)
#define nh5awrite_double_s_c       H5_FC_FUNC_(h5awrite_double_s_c, H5AWRITE_DOUBLE_S_C)
#define nh5awrite_double_1_c       H5_FC_FUNC_(h5awrite_double_1_c, H5AWRITE_DOUBLE_1_C)
#define nh5awrite_double_2_c       H5_FC_FUNC_(h5awrite_double_2_c, H5AWRITE_DOUBLE_2_C)
#define nh5awrite_double_3_c       H5_FC_FUNC_(h5awrite_double_3_c, H5AWRITE_DOUBLE_3_C)
#define nh5awrite_double_4_c       H5_FC_FUNC_(h5awrite_double_4_c, H5AWRITE_DOUBLE_4_C)
#define nh5awrite_double_5_c       H5_FC_FUNC_(h5awrite_double_5_c, H5AWRITE_DOUBLE_5_C)
#define nh5awrite_double_6_c       H5_FC_FUNC_(h5awrite_double_6_c, H5AWRITE_DOUBLE_6_C)
#define nh5awrite_double_7_c       H5_FC_FUNC_(h5awrite_double_7_c, H5AWRITE_DOUBLE_7_C)
#define nh5awritec_c        H5_FC_FUNC_(h5awritec_c, H5AWRITEC_C)
#define nh5awritec_s_c      H5_FC_FUNC_(h5awritec_s_c, H5AWRITEC_S_C)
#define nh5awritec_1_c      H5_FC_FUNC_(h5awritec_1_c, H5AWRITEC_1_C)
#define nh5awritec_2_c      H5_FC_FUNC_(h5awritec_2_c, H5AWRITEC_2_C)
#define nh5awritec_3_c      H5_FC_FUNC_(h5awritec_3_c, H5AWRITEC_3_C)
#define nh5awritec_4_c      H5_FC_FUNC_(h5awritec_4_c, H5AWRITEC_4_C)
#define nh5awritec_5_c      H5_FC_FUNC_(h5awritec_5_c, H5AWRITEC_5_C)
#define nh5awritec_6_c      H5_FC_FUNC_(h5awritec_6_c, H5AWRITEC_6_C)
#define nh5awritec_7_c      H5_FC_FUNC_(h5awritec_7_c, H5AWRITEC_7_C)
#define nh5awrite_f_c       H5_FC_FUNC_(h5awrite_f_c, H5AWRITE_F_C)
#define nh5aread_c                 H5_FC_FUNC_(h5aread_c, H5AREAD_C)
#define nh5aread_integer_s_c        H5_FC_FUNC_(h5aread_integer_s_c, H5AREAD_INTEGER_S_C)
#define nh5aread_integer_1_c        H5_FC_FUNC_(h5aread_integer_1_c, H5AREAD_INTEGER_1_C)
#define nh5aread_integer_2_c        H5_FC_FUNC_(h5aread_integer_2_c, H5AREAD_INTEGER_2_C)
#define nh5aread_integer_3_c        H5_FC_FUNC_(h5aread_integer_3_c, H5AREAD_INTEGER_3_C)
#define nh5aread_integer_4_c        H5_FC_FUNC_(h5aread_integer_4_c, H5AREAD_INTEGER_4_C)
#define nh5aread_integer_5_c        H5_FC_FUNC_(h5aread_integer_5_c, H5AREAD_INTEGER_5_C)
#define nh5aread_integer_6_c        H5_FC_FUNC_(h5aread_integer_6_c, H5AREAD_INTEGER_6_C)
#define nh5aread_integer_7_c        H5_FC_FUNC_(h5aread_integer_7_c, H5AREAD_INTEGER_7_C)
#define nh5aread_real_s_c        H5_FC_FUNC_(h5aread_real_s_c, H5AREAD_REAL_S_C)
#define nh5aread_real_1_c        H5_FC_FUNC_(h5aread_real_1_c, H5AREAD_REAL_1_C)
#define nh5aread_real_2_c        H5_FC_FUNC_(h5aread_real_2_c, H5AREAD_REAL_2_C)
#define nh5aread_real_3_c        H5_FC_FUNC_(h5aread_real_3_c, H5AREAD_REAL_3_C)
#define nh5aread_real_4_c        H5_FC_FUNC_(h5aread_real_4_c, H5AREAD_REAL_4_C)
#define nh5aread_real_5_c        H5_FC_FUNC_(h5aread_real_5_c, H5AREAD_REAL_5_C)
#define nh5aread_real_6_c        H5_FC_FUNC_(h5aread_real_6_c, H5AREAD_REAL_6_C)
#define nh5aread_real_7_c        H5_FC_FUNC_(h5aread_real_7_c, H5AREAD_REAL_7_C)
#define nh5aread_double_s_c        H5_FC_FUNC_(h5aread_double_s_c, H5AREAD_DOUBLE_S_C)
#define nh5aread_double_1_c        H5_FC_FUNC_(h5aread_double_1_c, H5AREAD_DOUBLE_1_C)
#define nh5aread_double_2_c        H5_FC_FUNC_(h5aread_double_2_c, H5AREAD_DOUBLE_2_C)
#define nh5aread_double_3_c        H5_FC_FUNC_(h5aread_double_3_c, H5AREAD_DOUBLE_3_C)
#define nh5aread_double_4_c        H5_FC_FUNC_(h5aread_double_4_c, H5AREAD_DOUBLE_4_C)
#define nh5aread_double_5_c        H5_FC_FUNC_(h5aread_double_5_c, H5AREAD_DOUBLE_5_C)
#define nh5aread_double_6_c        H5_FC_FUNC_(h5aread_double_6_c, H5AREAD_DOUBLE_6_C)
#define nh5aread_double_7_c        H5_FC_FUNC_(h5aread_double_7_c, H5AREAD_DOUBLE_7_C)
#define nh5areadc_c       H5_FC_FUNC_(h5areadc_c, H5AREADC_C)
#define nh5areadc_s_c       H5_FC_FUNC_(h5areadc_s_c, H5AREADC_S_C)
#define nh5areadc_1_c       H5_FC_FUNC_(h5areadc_1_c, H5AREADC_1_C)
#define nh5areadc_2_c       H5_FC_FUNC_(h5areadc_2_c, H5AREADC_2_C)
#define nh5areadc_3_c       H5_FC_FUNC_(h5areadc_3_c, H5AREADC_3_C)
#define nh5areadc_4_c       H5_FC_FUNC_(h5areadc_4_c, H5AREADC_4_C)
#define nh5areadc_5_c       H5_FC_FUNC_(h5areadc_5_c, H5AREADC_5_C)
#define nh5areadc_6_c       H5_FC_FUNC_(h5areadc_6_c, H5AREADC_6_C)
#define nh5areadc_7_c       H5_FC_FUNC_(h5areadc_7_c, H5AREADC_7_C)
#define nh5aread_f_c        H5_FC_FUNC_(h5aread_f_c, H5AREAD_F_C)
#define nh5aget_name_c    H5_FC_FUNC_(h5aget_name_c, H5AGET_NAME_C)
#define nh5aopen_idx_c    H5_FC_FUNC_(h5aopen_idx_c, H5AOPEN_IDX_C)
#define nh5aget_space_c   H5_FC_FUNC_(h5aget_space_c, H5AGET_SPACE_C)
#define nh5aget_type_c    H5_FC_FUNC_(h5aget_type_c, H5AGET_TYPE_C)
#define nh5aget_num_attrs_c H5_FC_FUNC_(h5aget_num_attrs_c, H5AGET_NUM_ATTRS_C)
#define nh5adelete_c      H5_FC_FUNC_(h5adelete_c, H5ADELETE_C)
#define nh5aget_storage_size_c H5_FC_FUNC_(h5aget_storage_size_c, H5AGET_STORAGE_SIZE_C)
#define nh5arename_by_name_c H5_FC_FUNC_(h5arename_by_name_c, H5ARENAME_BY_NAME_C)
#define nh5aopen_c H5_FC_FUNC_(h5aopen_c, H5AOPEN_C)
#define nh5adelete_by_name_c H5_FC_FUNC_(h5adelete_by_name_c,H5ADELETE_BY_NAME_C)
#define nh5adelete_by_idx_c H5_FC_FUNC_(h5adelete_by_idx_c,H5ADELETE_BY_IDX_C)
#define nh5aget_name_by_idx_c H5_FC_FUNC_(h5aget_name_by_idx_c,H5AGET_NAME_BY_IDX_C)
#define nh5aget_create_plist_c H5_FC_FUNC_(h5aget_create_plist_c,H5AGET_CREATE_PLIST_C)
#define nh5aopen_by_idx_c H5_FC_FUNC_(h5aopen_by_idx_c,H5AOPEN_BY_IDX_C)
#define nh5aget_info_c H5_FC_FUNC_(h5aget_info_c,H5AGET_INFO_C)
#define nh5aget_info_by_idx_c H5_FC_FUNC_(h5aget_info_by_idx_c,H5AGET_INFO_BY_IDX_C)
#define nh5aget_info_by_name_c H5_FC_FUNC_(h5aget_info_by_name_c,H5AGET_INFO_BY_NAME_C)
#define nh5aget_info_by_name_c H5_FC_FUNC_(h5aget_info_by_name_c,H5AGET_INFO_BY_NAME_C)
#define nh5acreate_by_name_c H5_FC_FUNC_(h5acreate_by_name_c,H5ACREATE_BY_NAME_C)
#define nh5aexists_c H5_FC_FUNC_(h5aexists_c,H5AEXISTS_C)
#define nh5aexists_by_name_c H5_FC_FUNC_(h5aexists_by_name_c,H5AEXISTS_BY_NAME_C)
#define nh5aopen_by_name_c H5_FC_FUNC_(h5aopen_by_name_c,H5AOPEN_BY_NAME_C)
#define nh5arename_c H5_FC_FUNC_(h5arename_c,H5ARENAME_C)

H5_FCDLL int_f nh5acreate_c (hid_t_f *obj_id, _fcd name, size_t_f *namelen, hid_t_f *type_id, hid_t_f *space_id, hid_t_f *crt_prp, hid_t_f *aapl,  hid_t_f *attr_id);
H5_FCDLL int_f nh5aopen_name_c (hid_t_f *obj_id, _fcd name, size_t_f *namelen, hid_t_f *attr_id);
H5_FCDLL int_f nh5awritec_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awritec_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5awrite_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_integer_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_real_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_double_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5awrite_f_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf);
H5_FCDLL int_f nh5areadc_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5areadc_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, _fcd buf, void *dims);
H5_FCDLL int_f nh5aread_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_integer_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_real_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_s_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_1_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_2_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_3_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_4_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_5_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_6_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_double_7_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf, void *dims);
H5_FCDLL int_f nh5aread_f_c (hid_t_f *attr_id, hid_t_f *mem_type_id, void *buf);
H5_FCDLL int_f nh5aclose_c ( hid_t_f *attr_id );
H5_FCDLL int_f nh5adelete_c (hid_t_f *obj_id, _fcd name, size_t_f *namelen);
H5_FCDLL int_f nh5aopen_idx_c (hid_t_f *obj_id, int_f *idx, hid_t_f *attr_id);
H5_FCDLL int_f nh5aget_space_c (hid_t_f *attr_id, hid_t_f *space_id);
H5_FCDLL int_f nh5aget_type_c (hid_t_f *attr_id, hid_t_f *type_id);
H5_FCDLL int_f nh5aget_num_attrs_c (hid_t_f *obj_id, int_f *attr_num);
H5_FCDLL int_f nh5aget_name_c(hid_t_f *attr_id, size_t_f *size, _fcd buf);
H5_FCDLL int_f nh5aget_storage_size_c ( hid_t_f *attr_id, hsize_t_f *size );
H5_FCDLL int_f nh5arename_by_name_c ( hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				      _fcd old_attr_name, size_t_f *old_attr_namelen,
				      _fcd new_attr_name, size_t_f *new_attr_namelen,
				      hid_t_f *lapl_id );
H5_FCDLL int_f nh5aopen_c ( hid_t_f *obj_id, _fcd attr_name, size_t_f *attr_namelen,
			    hid_t_f *aapl_id, hid_t_f *attr_id);
H5_FCDLL int_f nh5adelete_by_name_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				     _fcd attr_name, size_t_f *attr_namelen, hid_t_f *lapl_id);
H5_FCDLL int_f nh5adelete_by_idx_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				    int_f *idx_type, int_f *order, hsize_t_f *n, hid_t_f *lapl_id);
H5_FCDLL int_f nh5aget_name_by_idx_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				      int_f *idx_type, int_f *order, hsize_t_f *n, _fcd name,
				      size_t_f *size, hid_t_f *lapl_id);
H5_FCDLL int_f nh5aget_create_plist_c ( hid_t_f *attr_id, hid_t_f *creation_prop_id );
H5_FCDLL int_f nh5aopen_by_idx_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
		     int_f *idx_type, int_f *order, hsize_t_f *n, hid_t_f *aapl_id, hid_t_f *lapl_id, hid_t_f *attr_id);
H5_FCDLL int_f nh5aget_info_c (hid_t_f *loc_id, int_f *corder_valid, int_f *corder,
			       int_f *cset, hsize_t_f *data_size );
H5_FCDLL int_f nh5aget_info_by_idx_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				      int_f *idx_type, int_f *order, hsize_t_f *n, hid_t_f *lapl_id,
				      int_f *corder_valid, int_f *corder,
				      int_f *cset, hsize_t_f *data_size );
H5_FCDLL int_f nh5aget_info_by_name_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				       _fcd attr_name, size_t_f *attr_namelen, hid_t_f *lapl_id,
				       int_f *corder_valid, int_f *corder,
				       int_f *cset, hsize_t_f *data_size );
H5_FCDLL int_f nh5acreate_by_name_c(hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen,
				    _fcd attr_name, size_t_f *attr_namelen,  hid_t_f *type_id,
				    hid_t_f *space_id, hid_t_f *acpl_id, hid_t_f *aapl_id,
				    hid_t_f *lapl_id, hid_t_f *attr_id );
H5_FCDLL int_f nh5aexists_c (hid_t_f *obj_id, _fcd name, size_t_f *namelen, hid_t_f *attr_exists);
H5_FCDLL int_f nh5aexists_by_name_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen, _fcd attr_name, size_t_f *attr_namelen,
		      hid_t_f *lapl_id, int_f *attr_exists);
H5_FCDLL int_f nh5aopen_by_name_c (hid_t_f *loc_id, _fcd obj_name, size_t_f *obj_namelen, _fcd attr_name, size_t_f *attr_namelen,
				   hid_t_f *aapl_id, hid_t_f *lapl_id, hid_t_f *attr_id);
H5_FCDLL int_f nh5arename_c( hid_t_f *loc_id,
		      _fcd old_attr_name, size_t_f *old_attr_namelen,
		      _fcd new_attr_name, size_t_f *new_attr_namelen);

/*
 * Functions form H5Tf.c file
 */
#define nh5topen_c         H5_FC_FUNC_(h5topen_c, H5TOPEN_C)
#define nh5tcommit_c       H5_FC_FUNC_(h5tcommit_c, H5TCOMMIT_C)
#define nh5tcommitted_c    H5_FC_FUNC_(h5tcommitted_c, H5TCOMMITTED_C)
#define nh5tconvert_c      H5_FC_FUNC_(h5tconvert_c, H5TCONVERT_C)
#define nh5tclose_c        H5_FC_FUNC_(h5tclose_c, H5TCLOSE_C)
#define nh5tcopy_c         H5_FC_FUNC_(h5tcopy_c, H5TCOPY_C)
#define nh5tequal_c        H5_FC_FUNC_(h5tequal_c, H5TEQUAL_C)
#define nh5tget_class_c    H5_FC_FUNC_(h5tget_class_c, H5TGET_CLASS_C)
#define nh5tget_order_c    H5_FC_FUNC_(h5tget_order_c, H5TGET_ORDER_C)
#define nh5tset_order_c    H5_FC_FUNC_(h5tset_order_c, H5TSET_ORDER_C)
#define nh5tget_size_c     H5_FC_FUNC_(h5tget_size_c, H5TGET_SIZE_C)
#define nh5tset_size_c     H5_FC_FUNC_(h5tset_size_c, H5TSET_SIZE_C)
#define nh5tget_precision_c     H5_FC_FUNC_(h5tget_precision_c, H5TGET_PRECISION_C)
#define nh5tset_precision_c     H5_FC_FUNC_(h5tset_precision_c, H5TSET_PRECISION_C)
#define nh5tget_offset_c        H5_FC_FUNC_(h5tget_offset_c, H5TGET_OFFSET_C)
#define nh5tset_offset_c        H5_FC_FUNC_(h5tset_offset_c, H5TSET_OFFSET_C)
#define nh5tget_pad_c        H5_FC_FUNC_(h5tget_pad_c, H5TGET_PAD_C)
#define nh5tset_pad_c        H5_FC_FUNC_(h5tset_pad_c, H5TSET_PAD_C)
#define nh5tget_sign_c       H5_FC_FUNC_(h5tget_sign_c, H5TGET_SIGN_C)
#define nh5tset_sign_c       H5_FC_FUNC_(h5tset_sign_c, H5TSET_SIGN_C)
#define nh5tget_fields_c       H5_FC_FUNC_(h5tget_fields_c, H5TGET_FIELDS_C)
#define nh5tset_fields_c       H5_FC_FUNC_(h5tset_fields_c, H5TSET_FIELDS_C)
#define nh5tget_ebias_c     H5_FC_FUNC_(h5tget_ebias_c, H5TGET_EBIAS_C)
#define nh5tset_ebias_c     H5_FC_FUNC_(h5tset_ebias_c, H5TSET_EBIAS_C)
#define nh5tget_norm_c     H5_FC_FUNC_(h5tget_norm_c, H5TGET_NORM_C)
#define nh5tset_norm_c     H5_FC_FUNC_(h5tset_norm_c, H5TSET_NORM_C)
#define nh5tget_inpad_c        H5_FC_FUNC_(h5tget_inpad_c, H5TGET_INPAD_C)
#define nh5tset_inpad_c        H5_FC_FUNC_(h5tset_inpad_c, H5TSET_INPAD_C)
#define nh5tget_cset_c        H5_FC_FUNC_(h5tget_cset_c, H5TGET_CSET_C)
#define nh5tset_cset_c        H5_FC_FUNC_(h5tset_cset_c, H5TSET_CSET_C)
#define nh5tget_strpad_c        H5_FC_FUNC_(h5tget_strpad_c, H5TGET_STRPAD_C)
#define nh5tset_strpad_c        H5_FC_FUNC_(h5tset_strpad_c, H5TSET_STRPAD_C)
#define nh5tget_nmembers_c        H5_FC_FUNC_(h5tget_nmembers_c, H5TGET_NMEMBERS_C)
#define nh5tget_member_name_c        H5_FC_FUNC_(h5tget_member_name_c, H5TGET_MEMBER_NAME_C)
#define nh5tget_member_offset_c        H5_FC_FUNC_(h5tget_member_offset_c, H5TGET_MEMBER_OFFSET_C)
#define nh5tget_member_dims_c        H5_FC_FUNC_(h5tget_member_dims_c, H5TGET_MEMBER_DIMS_C)
#define nh5tget_member_type_c        H5_FC_FUNC_(h5tget_member_type_c, H5TGET_MEMBER_TYPE_C)
#define nh5tget_member_index_c        H5_FC_FUNC_(h5tget_member_index_c, H5TGET_MEMBER_INDEX_C)
#define nh5tinsert_c        H5_FC_FUNC_(h5tinsert_c, H5TINSERT_C)
#define nh5tcreate_c        H5_FC_FUNC_(h5tcreate_c, H5TCREATE_C)
#define nh5tpack_c                   H5_FC_FUNC_(h5tpack_c, H5TPACK_C)
#define nh5tinsert_array_c           H5_FC_FUNC_(h5tinsert_array_c, H5TINSERT_ARRAY_C)
#define nh5tinsert_array_c2           H5_FC_FUNC_(h5tinsert_array_c2, H5TINSERT_ARRAY_C2)
#define nh5tenum_create_c             H5_FC_FUNC_(h5tenum_create_c, H5TENUM_CREATE_C)
#define nh5tenum_insert_c             H5_FC_FUNC_(h5tenum_insert_c, H5TENUM_INSERT_C)
#define nh5tenum_nameof_c             H5_FC_FUNC_(h5tenum_nameof_c, H5TENUM_NAMEOF_C)
#define nh5tenum_valueof_c             H5_FC_FUNC_(h5tenum_valueof_c, H5TENUM_VALUEOF_C)
#define nh5tget_member_value_c         H5_FC_FUNC_(h5tget_member_value_c, H5TGET_MEMBER_VALUE_C)
#define nh5tset_tag_c                  H5_FC_FUNC_(h5tset_tag_c, H5TSET_TAG_C)
#define nh5tget_tag_c                  H5_FC_FUNC_(h5tget_tag_c, H5TGET_TAG_C)
#define nh5tarray_create_c             H5_FC_FUNC_(h5tarray_create_c, H5TARRAY_CREATE_C)
#define nh5tget_array_ndims_c          H5_FC_FUNC_(h5tget_array_ndims_c, H5TGET_ARRAY_NDIMS_C)
#define nh5tget_array_dims_c          H5_FC_FUNC_(h5tget_array_dims_c, H5TGET_ARRAY_DIMS_C)
#define nh5tget_super_c               H5_FC_FUNC_(h5tget_super_c, H5TGET_SUPER_C)
#define nh5tvlen_create_c               H5_FC_FUNC_(h5tvlen_create_c, H5TVLEN_CREATE_C)
#define nh5tis_variable_str_c         H5_FC_FUNC_(h5tis_variable_str_c, H5TIS_VARIABLE_STR_C)
#define nh5tget_member_class_c         H5_FC_FUNC_(h5tget_member_class_c, H5TGET_MEMBER_CLASS_C)
#define nh5tcommit_anon_c       H5_FC_FUNC_(h5tcommit_anon_c, H5TCOMMIT_ANON_C)
#define nh5tdecode_c    H5_FC_FUNC_(h5tdecode_c, H5TDECODE_C)
#define nh5tencode_c    H5_FC_FUNC_(h5tencode_c, H5TENCODE_C)
#define nh5tget_create_plist_c H5_FC_FUNC_(h5tget_create_plist_c, H5TGET_CREATE_PLIST_C)
#define nh5tcompiler_conv_c H5_FC_FUNC_(h5tcompiler_conv_c, H5TCOMPILER_CONV_C)
#define nh5tget_native_type_c H5_FC_FUNC_(h5tget_native_type_c, H5TGET_NATIVE_TYPE_C )

H5_FCDLL int_f nh5tcreate_c(int_f *cls, size_t_f *size, hid_t_f *type_id);
H5_FCDLL int_f nh5topen_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *type_id, hid_t_f *tapl_id );
H5_FCDLL int_f nh5tcommit_c (hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *type_id, hid_t_f *lcpl_id, hid_t_f *tcpl_id, hid_t_f *tapl_id);
H5_FCDLL int_f nh5tclose_c ( hid_t_f *type_id );
H5_FCDLL int_f nh5tequal_c ( hid_t_f *type1_id , hid_t_f *type2_id, int_f *c_flag);
H5_FCDLL int_f nh5tcopy_c ( hid_t_f *type_id , hid_t_f *new_type_id);
H5_FCDLL int_f nh5tget_class_c ( hid_t_f *type_id , int_f *classtype);
H5_FCDLL int_f nh5tget_order_c ( hid_t_f *type_id , int_f *order);
H5_FCDLL int_f nh5tset_order_c ( hid_t_f *type_id , int_f *order);
H5_FCDLL int_f nh5tget_size_c ( hid_t_f *type_id , size_t_f *size);
H5_FCDLL int_f nh5tset_size_c ( hid_t_f *type_id , size_t_f *size);
H5_FCDLL int_f nh5tcommitted_c (hid_t_f *dtype_id);
H5_FCDLL int_f nh5tget_precision_c ( hid_t_f *type_id , size_t_f *precision);
H5_FCDLL int_f nh5tset_precision_c ( hid_t_f *type_id , size_t_f *precision);
H5_FCDLL int_f nh5tget_offset_c ( hid_t_f *type_id , size_t_f *offset);
H5_FCDLL int_f nh5tset_offset_c ( hid_t_f *type_id , size_t_f *offset);
H5_FCDLL int_f nh5tget_pad_c ( hid_t_f *type_id , int_f * lsbpad, int_f * msbpad);
H5_FCDLL int_f nh5tset_pad_c ( hid_t_f *type_id, int_f * lsbpad, int_f * msbpad );
H5_FCDLL int_f nh5tget_sign_c ( hid_t_f *type_id , int_f* sign);
H5_FCDLL int_f nh5tset_sign_c ( hid_t_f *type_id , int_f *sign);
H5_FCDLL int_f nh5tget_fields_c ( hid_t_f *type_id, size_t_f *spos, size_t_f *epos, size_t_f* esize, size_t_f* mpos, size_t_f* msize);
H5_FCDLL int_f nh5tset_fields_c ( hid_t_f *type_id, size_t_f *spos, size_t_f *epos, size_t_f* esize, size_t_f* mpos, size_t_f* msize);
H5_FCDLL int_f nh5tget_ebias_c ( hid_t_f *type_id , size_t_f *ebias);
H5_FCDLL int_f nh5tset_ebias_c ( hid_t_f *type_id , size_t_f *ebias);
H5_FCDLL int_f nh5tget_norm_c ( hid_t_f *type_id , int_f *norm);
H5_FCDLL int_f nh5tset_norm_c ( hid_t_f *type_id , int_f *norm);
H5_FCDLL int_f nh5tget_inpad_c ( hid_t_f *type_id, int_f * padtype);
H5_FCDLL int_f nh5tset_inpad_c ( hid_t_f *type_id, int_f * padtype);
H5_FCDLL int_f nh5tget_cset_c ( hid_t_f *type_id, int_f * cset);
H5_FCDLL int_f nh5tset_cset_c ( hid_t_f *type_id, int_f * cset);
H5_FCDLL int_f nh5tget_strpad_c ( hid_t_f *type_id, int_f * strpad);
H5_FCDLL int_f nh5tset_strpad_c ( hid_t_f *type_id, int_f * strpad);
H5_FCDLL int_f nh5tget_nmembers_c ( hid_t_f *type_id , int_f * num_members);
H5_FCDLL int_f nh5tget_member_name_c ( hid_t_f *type_id ,int_f* idx, _fcd member_name, int_f *namelen);
H5_FCDLL int_f nh5tget_member_dims_c ( hid_t_f *type_id ,int_f* field_idx, int_f * dims, size_t_f * field_dims, int_f * perm );
H5_FCDLL int_f nh5tget_member_offset_c ( hid_t_f *type_id ,int_f* member_no, size_t_f* offset);
H5_FCDLL int_f nh5tget_member_type_c ( hid_t_f *type_id ,int_f* field_idx, hid_t_f * datatype);
H5_FCDLL int_f nh5tget_member_index_c ( hid_t_f *type_id ,_fcd name, int_f* namelen, int_f *idx);
H5_FCDLL int_f nh5tinsert_c(hid_t_f *type_id, _fcd name, int_f* namelen, size_t_f *offset, hid_t_f * field_id);
H5_FCDLL int_f nh5tpack_c(hid_t_f * type_id);
H5_FCDLL int_f nh5tinsert_array_c(hid_t_f * parent_id, _fcd name, int_f* namelen, size_t_f* offset, int_f* ndims, size_t_f* dims, hid_t_f* member_id, int_f* perm );
H5_FCDLL int_f nh5tinsert_array_c2(hid_t_f * parent_id, _fcd name, int_f* namelen, size_t_f* offset, int_f* ndims, size_t_f* dims, hid_t_f* member_id);
H5_FCDLL int_f nh5tenum_create_c ( hid_t_f *parent_id , hid_t_f *new_type_id);
H5_FCDLL int_f nh5tenum_insert_c(hid_t_f *type_id, _fcd name, int_f* namelen, int_f* value);
H5_FCDLL int_f nh5tenum_nameof_c(hid_t_f *type_id, int_f* value, _fcd name, size_t_f* namelen);
H5_FCDLL int_f nh5tenum_valueof_c(hid_t_f *type_id, _fcd name, int_f* namelen, int_f* value);
H5_FCDLL int_f nh5tget_member_value_c(hid_t_f *type_id, int_f* member_no, int_f* value);
H5_FCDLL int_f nh5tset_tag_c(hid_t_f* type_id, _fcd tag, int_f* namelen);
H5_FCDLL int_f nh5tget_tag_c(hid_t_f* type_id, _fcd tag, size_t_f* tag_size, int_f* namelen);
H5_FCDLL int_f nh5tarray_create_c(hid_t_f * base_id, int_f *rank, hsize_t_f* dims, hid_t_f* type_id);
H5_FCDLL int_f nh5tget_array_dims_c ( hid_t_f *type_id , hsize_t_f * dims);
H5_FCDLL int_f nh5tget_array_ndims_c ( hid_t_f *type_id , int_f * ndims);
H5_FCDLL int_f nh5tget_super_c ( hid_t_f *type_id , hid_t_f *base_type_id);
H5_FCDLL int_f nh5tvlen_create_c ( hid_t_f *type_id , hid_t_f *vltype_id);
H5_FCDLL int_f nh5tis_variable_str_c ( hid_t_f *type_id , int_f *flag );
H5_FCDLL int_f nh5tget_member_class_c ( hid_t_f *type_id ,  int_f *member_no, int_f *cls );
H5_FCDLL int_f nh5tcommit_anon_c(hid_t_f *loc_id, hid_t_f *dtype_id, hid_t_f *tcpl_id, hid_t_f *tapl_id);
H5_FCDLL int_f nh5tdecode_c ( _fcd buf, hid_t_f *obj_id );
H5_FCDLL int_f nh5tencode_c (_fcd buf, hid_t_f *obj_id, size_t_f *nalloc );
H5_FCDLL int_f nh5tget_create_plist_c ( hid_t_f *dtype_id,  hid_t_f *dtpl_id);
H5_FCDLL int_f nh5tcompiler_conv_c ( hid_t_f *src_id, hid_t_f *dst_id, int_f *c_flag);
H5_FCDLL int_f nh5tget_native_type_c(hid_t_f *dtype_id, int_f *direction, hid_t_f *native_dtype_id);
H5_FCDLL int_f nh5tconvert_c(hid_t_f *src_id, hid_t_f *dst_id, size_t_f *nelmts, void *buf, void *background, hid_t_f *plist_id);


/*
 * Functions from H5Of.c
 */

#define nh5olink_c       H5_FC_FUNC_(h5olink_c, H5OLINK_C)
#define nh5oopen_c       H5_FC_FUNC_(h5oopen_c, H5OOPEN_C)
#define nh5oclose_c      H5_FC_FUNC_(h5oclose_c, H5OCLOSE_C)
#define nh5ovisit_c      H5_FC_FUNC_(h5ovisit_c, H5OVISIT_C)
#define nh5ovisit_by_name_c H5_FC_FUNC_(h5ovisit_by_name_c, H5OVISIT_BY_NAME_C)
#define nh5oget_info_c   H5_FC_FUNC_(h5oget_info_c, H5OGET_INFO_C)
#define nh5oget_info_by_idx_c H5_FC_FUNC_(h5oget_info_by_idx_c ,H5OGET_INFO_BY_IDX_C)
#define nh5oget_info_by_name_c H5_FC_FUNC_(h5oget_info_by_name_c ,H5OGET_INFO_BY_NAME_C)
#define nh5oopen_by_addr_c H5_FC_FUNC_(h5oopen_by_addr_c, H5OOPEN_BY_ADDR_C)
#define nh5ocopy_c         H5_FC_FUNC_(h5ocopy_c, H5OCOPY_C)
#define nh5odecr_refcount_c H5_FC_FUNC_(h5odecr_refcount_c, H5ODECR_REFCOUNT_C)
#define nh5oincr_refcount_c H5_FC_FUNC_(h5oincr_refcount_c, H5OINCR_REFCOUNT_C)
#define nh5oexists_by_name_c H5_FC_FUNC_(h5oexists_by_name_c, H5OEXISTS_BY_NAME_C)
#define nh5oset_comment_c H5_FC_FUNC_(h5oset_comment_c, H5OSET_COMMENT_C)
#define nh5oset_comment_by_name_c H5_FC_FUNC_(h5oset_comment_by_name_c, H5OSET_COMMENT_BY_NAME_C)
#define nh5oopen_by_idx_c H5_FC_FUNC_(h5oopen_by_idx_c, H5OOPEN_BY_IDX_C)
#define nh5oget_comment_c H5_FC_FUNC_(h5oget_comment_c, H5OGET_COMMENT_C)
#define nh5oget_comment_by_name_c H5_FC_FUNC_(h5oget_comment_by_name_c, H5OGET_COMMENT_BY_NAME_C)

H5_FCDLL int_f nh5oopen_c (hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id, hid_t_f *obj_id);
H5_FCDLL int_f nh5oclose_c (hid_t_f *object_id );
H5_FCDLL int_f nh5oopen_by_addr_c (hid_t_f *loc_id, haddr_t_f *addr, hid_t_f *obj_id);
H5_FCDLL int_f nh5olink_c (hid_t_f *object_id, hid_t_f *new_loc_id, _fcd name, size_t_f *namelen,
			   hid_t_f *lcpl_id, hid_t_f *lapl_id);
H5_FCDLL int_f nh5ovisit_c (hid_t_f *group_id, int_f *index_type, int_f *order, H5O_iterate_t op, void *op_data);
H5_FCDLL int_f nh5ovisit_by_name_c(hid_t_f *loc_id,  _fcd object_name, size_t_f *namelen, int_f *index_type, int_f *order,
				   H5O_iterate_t op, void *op_data, hid_t_f *lapl_id );
H5_FCDLL int_f nh5oget_info_c (hid_t_f *object_id, H5O_info_t_f *object_info);
H5_FCDLL int_f nh5oget_info_by_idx_c (hid_t_f *loc_id, _fcd  group_name, size_t_f *namelen, 
				      int_f *index_field, int_f *order, hsize_t_f *n, hid_t_f *lapl_id, H5O_info_t_f *object_info);
H5_FCDLL int_f nh5oget_info_by_name_c (hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id,
				       H5O_info_t_f *object_info);
H5_FCDLL int_f nh5ocopy_c (hid_t_f *src_loc_id, _fcd src_name, size_t_f *src_name_len,
			   hid_t_f *dst_loc_id, _fcd dst_name, size_t_f *dst_name_len, 
			   hid_t_f *ocpypl_id, hid_t_f *lcpl_id );
H5_FCDLL int_f nh5odecr_refcount_c (hid_t_f *object_id);
H5_FCDLL int_f nh5oincr_refcount_c (hid_t_f *object_id);
H5_FCDLL int_f nh5oexists_by_name_c (hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id);
H5_FCDLL int_f nh5oset_comment_c (hid_t_f *object_id, _fcd comment, size_t_f *commentlen);
H5_FCDLL int_f nh5oset_comment_by_name_c (hid_t_f *object_id, _fcd name, size_t_f *namelen,  _fcd comment, size_t_f *commentlen, hid_t_f *lapl_id);
H5_FCDLL int_f nh5oopen_by_idx_c (hid_t_f *loc_id, _fcd  group_name, size_t_f *group_namelen, 
				      int_f *index_type, int_f *order, hsize_t_f *n, hid_t_f *obj_id, hid_t_f *lapl_id);
H5_FCDLL int_f nh5oget_comment_c (hid_t_f *object_id, _fcd comment, size_t_f *commentsize, hssize_t_f *bufsize);
H5_FCDLL int_f nh5oget_comment_by_name_c (hid_t_f *loc_id, _fcd name, size_t_f *name_size, 
					  _fcd comment, size_t_f *commentsize, size_t_f *bufsize, hid_t_f *lapl_id);
/*
 * Functions from H5Pf.c
 */
#define nh5pcreate_c       H5_FC_FUNC_(h5pcreate_c, H5PCREATE_C)
#define nh5pclose_c        H5_FC_FUNC_(h5pclose_c, H5PCLOSE_C)
#define nh5pcopy_c         H5_FC_FUNC_(h5pcopy_c, H5PCOPY_C)
#define nh5pequal_c         H5_FC_FUNC_(h5pequal_c, H5PEQUAL_C)
#define nh5pget_class_c    H5_FC_FUNC_(h5pget_class_c, H5PGET_CLASS_C)
#define nh5pset_deflate_c  H5_FC_FUNC_(h5pset_deflate_c, H5PSET_DEFLATE_C)
#define nh5pset_preserve_c  H5_FC_FUNC_(h5pset_preserve_c, H5PSET_PRESERVE_C)
#define nh5pget_preserve_c  H5_FC_FUNC_(h5pget_preserve_c, H5PGET_PRESERVE_C)
#define nh5pset_chunk_c    H5_FC_FUNC_(h5pset_chunk_c, H5PSET_CHUNK_C)
#define nh5pget_chunk_c    H5_FC_FUNC_(h5pget_chunk_c, H5PGET_CHUNK_C)
#define nh5pset_fill_valuec_c        H5_FC_FUNC_(h5pset_fill_valuec_c, H5PSET_FILL_VALUEC_C)
#define nh5pset_fill_value_c         H5_FC_FUNC_(h5pset_fill_value_c, H5PSET_FILL_VALUE_C)
#define nh5pset_fill_value_integer_c H5_FC_FUNC_(h5pset_fill_value_integer_c, H5PSET_FILL_VALUE_INTEGER_C)
#define nh5pset_fill_value_real_c    H5_FC_FUNC_(h5pset_fill_value_real_c, H5PSET_FILL_VALUE_REAL_C)
#define nh5pset_fill_value_double_c  H5_FC_FUNC_(h5pset_fill_value_double_c, H5PSET_FILL_VALUE_DOUBLE_C)
#define nh5pget_fill_valuec_c        H5_FC_FUNC_(h5pget_fill_valuec_c, H5PGET_FILL_VALUEC_C)
#define nh5pget_fill_value_c         H5_FC_FUNC_(h5pget_fill_value_c, H5PGET_FILL_VALUE_C)
#define nh5pget_fill_value_integer_c H5_FC_FUNC_(h5pget_fill_value_integer_c, H5PGET_FILL_VALUE_INTEGER_C)
#define nh5pget_fill_value_real_c    H5_FC_FUNC_(h5pget_fill_value_real_c, H5PGET_FILL_VALUE_REAL_C)
#define nh5pget_fill_value_double_c  H5_FC_FUNC_(h5pget_fill_value_double_c, H5PGET_FILL_VALUE_DOUBLE_C)
#define nh5pget_version_c    H5_FC_FUNC_(h5pget_version_c, H5PGET_VERSION_C)
#define nh5pget_userblock_c    H5_FC_FUNC_(h5pget_userblock_c, H5PGET_USERBLOCK_C)
#define nh5pset_userblock_c    H5_FC_FUNC_(h5pset_userblock_c, H5PSET_USERBLOCK_C)
#define nh5pset_sizes_c        H5_FC_FUNC_(h5pset_sizes_c, H5PSET_SIZES_C)
#define nh5pget_sizes_c         H5_FC_FUNC_(h5pget_sizes_c, H5PGET_SIZES_C)
#define nh5pget_sym_k_c         H5_FC_FUNC_(h5pget_sym_k_c, H5PGET_SYM_K_C)
#define nh5pset_sym_k_c         H5_FC_FUNC_(h5pset_sym_k_c, H5PSET_SYM_K_C)
#define nh5pget_istore_k_c         H5_FC_FUNC_(h5pget_istore_k_c, H5PGET_ISTORE_K_C)
#define nh5pset_istore_k_c         H5_FC_FUNC_(h5pset_istore_k_c, H5PSET_ISTORE_K_C)
#define nh5pget_driver_c         H5_FC_FUNC_(h5pget_driver_c, H5PGET_DRIVER_C)
#define nh5pset_fapl_stdio_c         H5_FC_FUNC_(h5pset_fapl_stdio_c, H5PSET_FAPL_STDIO_C)
#define nh5pget_fapl_stdio_c         H5_FC_FUNC_(h5pget_fapl_stdio_c, H5PGET_FAPL_STDIO_C)
#define nh5pset_fapl_sec2_c         H5_FC_FUNC_(h5pset_fapl_sec2_c, H5PSET_FAPL_SEC2_C)
#define nh5pget_fapl_sec2_c         H5_FC_FUNC_(h5pget_fapl_sec2_c, H5PGET_FAPL_SEC2_C)
#define nh5pset_alignment_c         H5_FC_FUNC_(h5pset_alignment_c, H5PSET_ALIGNMENT_C)
#define nh5pget_alignment_c         H5_FC_FUNC_(h5pget_alignment_c, H5PGET_ALIGNMENT_C)
#define nh5pset_fapl_core_c         H5_FC_FUNC_(h5pset_fapl_core_c, H5PSET_FAPL_CORE_C)
#define nh5pget_fapl_core_c         H5_FC_FUNC_(h5pget_fapl_core_c, H5PGET_FAPL_CORE_C)
#define nh5pset_fapl_family_c         H5_FC_FUNC_(h5pset_fapl_family_c, H5PSET_FAPL_FAMILY_C)
#define nh5pget_fapl_family_c         H5_FC_FUNC_(h5pget_fapl_family_c, H5PGET_FAPL_FAMILY_C)
#define nh5pset_cache_c         H5_FC_FUNC_(h5pset_cache_c, H5PSET_CACHE_C)
#define nh5pget_cache_c         H5_FC_FUNC_(h5pget_cache_c, H5PGET_CACHE_C)
#define nh5pset_fapl_split_c         H5_FC_FUNC_(h5pset_fapl_split_c, H5PSET_FAPL_SPLIT_C)
#define nh5pget_fapl_split_c         H5_FC_FUNC_(h5pget_fapl_split_c, H5PGET_FAPL_SPLIT_C)
#define nh5pset_gc_references_c         H5_FC_FUNC_(h5pset_gc_references_c, H5PSET_GC_REFERENCES_C)
#define nh5pget_gc_references_c         H5_FC_FUNC_(h5pget_gc_references_c, H5PGET_GC_REFERENCES_C)
#define nh5pset_layout_c         H5_FC_FUNC_(h5pset_layout_c, H5PSET_LAYOUT_C)
#define nh5pget_layout_c         H5_FC_FUNC_(h5pget_layout_c, H5PGET_LAYOUT_C)
#define nh5pset_filter_c         H5_FC_FUNC_(h5pset_filter_c, H5PSET_FILTER_C)
#define nh5premove_filter_c         H5_FC_FUNC_(h5premove_filter_c, H5PREMOVE_FILTER_C)
#define nh5pmodify_filter_c         H5_FC_FUNC_(h5pmodify_filter_c, H5PMODIFY_FILTER_C)
#define nh5pget_nfilters_c         H5_FC_FUNC_(h5pget_nfilters_c, H5PGET_NFILTERS_C)
#define nh5pget_filter_c         H5_FC_FUNC_(h5pget_filter_c, H5PGET_FILTER_C)
#define nh5pget_filter_by_id_c         H5_FC_FUNC_(h5pget_filter_by_id_c, H5PGET_FILTER_BY_ID_C)
#define nh5pset_external_c         H5_FC_FUNC_(h5pset_external_c, H5PSET_EXTERNAL_C)
#define nh5pget_external_count_c         H5_FC_FUNC_(h5pget_external_count_c, H5PGET_EXTERNAL_COUNT_C)
#define nh5pget_external_c         H5_FC_FUNC_(h5pget_external_c, H5PGET_EXTERNAL_C)
#define nh5pget_btree_ratios_c         H5_FC_FUNC_(h5pget_btree_ratios_c, H5PGET_BTREE_RATIOS_C)
#define nh5pset_btree_ratios_c         H5_FC_FUNC_(h5pset_btree_ratios_c, H5PSET_BTREE_RATIOS_C)
#define nh5pset_fapl_mpio_c         H5_FC_FUNC_(h5pset_fapl_mpio_c, H5PSET_FAPL_MPIO_C)
#define nh5pget_fapl_mpio_c         H5_FC_FUNC_(h5pget_fapl_mpio_c, H5PGET_FAPL_MPIO_C)
#define nh5pset_fapl_mpiposix_c     H5_FC_FUNC_(h5pset_fapl_mpiposix_c, H5PSET_FAPL_MPIPOSIX_C)
#define nh5pget_fapl_mpiposix_c     H5_FC_FUNC_(h5pget_fapl_mpiposix_c, H5PGET_FAPL_MPIPOSIX_C)
#define nh5pset_dxpl_mpio_c        H5_FC_FUNC_(h5pset_dxpl_mpio_c, H5PSET_DXPL_MPIO_C)
#define nh5pget_dxpl_mpio_c        H5_FC_FUNC_(h5pget_dxpl_mpio_c, H5PGET_DXPL_MPIO_C)
#define nh5pget_fclose_degree_c    H5_FC_FUNC_(h5pget_fclose_degree_c, H5PGET_FCLOSE_DEGREE_C)
#define nh5pset_fclose_degree_c    H5_FC_FUNC_(h5pset_fclose_degree_c, H5PSET_FCLOSE_DEGREE_C)
#define nh5pset_buffer_c    H5_FC_FUNC_(h5pset_buffer_c, H5PSET_BUFFER_C)
#define nh5pget_buffer_c    H5_FC_FUNC_(h5pget_buffer_c, H5PGET_BUFFER_C)
#define nh5pfill_value_defined_c    H5_FC_FUNC_(h5pfill_value_defined_c, H5PFILL_VALUE_DEFINED_C)
#define nh5pset_alloc_time_c    H5_FC_FUNC_(h5pset_alloc_time_c, H5PSET_ALLOC_TIME_C)
#define nh5pget_alloc_time_c    H5_FC_FUNC_(h5pget_alloc_time_c, H5PGET_ALLOC_TIME_C)
#define nh5pset_fill_time_c    H5_FC_FUNC_(h5pset_fill_time_c, H5PSET_FILL_TIME_C)
#define nh5pget_fill_time_c    H5_FC_FUNC_(h5pget_fill_time_c, H5PGET_FILL_TIME_C)
#define nh5pset_meta_block_size_c    H5_FC_FUNC_(h5pset_meta_block_size_c, H5PSET_META_BLOCK_SIZE_C)
#define nh5pget_meta_block_size_c    H5_FC_FUNC_(h5pget_meta_block_size_c, H5PGET_META_BLOCK_SIZE_C)
#define nh5pset_sieve_buf_size_c    H5_FC_FUNC_(h5pset_sieve_buf_size_c, H5PSET_SIEVE_BUF_SIZE_C)
#define nh5pget_sieve_buf_size_c    H5_FC_FUNC_(h5pget_sieve_buf_size_c, H5PGET_SIEVE_BUF_SIZE_C)
#define nh5pset_hyper_vector_size_c    H5_FC_FUNC_(h5pset_hyper_vector_size_c, H5PSET_HYPER_VECTOR_SIZE_C)
#define nh5pget_hyper_vector_size_c    H5_FC_FUNC_(h5pget_hyper_vector_size_c, H5PGET_HYPER_VECTOR_SIZE_C)
#define nh5pset_small_data_block_size_c    H5_FC_FUNC_(h5pset_small_data_block_size_c, H5PSET_SMALL_DATA_BLOCK_SIZE_C)
#define nh5pget_small_data_block_size_c    H5_FC_FUNC_(h5pget_small_data_block_size_c, H5PGET_SMALL_DATA_BLOCK_SIZE_C)
#define nh5pcreate_class_c             H5_FC_FUNC_(h5pcreate_class_c, H5PCREATE_CLASS_C)
#define nh5pregister_c                 H5_FC_FUNC_(h5pregister_c, H5PREGISTER_C)
#define nh5pregister_integer_c                 H5_FC_FUNC_(h5pregister_integer_c, H5PREGISTER_INTEGER_C)
#define nh5pregister_real_c                    H5_FC_FUNC_(h5pregister_real_c, H5PREGISTER_REAL_C)
#define nh5pregister_double_c                  H5_FC_FUNC_(h5pregister_double_c, H5PREGISTER_DOUBLE_C)
#define nh5pregisterc_c                H5_FC_FUNC_(h5pregisterc_c, H5PREGISTERC_C)
#define nh5pinsert_c                   H5_FC_FUNC_(h5pinsert_c, H5PINSERT_C)
#define nh5pinsert_integer_c                   H5_FC_FUNC_(h5pinsert_integer_c, H5PINSERT_INTEGER_C)
#define nh5pinsert_real_c                   H5_FC_FUNC_(h5pinsert_real_c, H5PINSERT_REAL_C)
#define nh5pinsert_double_c                   H5_FC_FUNC_(h5pinsert_double_c, H5PINSERT_DOUBLE_C)
#define nh5pinsertc_c                  H5_FC_FUNC_(h5pinsertc_c, H5PINSERTC_C)
#define nh5pset_c                      H5_FC_FUNC_(h5pset_c, H5PSET_C)
#define nh5pset_integer_c                      H5_FC_FUNC_(h5pset_integer_c, H5PSET_INTEGER_C)
#define nh5pset_real_c                         H5_FC_FUNC_(h5pset_real_c, H5PSET_REAL_C)
#define nh5pset_double_c                       H5_FC_FUNC_(h5pset_double_c, H5PSET_DOUBLE_C)
#define nh5psetc_c                     H5_FC_FUNC_(h5psetc_c, H5PSETC_C)
#define nh5pget_c                      H5_FC_FUNC_(h5pget_c, H5PGET_C)
#define nh5pget_integer_c                      H5_FC_FUNC_(h5pget_integer_c, H5PGET_INTEGER_C)
#define nh5pget_real_c                         H5_FC_FUNC_(h5pget_real_c, H5PGET_REAL_C)
#define nh5pget_double_c                       H5_FC_FUNC_(h5pget_double_c, H5PGET_DOUBLE_C)
#define nh5pgetc_c                     H5_FC_FUNC_(h5pgetc_c, H5PGETC_C)
#define nh5pexist_c                    H5_FC_FUNC_(h5pexist_c, H5PEXIST_C)
#define nh5pget_size_c                 H5_FC_FUNC_(h5pget_size_c, H5PGET_SIZE_C)
#define nh5pget_nprops_c               H5_FC_FUNC_(h5pget_nprops_c, H5PGET_NPROPS_C)
#define nh5pget_class_parent_c         H5_FC_FUNC_(h5pget_class_parent_c, H5PGET_CLASS_PARENT_C)
#define nh5pisa_class_c                H5_FC_FUNC_(h5pisa_class_c, H5PISA_CLASS_C)
#define nh5pcopy_prop_c                H5_FC_FUNC_(h5pcopy_prop_c, H5PCOPY_PROP_C)
#define nh5premove_c                   H5_FC_FUNC_(h5premove_c, H5PREMOVE_C)
#define nh5punregister_c               H5_FC_FUNC_(h5punregister_c, H5PUNREGISTER_C)
#define nh5pclose_class_c              H5_FC_FUNC_(h5pclose_class_c, H5PCLOSE_CLASS_C)
#define nh5pget_class_name_c           H5_FC_FUNC_(h5pget_class_name_c, H5PGET_CLASS_NAME_C)
#define nh5pset_shuffle_c               H5_FC_FUNC_(h5pset_shuffle_c, H5PSET_SHUFFLE_C)
#define nh5pset_fletcher32_c           H5_FC_FUNC_(h5pset_fletcher32_c, H5PSET_FLETCHER32_C)
#define nh5pset_edc_check_c            H5_FC_FUNC_(h5pset_edc_check_c, H5PSET_EDC_CHECK_C)
#define nh5pget_edc_check_c            H5_FC_FUNC_(h5pget_edc_check_c, H5PGET_EDC_CHECK_C)
#define nh5pset_family_offset_c       H5_FC_FUNC_(h5pset_family_offset_c, H5PSET_FAMILY_OFFSET_C)
#define nh5pget_fapl_multi_c          H5_FC_FUNC_(h5pget_fapl_multi_c, H5PGET_FAPL_MULTI_C)
#define nh5pset_fapl_multi_c          H5_FC_FUNC_(h5pset_fapl_multi_c, H5PSET_FAPL_MULTI_C)
#define nh5pset_fapl_multi_sc          H5_FC_FUNC_(h5pset_fapl_multi_sc, H5PSET_FAPL_MULTI_SC)
#define nh5pset_szip_c                 H5_FC_FUNC_(h5pset_szip_c, H5PSET_SZIP_C)
#define nh5pall_filters_avail_c        H5_FC_FUNC_(h5pall_filters_avail_c, H5PALL_FILTERS_AVAIL_C)
#define nh5pget_attr_phase_change_c    H5_FC_FUNC_(h5pget_attr_phase_change_c, H5PGET_ATTR_PHASE_CHANGE_C)
#define nh5pset_attr_creation_order_c  H5_FC_FUNC_(h5pset_attr_creation_order_c, H5PSET_ATTR_CREATION_ORDER_C)
#define nh5pset_shared_mesg_nindexes_c  H5_FC_FUNC_(h5pset_shared_mesg_nindexes_c, H5PSET_SHARED_MESG_NINDEXES_C)
#define nh5pset_shared_mesg_index_c  H5_FC_FUNC_(h5pset_shared_mesg_index_c,H5PSET_SHARED_MESG_INDEX_C)
#define nh5pget_attr_creation_order_c  H5_FC_FUNC_(h5pget_attr_creation_order_c,H5PGET_ATTR_CREATION_ORDER_C)
#define nh5pset_libver_bounds_c H5_FC_FUNC_(h5pset_libver_bounds_c,H5PSET_LIBVER_BOUNDS_C)
#define nh5pset_link_creation_order_c H5_FC_FUNC_(h5pset_link_creation_order_c, H5PSET_LINK_CREATION_ORDER_C)
#define nh5pget_link_phase_change_c H5_FC_FUNC_(h5pget_link_phase_change_c, H5PGET_LINK_PHASE_CHANGE_C)
#define nh5pget_obj_track_times_c H5_FC_FUNC_(h5pget_obj_track_times_c, H5PGET_OBJ_TRACK_TIMES_C)
#define nh5pset_obj_track_times_c H5_FC_FUNC_(h5pset_obj_track_times_c, H5PSET_OBJ_TRACK_TIMES_C)
#define nh5pset_create_inter_group_c H5_FC_FUNC_(h5pset_create_inter_group_c,H5PSET_CREATE_INTER_GROUP_C)
#define nh5pget_create_inter_group_c H5_FC_FUNC_(h5pget_create_inter_group_c,H5PGET_CREATE_INTER_GROUP_C)
#define nh5pget_link_creation_order_c H5_FC_FUNC_(h5pget_link_creation_order_c,H5PGET_LINK_CREATION_ORDER_C)
#define nh5pset_char_encoding_c H5_FC_FUNC_(h5pset_char_encoding_c, H5PSET_CHAR_ENCODING_C)
#define nh5pget_char_encoding_c H5_FC_FUNC_(h5pget_char_encoding_c, H5PGET_CHAR_ENCODING_C)
#define nh5pset_copy_object_c H5_FC_FUNC_(h5pset_copy_object_c, H5PSET_COPY_OBJECT_C)
#define nh5pget_copy_object_c H5_FC_FUNC_(h5pget_copy_object_c, H5PGET_COPY_OBJECT_C)
#define nh5pget_data_transform_c H5_FC_FUNC_(h5pget_data_transform_c, H5PGET_DATA_TRANSFORM_C)
#define nh5pset_data_transform_c H5_FC_FUNC_(h5pset_data_transform_c, H5PSET_DATA_TRANSFORM_C)
#define nh5pget_local_heap_size_hint_c H5_FC_FUNC_(h5pget_local_heap_size_hint_c, H5PGET_LOCAL_HEAP_SIZE_HINT_C)
#define nh5pget_est_link_info_c H5_FC_FUNC_(h5pget_est_link_info_c,H5PGET_EST_LINK_INFO_C)
#define nh5pset_est_link_info_c H5_FC_FUNC_(h5pset_est_link_info_c,H5PSET_EST_LINK_INFO_C)
#define nh5pset_local_heap_size_hint_c H5_FC_FUNC_(h5pset_local_heap_size_hint_c, H5PSET_LOCAL_HEAP_SIZE_HINT_C)
#define nh5pset_link_phase_change_c H5_FC_FUNC_(h5pset_link_phase_change_c, H5PSET_LINK_PHASE_CHANGE_C)
#define nh5pset_fapl_direct_c H5_FC_FUNC_(h5pset_fapl_direct_c, H5PSET_FAPL_DIRECT_C)
#define nh5pget_fapl_direct_c H5_FC_FUNC_(h5pget_fapl_direct_c, H5PGET_FAPL_DIRECT_C)
#define nh5pset_attr_phase_change_c H5_FC_FUNC_(h5pset_attr_phase_change_c, H5PSET_ATTR_PHASE_CHANGE_C)
#define nh5pset_nbit_c H5_FC_FUNC_(h5pset_nbit_c, H5PSET_NBIT_C)
#define nh5pset_scaleoffset_c H5_FC_FUNC_(h5pset_scaleoffset_c, H5PSET_SCALEOFFSET_C)
#define nh5pset_nlinks_c H5_FC_FUNC_(h5pset_nlinks_c, H5PSET_NLINKS_C)
#define nh5pget_nlinks_c H5_FC_FUNC_(h5pget_nlinks_c, H5PGET_NLINKS_C)
#define nh5pset_chunk_cache_c H5_FC_FUNC_(h5pset_chunk_cache_c, H5PSET_CHUNK_CACHE_C)
#define nh5pget_chunk_cache_c H5_FC_FUNC_(h5pget_chunk_cache_c, H5PGET_CHUNK_CACHE_C)
#define nh5pget_mpio_actual_io_mode_c H5_FC_FUNC_(h5pget_mpio_actual_io_mode_c, H5PGET_MPIO_ACTUAL_IO_MODE_C)

H5_FCDLL int_f nh5pcreate_c ( hid_t_f *cls, hid_t_f *prp_id );
H5_FCDLL int_f nh5pclose_c ( hid_t_f *prp_id );
H5_FCDLL int_f nh5pcopy_c ( hid_t_f *prp_id , hid_t_f *new_prp_id);
H5_FCDLL int_f nh5pequal_c ( hid_t_f *plist1_id , hid_t_f *plist2_id, int_f *c_flag);
H5_FCDLL int_f nh5pget_class_c ( hid_t_f *prp_id , int_f *classtype);
H5_FCDLL int_f nh5pset_deflate_c ( hid_t_f *prp_id , int_f *level);
H5_FCDLL int_f nh5pset_chunk_c ( hid_t_f *prp_id, int_f *rank, hsize_t_f *dims );
H5_FCDLL int_f nh5pget_chunk_c ( hid_t_f *prp_id, int_f *max_rank, hsize_t_f *dims );
H5_FCDLL int_f nh5pset_fill_valuec_c (hid_t_f *prp_id, hid_t_f *type_id, _fcd fillvalue);
H5_FCDLL int_f nh5pset_fill_value_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pset_fill_value_integer_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pset_fill_value_real_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pset_fill_value_double_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pget_fill_valuec_c (hid_t_f *prp_id, hid_t_f *type_id, _fcd fillvalue);
H5_FCDLL int_f nh5pget_fill_value_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pget_fill_value_integer_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pget_fill_value_real_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pget_fill_value_double_c (hid_t_f *prp_id, hid_t_f *type_id, void *fillvalue);
H5_FCDLL int_f nh5pset_preserve_c ( hid_t_f *prp_id , int_f *flag);
H5_FCDLL int_f nh5pget_preserve_c ( hid_t_f *prp_id , int_f *flag);
H5_FCDLL int_f nh5pget_version_c (hid_t_f *prp_id, int_f * boot,int_f * freelist, int_f * stab, int_f *shhdr);
H5_FCDLL int_f nh5pset_userblock_c (hid_t_f *prp_id, hsize_t_f * size);
H5_FCDLL int_f nh5pget_userblock_c (hid_t_f *prp_id, hsize_t_f * size);
H5_FCDLL int_f nh5pget_sizes_c (hid_t_f *prp_id, size_t_f * sizeof_addr, size_t_f * sizeof_size);
H5_FCDLL int_f nh5pset_sizes_c (hid_t_f *prp_id, size_t_f * sizeof_addr, size_t_f * sizeof_size);
H5_FCDLL int_f nh5pset_sym_k_c (hid_t_f *prp_id, int_f* ik, int_f* lk);
H5_FCDLL int_f nh5pget_sym_k_c (hid_t_f *prp_id, int_f* ik, int_f* lk);
H5_FCDLL int_f nh5pset_istore_k_c (hid_t_f *prp_id, int_f* ik);
H5_FCDLL int_f nh5pget_istore_k_c (hid_t_f *prp_id, int_f* ik);
H5_FCDLL int_f nh5pget_driver_c (hid_t_f *prp_id, hid_t_f*driver);
H5_FCDLL int_f nh5pset_fapl_stdio_c (hid_t_f *prp_id);
H5_FCDLL int_f nh5pget_fapl_stdio_c (hid_t_f *prp_id, int_f* io);
H5_FCDLL int_f nh5pset_fapl_sec2_c (hid_t_f *prp_id);
H5_FCDLL int_f nh5pget_fapl_sec2_c (hid_t_f *prp_id, int_f* sec2);
H5_FCDLL int_f nh5pset_alignment_c(hid_t_f *prp_id, hsize_t_f* threshold, hsize_t_f* alignment);
H5_FCDLL int_f nh5pget_alignment_c(hid_t_f *prp_id, hsize_t_f* threshold, hsize_t_f* alignment);
H5_FCDLL int_f nh5pget_fapl_core_c (hid_t_f *prp_id, size_t_f* increment, int_f *flag);
H5_FCDLL int_f nh5pset_fapl_core_c (hid_t_f *prp_id, size_t_f* increment, int_f *flag);
H5_FCDLL int_f nh5pset_fapl_family_c (hid_t_f *prp_id, hsize_t_f* memb_size, hid_t_f* memb_plist );
H5_FCDLL int_f nh5pget_fapl_family_c (hid_t_f *prp_id, hsize_t_f* memb_size, hid_t_f* memb_plist );
H5_FCDLL int_f nh5pset_cache_c(hid_t_f *prp_id, int_f* mdc_nelmts, size_t_f* rdcc_nelmts, size_t_f* rdcc_nbytes, real_f* rdcc_w0);
H5_FCDLL int_f nh5pget_cache_c(hid_t_f *prp_id, int_f* mdc_nelmts, size_t_f* rdcc_nelmts, size_t_f* rdcc_nbytes, real_f* rdcc_w0);
H5_FCDLL int_f nh5pget_fapl_split_c(hid_t_f *prp_id, size_t_f* meta_ext_size , _fcd meta_ext, hid_t_f* meta_plist, size_t_f* raw_ext_size, _fcd raw_ext, hid_t_f * raw_plist);
H5_FCDLL int_f nh5pset_fapl_split_c(hid_t_f *prp_id, int_f* meta_len, _fcd meta_ext, hid_t_f* meta_plist, int_f* raw_len, _fcd raw_ext, hid_t_f * raw_plist);
H5_FCDLL int_f nh5pset_gc_references_c(hid_t_f *prp_id, int_f* gc_references);
H5_FCDLL int_f nh5pget_gc_references_c(hid_t_f *prp_id, int_f* gc_references);
H5_FCDLL int_f nh5pset_layout_c (hid_t_f *prp_id, int_f* layout);
H5_FCDLL int_f nh5pget_layout_c (hid_t_f *prp_id, int_f* layout);
H5_FCDLL int_f nh5pset_filter_c (hid_t_f *prp_id, int_f* filter, int_f* flags, size_t_f* cd_nelmts, int_f* cd_values );
H5_FCDLL int_f nh5premove_filter_c (hid_t_f *prp_id, int_f* filter);
H5_FCDLL int_f nh5pmodify_filter_c (hid_t_f *prp_id, int_f* filter, int_f* flags, size_t_f* cd_nelmts, int_f* cd_values );
H5_FCDLL int_f nh5pget_nfilters_c (hid_t_f *prp_id, int_f* nfilters);
H5_FCDLL int_f nh5pget_filter_c(hid_t_f *prp_id, int_f* filter_number, int_f* flags, size_t_f* cd_nelmts, int_f* cd_values, size_t_f *namelen, _fcd name, int_f* filter_id);
H5_FCDLL int_f nh5pget_filter_by_id_c(hid_t_f *prp_id, int_f* filter_id, int_f* flags, size_t_f* cd_nelmts, int_f* cd_values, size_t_f *namelen, _fcd name);
H5_FCDLL int_f nh5pset_external_c (hid_t_f *prp_id, _fcd name, int_f* namelen, int_f* offset, hsize_t_f*bytes);
H5_FCDLL int_f nh5pget_external_count_c (hid_t_f *prp_id, int_f* count);
H5_FCDLL int_f nh5pget_external_c(hid_t_f *prp_id, int_f *idx, size_t_f* name_size, _fcd name, int_f* offset, hsize_t_f*bytes);
H5_FCDLL int_f nh5pget_btree_ratios_c(hid_t_f *prp_id, real_f* left, real_f* middle, real_f* right);
H5_FCDLL int_f nh5pset_btree_ratios_c(hid_t_f *prp_id, real_f* left, real_f* middle, real_f* right);
H5_FCDLL int_f nh5pget_fapl_mpio_c(hid_t_f *prp_id, int_f* comm, int_f* info);
H5_FCDLL int_f nh5pset_fapl_mpio_c(hid_t_f *prp_id, int_f* comm, int_f* info);
H5_FCDLL int_f nh5pget_fapl_mpiposix_c(hid_t_f *prp_id, int_f* comm, int_f* flag);
H5_FCDLL int_f nh5pset_fapl_mpiposix_c(hid_t_f *prp_id, int_f* comm, int_f* flag);
H5_FCDLL int_f nh5pget_dxpl_mpio_c(hid_t_f *prp_id, int_f* data_xfer_mode);
H5_FCDLL int_f nh5pset_dxpl_mpio_c(hid_t_f *prp_id, int_f* data_xfer_mode);
H5_FCDLL int_f nh5pset_fclose_degree_c(hid_t_f *fapl, int_f *degree);
H5_FCDLL int_f nh5pget_fclose_degree_c(hid_t_f *fapl, int_f *degree);
H5_FCDLL int_f nh5pget_buffer_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pset_buffer_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pset_alloc_time_c(hid_t_f *plist, int_f *flag);
H5_FCDLL int_f nh5pget_alloc_time_c(hid_t_f *plist, int_f *flag);
H5_FCDLL int_f nh5pset_fill_time_c(hid_t_f *plist, int_f *flag);
H5_FCDLL int_f nh5pget_fill_time_c(hid_t_f *plist, int_f *flag);
H5_FCDLL int_f nh5pset_meta_block_size_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pget_meta_block_size_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pset_sieve_buf_size_c(hid_t_f *plist, size_t_f *size);
H5_FCDLL int_f nh5pget_sieve_buf_size_c(hid_t_f *plist, size_t_f *size);
H5_FCDLL int_f nh5pset_small_data_block_size_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pget_small_data_block_size_c(hid_t_f *plist, hsize_t_f *size);
H5_FCDLL int_f nh5pset_hyper_vector_size_c(hid_t_f *plist, size_t_f *size);
H5_FCDLL int_f nh5pget_hyper_vector_size_c(hid_t_f *plist, size_t_f *size);
H5_FCDLL int_f nh5pcreate_class_c(hid_t_f *parent, _fcd name, int_f *name_len, hid_t_f *cls,
				  H5P_cls_create_func_t create, void *create_data,
				  H5P_cls_copy_func_t copy, void *copy_data,
				  H5P_cls_close_func_t close, void *close_data);
H5_FCDLL int_f nh5pregister_c(hid_t_f *cls, _fcd name, int_f * name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pregister_integer_c(hid_t_f *cls, _fcd name, int_f * name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pregister_real_c(hid_t_f *cls, _fcd name, int_f * name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pregister_double_c(hid_t_f *cls, _fcd name, int_f * name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pregisterc_c(hid_t_f *cls, _fcd name, int_f * name_len, size_t_f *size, _fcd value, int_f *value_len);
H5_FCDLL int_f nh5pinsert_c(hid_t_f  *plist, _fcd name, int_f *name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pinsert_integer_c(hid_t_f  *plist, _fcd name, int_f *name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pinsert_real_c(hid_t_f  *plist, _fcd name, int_f *name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pinsert_double_c(hid_t_f  *plist, _fcd name, int_f *name_len, size_t_f *size, void *value);
H5_FCDLL int_f nh5pinsertc_c(hid_t_f  *plist, _fcd name, int_f *name_len, size_t_f *size, _fcd value, int_f *value_len);
H5_FCDLL int_f nh5pset_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pset_integer_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pset_real_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pset_double_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5psetc_c(hid_t_f *prp_id, _fcd name, int_f *name_len, _fcd value, int_f *value_len);
H5_FCDLL int_f nh5pget_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pget_double_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pget_integer_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pget_real_c(hid_t_f *prp_id, _fcd name, int_f *name_len, void *value);
H5_FCDLL int_f nh5pgetc_c(hid_t_f *prp_id, _fcd name, int_f *name_len, _fcd value, int_f *value_len);
H5_FCDLL int_f nh5pexist_c(hid_t_f *prp_id, _fcd name, int_f *name_len);
H5_FCDLL int_f nh5pget_size_c(hid_t_f *prp_id, _fcd name, int_f *name_len, size_t_f *size);
H5_FCDLL int_f nh5pget_nprops_c(hid_t_f *prp_id, size_t_f *nprops);
H5_FCDLL int_f nh5pget_class_parent_c(hid_t_f *prp_id, hid_t_f *parent_id);
H5_FCDLL int_f nh5pisa_class_c(hid_t_f *plist, hid_t_f *pclass);
H5_FCDLL int_f nh5pcopy_prop_c(hid_t_f *dst_id, hid_t_f *src_id, _fcd name, int_f *name_len);
H5_FCDLL int_f nh5premove_c(hid_t_f *plid, _fcd name, int_f *name_len);
H5_FCDLL int_f nh5punregister_c(hid_t_f *cls, _fcd name, int_f *name_len);
H5_FCDLL int_f nh5pclose_class_c(hid_t_f * cls);
H5_FCDLL int_f nh5pget_class_name_c(hid_t_f *prp_id, _fcd name, int_f *name_len);
H5_FCDLL int_f nh5pset_shuffle_c ( hid_t_f *prp_id);
H5_FCDLL int_f nh5pset_fletcher32_c ( hid_t_f *prp_id );
H5_FCDLL int_f nh5pset_edc_check_c ( hid_t_f *prp_id, int_f *flag );
H5_FCDLL int_f nh5pget_edc_check_c ( hid_t_f *prp_id, int_f *flag );
H5_FCDLL int_f nh5pset_family_offset_c ( hid_t_f *prp_id , hsize_t_f *offset);
H5_FCDLL int_f nh5pget_fapl_multi_c ( hid_t_f *prp_id , int_f *mem_map, hid_t_f *memb_fapl, _fcd memb_name, int_f *len, int_f *lenmax, real_f *memb_addr, int_f *flag, int_f *maxlen_out);
H5_FCDLL int_f nh5pset_fapl_multi_c ( hid_t_f *prp_id , int_f *mem_map, hid_t_f *memb_fapl, _fcd memb_name, int_f *len, int_f *lenmax, real_f *memb_addr, int_f *flag);
H5_FCDLL int_f nh5pset_fapl_multi_sc ( hid_t_f *prp_id , int_f *flag);
H5_FCDLL int_f nh5pset_szip_c ( hid_t_f *prp_id , int_f *options_mask, int_f *pixels_per_block);
H5_FCDLL int_f nh5pall_filters_avail_c ( hid_t_f *prp_id , int_f *status);
H5_FCDLL int_f nh5pfill_value_defined_c ( hid_t_f *prp_id , int_f *flag);
H5_FCDLL int_f nh5pget_attr_phase_change_c (hid_t_f *ocpl_id, int_f *max_compact, int_f *min_dense );
H5_FCDLL int_f nh5pset_attr_creation_order_c(hid_t_f *ocpl_id, int_f *crt_order_flags );
H5_FCDLL int_f nh5pset_shared_mesg_nindexes_c(hid_t_f *plist_id, int_f *nindexes );
H5_FCDLL int_f nh5pset_shared_mesg_index_c(hid_t_f *fcpl_id, int_f *index_num, int_f *mesg_type_flags, int_f *min_mesg_size);
H5_FCDLL int_f nh5pget_attr_creation_order_c(hid_t_f *ocpl_id, int_f *crt_order_flags);
H5_FCDLL int_f nh5pset_libver_bounds_c(hid_t_f *fapl_id, int_f *low, int_f *high);
H5_FCDLL int_f nh5pset_link_creation_order_c(hid_t_f *gcpl_id, int_f *crt_order_flags);
H5_FCDLL int_f nh5pget_link_phase_change_c(hid_t_f *gcpl_id, int_f *max_compact, int_f *min_dense );
H5_FCDLL int_f nh5pget_obj_track_times_c(hid_t_f *plist_id, int_f *flag);
H5_FCDLL int_f nh5pset_obj_track_times_c(hid_t_f *plist_id, int_f *flag);
H5_FCDLL int_f nh5pset_create_inter_group_c(hid_t_f *lcpl_id, int_f *crt_intermed_group);
H5_FCDLL int_f nh5pget_create_inter_group_c(hid_t_f *lcpl_id, int_f *crt_intermed_group);
H5_FCDLL int_f nh5pget_link_creation_order_c(hid_t_f *gcpl_id, int_f *crt_order_flags);
H5_FCDLL int_f nh5pset_char_encoding_c(hid_t_f *plist_id, int_f *encoding);
H5_FCDLL int_f nh5pget_char_encoding_c(hid_t_f *plist_id, int_f *encoding);
H5_FCDLL int_f nh5pset_copy_object_c(hid_t_f *ocp_plist_id, int_f *copy_options);
H5_FCDLL int_f nh5pget_copy_object_c(hid_t_f *ocp_plist_id, int_f *copy_options);
H5_FCDLL int_f nh5pget_data_transform_c(hid_t_f *plist_id, _fcd expression, int_f *expression_len, size_t_f *size);
H5_FCDLL int_f nh5pset_data_transform_c(hid_t_f *plist_id, _fcd expression, int_f *expression_len);
H5_FCDLL int_f nh5pget_local_heap_size_hint_c(hid_t_f *gcpl_id, size_t_f *size_hint);
H5_FCDLL int_f nh5pget_est_link_info_c(hid_t_f *gcpl_id, int_f *est_num_entries, int_f *est_name_len);
H5_FCDLL int_f nh5pset_local_heap_size_hint_c(hid_t_f *gcpl_id, size_t_f *size_hint);
H5_FCDLL int_f nh5pset_est_link_info_c(hid_t_f *gcpl_id, int_f *est_num_entries, int_f *est_name_len);
H5_FCDLL int_f nh5pset_link_phase_change_c(hid_t_f *gcpl_id, int_f *max_compact, int_f *min_dense );
H5_FCDLL int_f nh5pset_fapl_direct_c(hid_t_f *fapl_id, size_t_f *alignment, size_t_f *block_size, size_t_f *cbuf_size );
H5_FCDLL int_f nh5pget_fapl_direct_c(hid_t_f *fapl_id, size_t_f *alignment, size_t_f *block_size, size_t_f *cbuf_size );
H5_FCDLL int_f nh5pset_attr_phase_change_c (hid_t_f *ocpl_id, int_f *max_compact, int_f *min_dense );
H5_FCDLL int_f nh5pset_nbit_c(hid_t_f *plist_id );
H5_FCDLL int_f nh5pset_scaleoffset_c(hid_t_f *plist_id, int_f *scale_type, int_f *scale_factor );
H5_FCDLL int_f nh5pset_nlinks_c(hid_t_f *lapl_id, size_t_f *nlinks);
H5_FCDLL int_f nh5pget_nlinks_c(hid_t_f *lapl_id, size_t_f *nlinks);
H5_FCDLL int_f nh5pset_chunk_cache_c(hid_t_f *dapl_id, size_t_f *rdcc_nslots, size_t_f *rdcc_nbytes, real_f *rdcc_w0);
H5_FCDLL int_f nh5pget_chunk_cache_c(hid_t_f *dapl_id, size_t_f *rdcc_nslots, size_t_f *rdcc_nbytes, real_f *rdcc_w0);
H5_FCDLL int_f nh5pget_mpio_actual_io_mode_c(hid_t_f *dxpl_id, int_f *actual_io_mode);
/*
 * Functions frome H5Rf.c
 */
#define nh5rcreate_object_c     H5_FC_FUNC_(h5rcreate_object_c, H5RCREATE_OBJECT_C)
#define nh5rcreate_region_c     H5_FC_FUNC_(h5rcreate_region_c, H5RCREATE_REGION_C)
#define nh5rcreate_ptr_c     H5_FC_FUNC_(h5rcreate_ptr_c, H5RCREATE_PTR_C)
#define nh5rdereference_region_c H5_FC_FUNC_(h5rdereference_region_c, H5RDEREFERENCE_REGION_C)
#define nh5rdereference_object_c H5_FC_FUNC_(h5rdereference_object_c, H5RDEREFERENCE_OBJECT_C)
#define nh5rdereference_ptr_c H5_FC_FUNC_(h5rdereference_ptr_c, H5RDEREFERENCE_PTR_C)
#define nh5rget_region_region_c H5_FC_FUNC_(h5rget_region_region_c, H5RGET_REGION_REGION_C)
#define nh5rget_region_ptr_c H5_FC_FUNC_(h5rget_region_ptr_c, H5RGET_REGION_PTR_C)
#define nh5rget_object_type_obj_c H5_FC_FUNC_(h5rget_object_type_obj_c, H5RGET_OBJECT_TYPE_OBJ_C)
#define nh5rget_name_object_c H5_FC_FUNC_(h5rget_name_object_c, H5RGET_NAME_OBJECT_C)
#define nh5rget_name_region_c H5_FC_FUNC_(h5rget_name_region_c, H5RGET_NAME_REGION_C)
#define nh5rget_name_ptr_c H5_FC_FUNC_(h5rget_name_ptr_c, H5RGET_NAME_PTR_C)
#define nh5rget_obj_type_c H5_FC_FUNC_(h5rget_obj_type_c, H5RGET_OBJ_TYPE_C)


H5_FCDLL int_f nh5rcreate_object_c (haddr_t_f *ref, hid_t_f *loc_id, _fcd name, int_f *namelen);
H5_FCDLL int_f nh5rcreate_region_c (int_f *ref, hid_t_f *loc_id, _fcd name, int_f *namelen, hid_t_f *space_id);
H5_FCDLL int_f nh5rcreate_ptr_c (void *ref, hid_t_f *loc_id, _fcd name, int_f *namelen, int_f *ref_type, hid_t_f *space_id);
H5_FCDLL int_f nh5rdereference_region_c (hid_t_f *dset_id, int_f *ref, hid_t_f *obj_id);
H5_FCDLL int_f nh5rdereference_object_c (hid_t_f *dset_id, haddr_t_f *ref, hid_t_f *obj_id);
H5_FCDLL int_f nh5rdereference_ptr_c (hid_t_f *obj_id, int_f *ref_type, void *ref, hid_t_f *ref_obj_id);
H5_FCDLL int_f nh5rget_region_region_c (hid_t_f *dset_id, int_f *ref, hid_t_f *space_id);
H5_FCDLL int_f nh5rget_region_ptr_c(hid_t_f *dset_id, void *ref, hid_t_f *space_id);
H5_FCDLL int_f nh5rget_object_type_obj_c (hid_t_f *dset_id, haddr_t_f *ref, int_f *obj_type);
H5_FCDLL int_f nh5rget_name_object_c (hid_t_f *loc_id, haddr_t_f *ref, _fcd name, size_t_f *name_len, size_t_f *size_default);
H5_FCDLL int_f nh5rget_name_region_c (hid_t_f *loc_id, int_f *ref, _fcd name, size_t_f *name_len, size_t_f *size_default);
H5_FCDLL int_f nh5rget_name_ptr_c (hid_t_f *loc_id, int_f *ref_type, void *ref, _fcd name, size_t_f *name_len, size_t_f *size_default);
H5_FCDLL int_f nh5rget_obj_type_c (hid_t_f *loc_id, int_f *ref_type, void *ref, int_f *obj_type);
/*
 * Functions from H5If.c
 */
#define nh5iget_type_c    H5_FC_FUNC_(h5iget_type_c, H5IGET_TYPE_C)
#define nh5iget_name_c    H5_FC_FUNC_(h5iget_name_c, H5IGET_NAME_C)
#define nh5iinc_ref_c     H5_FC_FUNC_(h5iinc_ref_c, H5IINC_REF_C)
#define nh5idec_ref_c     H5_FC_FUNC_(h5idec_ref_c, H5IDEC_REF_C)
#define nh5iget_ref_c     H5_FC_FUNC_(h5iget_ref_c, H5IGET_REF_C)
#define nh5iget_file_id_c H5_FC_FUNC_(h5iget_file_id_c, H5IGET_FILE_ID_C)
#define nh5iis_valid_c    H5_FC_FUNC_(h5iis_valid_c, H5IIS_VALID_C)

H5_FCDLL int_f nh5iget_type_c(hid_t_f *obj_id, int_f *type);
H5_FCDLL int_f nh5iget_name_c(hid_t_f *obj_id, _fcd buf, size_t_f *buf_size, size_t_f *name_size);
H5_FCDLL int_f nh5iinc_ref_c(hid_t_f *obj_id, int_f *ref_count);
H5_FCDLL int_f nh5idec_ref_c(hid_t_f *obj_id, int_f *ref_count);
H5_FCDLL int_f nh5iget_ref_c(hid_t_f *obj_id, int_f *ref_count);
H5_FCDLL int_f nh5iget_file_id_c(hid_t_f *obj_id, hid_t_f *file_id);
H5_FCDLL int_f nh5iis_valid_c(hid_t_f *obj_id, int_f *c_valid);

/*
 * Functions from H5Ef.c
 */
#define nh5eclear_c     H5_FC_FUNC_(h5eclear_c, H5ECLEAR_C)
#define nh5eprint_c1    H5_FC_FUNC_(h5eprint_c1, H5EPRINT_C1)
#define nh5eprint_c2    H5_FC_FUNC_(h5eprint_c2, H5EPRINT_C2)
#define nh5eget_major_c H5_FC_FUNC_(h5eget_major_c, H5EGET_MAJOR_C)
#define nh5eget_minor_c H5_FC_FUNC_(h5eget_minor_c, H5EGET_MINOR_C)
#define nh5eset_auto_c  H5_FC_FUNC_(h5eset_auto_c, H5ESET_AUTO_C)
#define nh5eset_auto2_c  H5_FC_FUNC_(h5eset_auto2_c, H5ESET_AUTO2_C)
#define nprocess_buffer  H5_FC_FUNC_(process_buffer, PROCESS_BUFFER)


H5_FCDLL int_f nh5eclear_c(hid_t_f *estack_id);
H5_FCDLL int_f nh5eprint_c1(_fcd name, int_f* namelen);
H5_FCDLL int_f nh5eprint_c2(void);
H5_FCDLL int_f nh5eget_major_c(int_f* error_no, _fcd name, size_t_f* namelen);
H5_FCDLL int_f nh5eget_minor_c(int_f* error_no, _fcd name, size_t_f* namelen);
H5_FCDLL int_f nh5eset_auto_c(int_f* printflag);
H5_FCDLL int_f nh5eset_auto2_c(int_f* printflag, hid_t_f *estack_id, H5E_auto2_t func, void *client_data);
H5_FCDLL int_f nprocess_buffer(hid_t_f *estack_id,void **buffer);

/*
 * Functions from H5f.c
 */
#define nh5open_c            H5_FC_FUNC_(h5open_c, H5OPEN_C)
#define nh5close_c           H5_FC_FUNC_(h5close_c, H5CLOSE_C)
#define nh5init_types_c      H5_FC_FUNC_(h5init_types_c, H5INIT_TYPES_C)
#define nh5close_types_c     H5_FC_FUNC_(h5close_types_c, H5CLOSE_TYPES_C)
#define nh5init_flags_c      H5_FC_FUNC_(h5init_flags_c, H5INIT_FLAGS_C)
#define nh5init1_flags_c     H5_FC_FUNC_(h5init1_flags_c, H5INIT1_FLAGS_C)
#define nh5get_libversion_c  H5_FC_FUNC_(h5get_libversion_c, H5GET_LIBVERSION_C)
#define nh5check_version_c   H5_FC_FUNC_(h5check_version_c, H5CHECK_VERSION_C)
#define nh5garbage_collect_c H5_FC_FUNC_(h5garbage_collect_c, H5GARBAGE_COLLECT_C)
#define nh5dont_atexit_c     H5_FC_FUNC_(h5dont_atexit_c, H5DONT_ATEXIT_C)


H5_FCDLL int_f nh5open_c(void);
H5_FCDLL int_f nh5close_c(void);
H5_FCDLL int_f nh5init_types_c(hid_t_f *types, hid_t_f * floatingtypes, hid_t_f * integertypes);
H5_FCDLL int_f nh5close_types_c(hid_t_f *types, int_f *lentypes, hid_t_f * floatingtypes, int_f * floatinglen, hid_t_f * integertypes,  int_f * integerlen);
H5_FCDLL int_f nh5init_flags_c(int_f *h5d_flags, int_f *h5e_flags, hid_t_f *h5e_hid_flags, int_f *h5f_flags,
			       int_f *h5fd_flags, hid_t_f *h5fd_hid_flags,
			       int_f *h5g_flags, int_f *h5i_flags, int_f *h5l_flags, int_f *h5o_flags,
			       hid_t_f *h5p_flags, int_f *h5p_flags_int, int_f *h5r_flags, int_f *h5s_flags,
			       hsize_t_f *h5s_hsize_flags, int_f *h5t_flags, int_f *h5z_flags, int_f *h5_generic_flags);
H5_FCDLL int_f nh5init1_flags_c(int_f *h5lib_flags);
H5_FCDLL int_f nh5get_libversion_c(int_f *majnum, int_f *minnum, int_f *relnum);
H5_FCDLL int_f nh5check_version_c(int_f *majnum, int_f *minnum, int_f *relnum);
H5_FCDLL int_f nh5garbage_collect_c(void);
H5_FCDLL int_f nh5dont_atexit_c(void);

/*
 * Functions from H5Zf.c
 */
#define nh5zunregister_c    H5_FC_FUNC_(h5zunregister_c, H5ZUNREGISTER_C)
#define nh5zfilter_avail_c  H5_FC_FUNC_(h5zfilter_avail_c, H5ZFILTER_AVAIL_C)
#define nh5zget_filter_info_c H5_FC_FUNC_(h5zget_filter_info_c, H5ZGET_FILTER_INFO_C)


H5_FCDLL int_f nh5zunregister_c (int_f *filter);
H5_FCDLL int_f nh5zfilter_avail_c (int_f *filter, int_f *flag);
H5_FCDLL int_f nh5zget_filter_info_c (int_f *filter, int_f *flag);


/*
 * Functions from H5Lf.c
 */
#define nh5lcopy_c H5_FC_FUNC_(h5lcopy_c, H5LCOPY_C)
#define nh5lcreate_external_c H5_FC_FUNC_(h5lcreate_external_c, H5LCREATE_EXTERNAL_C)
#define nh5lcreate_hard_c H5_FC_FUNC_(h5lcreate_hard_c, H5LCREATE_HARD_C)
#define nh5lcreate_soft_c H5_FC_FUNC_(h5lcreate_soft_c, H5LCREATE_SOFT_C)
#define nh5ldelete_c H5_FC_FUNC_(h5ldelete_c, H5LDELETE_C)
#define nh5ldelete_by_idx_c H5_FC_FUNC_(h5ldelete_by_idx_c, H5LDELETE_BY_IDX_C)
#define nh5lexists_c H5_FC_FUNC_(h5lexists_c, H5LEXISTS_C)
#define nh5lget_info_c H5_FC_FUNC_(h5lget_info_c, H5LGET_INFO_C)
#define nh5lget_info_by_idx_c H5_FC_FUNC_(h5lget_info_by_idx_c, H5LGET_INFO_BY_IDX_C)
#define nh5lis_registered_c H5_FC_FUNC_(h5lis_registered_c, H5LIS_REGISTERED_C)
#define nh5lmove_c H5_FC_FUNC_(h5lmove_c, H5LMOVE_C)
#define nh5lget_name_by_idx_c H5_FC_FUNC_(h5lget_name_by_idx_c, H5LGET_NAME_BY_IDX_C)
#define nh5lget_val_c H5_FC_FUNC_(h5lget_val_c, H5LGET_VAL_C) 
#define nh5literate_c H5_FC_FUNC_(h5literate_c, H5LITERATE_C) 
#define nh5literate_by_name_c H5_FC_FUNC_(h5literate_by_name_c, H5LITERATE_BY_NAME_C)

H5_FCDLL int_f nh5lcopy_c(hid_t_f *src_loc_id, _fcd src_name, size_t_f *src_namelen, hid_t_f *dest_loc_id,
			  _fcd dest_name, size_t_f *dest_namelen,
			  hid_t_f *lcpl_id, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lcreate_external_c(_fcd file_name, size_t_f *file_namelen, _fcd obj_name, size_t_f *obj_namelen,
				     hid_t_f *link_loc_id, _fcd link_name, size_t_f *link_namelen,
				     hid_t_f *lcpl_id, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lcreate_hard_c(hid_t_f *obj_loc_id, _fcd obj_name, size_t_f *obj_namelen,
				 hid_t_f *link_loc_id,
				 _fcd link_name, size_t_f *link_namelen,
				 hid_t_f *lcpl_id, hid_t_f *lapl_id );
H5_FCDLL int_f nh5lcreate_soft_c(_fcd target_path, size_t_f *target_path_len,
				 hid_t_f *link_loc_id,
				 _fcd link_name, size_t_f *link_name_len,
				 hid_t_f *lcpl_id, hid_t_f *lapl_id );
H5_FCDLL int_f nh5ldelete_c( hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id );
H5_FCDLL int_f nh5ldelete_by_idx_c (hid_t_f *loc_id, _fcd group_name, size_t_f *group_namelen,
				    int_f *index_field, int_f *order, hsize_t_f *n, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lexists_c (hid_t_f *loc_id, _fcd name, size_t_f *namelen, hid_t_f *lapl_id, int_f *link_exists);
H5_FCDLL int_f nh5lget_info_c (hid_t_f *link_loc_id, _fcd link_name, size_t_f *link_namelen,
			       int_f *cset, int_f *corder, int_f *corder_valid, int_f *link_type,
			       haddr_t_f *address, size_t_f *val_size,
			       hid_t_f *lapl_id);
H5_FCDLL int_f nh5lget_info_by_idx_c(hid_t_f *loc_id, _fcd group_name, size_t_f *group_namelen,
		      int_f *index_field, int_f *order, hsize_t_f *n,
		      int_f *link_type, int_f *corder_valid, int_f *corder, int_f *cset, haddr_t_f *address, size_t_f *val_size, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lis_registered_c(int_f *link_cls_id);
H5_FCDLL int_f nh5lmove_c(hid_t_f *src_loc_id, _fcd src_name, size_t_f *src_namelen, hid_t_f *dest_loc_id,
			  _fcd dest_name, size_t_f *dest_namelen, hid_t_f *lcpl_id, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lget_name_by_idx_c(hid_t_f *loc_id, _fcd group_name, size_t_f *group_namelen,
				     int_f *index_field, int_f *order, hsize_t_f *n,
				     size_t_f *size, _fcd name, hid_t_f *lapl_id);
H5_FCDLL int_f nh5lget_val_c(hid_t_f *link_loc_id, _fcd link_name, size_t_f *link_namelen, size_t_f *size,
			     void *linkval_buff, hid_t_f *lapl_id) ;

H5_FCDLL int_f nh5literate_c(hid_t_f *group_id, int_f *index_type, int_f *order, hsize_t_f *idx, H5L_iterate_t op, void *op_data );
H5_FCDLL int_f nh5literate_by_name_c(hid_t_f *loc_id, _fcd name, size_t_f *namelen, int_f *index_type, int_f *order, hsize_t_f *idx, H5L_iterate_t op, void *op_data, hid_t_f *lapl_id);


#endif /* _H5f90proto_H */
