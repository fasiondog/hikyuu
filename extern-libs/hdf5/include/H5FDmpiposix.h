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
 * Programmer:  Quincey Koziol <koziol@hdfgroup.org>
 *              Thursday, July 11, 2002
 *
 * Purpose:	The public header file for the mpiposix driver.
 */

#ifndef __H5FDmpiposix_H
#define __H5FDmpiposix_H

#ifdef H5_HAVE_PARALLEL
#   define H5FD_MPIPOSIX    (H5FD_mpiposix_init())
#else
#   define H5FD_MPIPOSIX    (-1)
#endif

/* Macros */

#define IS_H5FD_MPIPOSIX(f) /* (H5F_t *f) */                                \
    (H5FD_MPIPOSIX==H5F_DRIVER_ID(f))

#ifdef H5_HAVE_PARALLEL

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif

H5_DLL hid_t H5FD_mpiposix_init(void);
H5_DLL void H5FD_mpiposix_term(void);
H5_DLL herr_t H5Pset_fapl_mpiposix(hid_t fapl_id, MPI_Comm comm, hbool_t use_gpfs);
H5_DLL herr_t H5Pget_fapl_mpiposix(hid_t fapl_id, MPI_Comm *comm/*out*/, hbool_t *use_gpfs/*out*/);

#ifdef __cplusplus
}
#endif

#endif /*H5_HAVE_PARALLEL*/

#endif /* __H5FDmpiposix_H */
