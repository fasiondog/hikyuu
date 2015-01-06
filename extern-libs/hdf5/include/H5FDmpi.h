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
 * Programmer:  Quincey Koziol <koziol@ncsa.uiuc.edu>
 *              Friday, January 30, 2004
 *
 * Purpose:	The public header file for common items for all MPI VFL drivers
 */
#ifndef H5FDmpi_H
#define H5FDmpi_H

/***** Macros for One linked collective IO case. *****/
/* The default value to do one linked collective IO for all chunks.
   If the average number of chunks per process is greater than this value,
      the library will create an MPI derived datatype to link all chunks to do collective IO.
      The user can set this value through an API. */

#define H5D_ONE_LINK_CHUNK_IO_THRESHOLD 0
/***** Macros for multi-chunk collective IO case. *****/
/* The default value of the threshold to do collective IO for this chunk.
   If the average percentage of processes per chunk is greater than the default value,
   collective IO is done for this chunk.
*/

#define H5D_MULTI_CHUNK_IO_COL_THRESHOLD 60
/* Type of I/O for data transfer properties */
typedef enum H5FD_mpio_xfer_t {
    H5FD_MPIO_INDEPENDENT = 0, 		/*zero is the default*/
    H5FD_MPIO_COLLECTIVE
} H5FD_mpio_xfer_t;

/* Type of chunked dataset I/O */
typedef enum H5FD_mpio_chunk_opt_t {
    H5FD_MPIO_CHUNK_DEFAULT = 0,
    H5FD_MPIO_CHUNK_ONE_IO,  		/*zero is the default*/
    H5FD_MPIO_CHUNK_MULTI_IO
} H5FD_mpio_chunk_opt_t;

/* Type of collective I/O */
typedef enum H5FD_mpio_collective_opt_t {
    H5FD_MPIO_COLLECTIVE_IO = 0,
    H5FD_MPIO_INDIVIDUAL_IO  		/*zero is the default*/
} H5FD_mpio_collective_opt_t;


#ifdef H5_HAVE_PARALLEL

/* Sub-class the H5FD_class_t to add more specific functions for MPI-based VFDs */
typedef struct H5FD_class_mpi_t {
    H5FD_class_t        super;          /* Superclass information & methods */
    int  (*get_rank)(const H5FD_t *file);     /* Get the MPI rank of a process */
    int  (*get_size)(const H5FD_t *file);     /* Get the MPI size of a communicator */
    MPI_Comm (*get_comm)(const H5FD_t *file); /* Get the communicator for a file */
} H5FD_class_mpi_t;
#endif /* H5_HAVE_PARALLEL */

/* Include all the MPI VFL headers */
#include "H5FDmpio.h"           /* MPI I/O file driver			*/
#include "H5FDmpiposix.h"       /* MPI/posix I/O file driver            */

/* Macros */

/* Single macro to check for all file drivers that use MPI */
#define IS_H5FD_MPI(file)  \
        (IS_H5FD_MPIO(file) || IS_H5FD_MPIPOSIX(file))

#ifdef H5_HAVE_PARALLEL
/* ======== Temporary data transfer properties ======== */
/* Definitions for memory MPI type property */
#define H5FD_MPI_XFER_MEM_MPI_TYPE_NAME        "H5FD_mpi_mem_mpi_type"
/* Definitions for file MPI type property */
#define H5FD_MPI_XFER_FILE_MPI_TYPE_NAME       "H5FD_mpi_file_mpi_type"

/*
 * The view is set to this value
 */
H5_DLLVAR char H5FD_mpi_native_g[];

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif
/* General routines */
H5_DLL haddr_t H5FD_mpi_MPIOff_to_haddr(MPI_Offset mpi_off);
H5_DLL herr_t H5FD_mpi_haddr_to_MPIOff(haddr_t addr, MPI_Offset *mpi_off/*out*/);
H5_DLL herr_t H5FD_mpi_comm_info_dup(MPI_Comm comm, MPI_Info info,
				MPI_Comm *comm_new, MPI_Info *info_new);
H5_DLL herr_t H5FD_mpi_comm_info_free(MPI_Comm *comm, MPI_Info *info);
#ifdef NOT_YET
H5_DLL herr_t H5FD_mpio_wait_for_left_neighbor(H5FD_t *file);
H5_DLL herr_t H5FD_mpio_signal_right_neighbor(H5FD_t *file);
#endif /* NOT_YET */
H5_DLL herr_t H5FD_mpi_setup_collective(hid_t dxpl_id, MPI_Datatype *btype,
    MPI_Datatype *ftype);

/* Driver specific methods */
H5_DLL int H5FD_mpi_get_rank(const H5FD_t *file);
H5_DLL int H5FD_mpi_get_size(const H5FD_t *file);
H5_DLL MPI_Comm H5FD_mpi_get_comm(const H5FD_t *_file);
#ifdef __cplusplus
}
#endif

#endif /* H5_HAVE_PARALLEL */

#endif /* H5FDmpi_H */

