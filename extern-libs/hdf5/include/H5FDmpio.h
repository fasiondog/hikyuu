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
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Monday, August  2, 1999
 *
 * Purpose:	The public header file for the mpio driver.
 */
#ifndef H5FDmpio_H
#define H5FDmpio_H

#ifdef H5_HAVE_PARALLEL
#   define H5FD_MPIO	(H5FD_mpio_init())
#else
#   define H5FD_MPIO	(-1)
#endif /* H5_HAVE_PARALLEL */

/* Macros */

#define IS_H5FD_MPIO(f)	/* (H5F_t *f) */				    \
    (H5FD_MPIO==H5F_DRIVER_ID(f))

#ifdef H5_HAVE_PARALLEL
/*Turn on H5FDmpio_debug if H5F_DEBUG is on */
#ifdef H5F_DEBUG
#ifndef H5FDmpio_DEBUG
#define H5FDmpio_DEBUG
#endif
#endif

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif
H5_DLL hid_t H5FD_mpio_init(void);
H5_DLL void H5FD_mpio_term(void);
H5_DLL herr_t H5Pset_fapl_mpio(hid_t fapl_id, MPI_Comm comm, MPI_Info info);
H5_DLL herr_t H5Pget_fapl_mpio(hid_t fapl_id, MPI_Comm *comm/*out*/,
			MPI_Info *info/*out*/);
H5_DLL herr_t H5Pset_dxpl_mpio(hid_t dxpl_id, H5FD_mpio_xfer_t xfer_mode);
H5_DLL herr_t H5Pget_dxpl_mpio(hid_t dxpl_id, H5FD_mpio_xfer_t *xfer_mode/*out*/);
H5_DLL herr_t H5Pset_dxpl_mpio_collective_opt(hid_t dxpl_id, H5FD_mpio_collective_opt_t opt_mode);
H5_DLL herr_t H5Pset_dxpl_mpio_chunk_opt(hid_t dxpl_id, H5FD_mpio_chunk_opt_t opt_mode);
H5_DLL herr_t H5Pset_dxpl_mpio_chunk_opt_num(hid_t dxpl_id, unsigned num_chunk_per_proc);
H5_DLL herr_t H5Pset_dxpl_mpio_chunk_opt_ratio(hid_t dxpl_id, unsigned percent_num_proc_per_chunk);
#ifdef __cplusplus
}
#endif

#endif /* H5_HAVE_PARALLEL */

#endif
