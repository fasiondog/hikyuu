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
 *              Monday, April 17, 2000
 *
 * Purpose:	The public header file for the log driver.
 */
#ifndef H5FDlog_H
#define H5FDlog_H

#include "H5Ipublic.h"

#define H5FD_LOG	(H5FD_log_init())

/* Flags for H5Pset_fapl_log() */
/* Flags for tracking where reads/writes/seeks occur */
#define H5FD_LOG_LOC_READ   0x00000001
#define H5FD_LOG_LOC_WRITE  0x00000002
#define H5FD_LOG_LOC_SEEK   0x00000004
#define H5FD_LOG_LOC_IO     (H5FD_LOG_LOC_READ|H5FD_LOG_LOC_WRITE|H5FD_LOG_LOC_SEEK)
/* Flags for tracking number of times each byte is read/written */
#define H5FD_LOG_FILE_READ  0x00000008
#define H5FD_LOG_FILE_WRITE 0x00000010
#define H5FD_LOG_FILE_IO    (H5FD_LOG_FILE_READ|H5FD_LOG_FILE_WRITE)
/* Flag for tracking "flavor" (type) of information stored at each byte */
#define H5FD_LOG_FLAVOR     0x00000020
/* Flags for tracking total number of reads/writes/seeks/truncates */
#define H5FD_LOG_NUM_READ   0x00000040
#define H5FD_LOG_NUM_WRITE  0x00000080
#define H5FD_LOG_NUM_SEEK   0x00000100
#define H5FD_LOG_NUM_TRUNCATE 0x00000200
#define H5FD_LOG_NUM_IO     (H5FD_LOG_NUM_READ|H5FD_LOG_NUM_WRITE|H5FD_LOG_NUM_SEEK|H5FD_LOG_NUM_TRUNCATE)
/* Flags for tracking time spent in open/stat/read/write/seek/close */
#define H5FD_LOG_TIME_OPEN  0x00000400
#define H5FD_LOG_TIME_STAT  0x00000800
#define H5FD_LOG_TIME_READ  0x00001000
#define H5FD_LOG_TIME_WRITE 0x00002000
#define H5FD_LOG_TIME_SEEK  0x00004000
#define H5FD_LOG_TIME_CLOSE 0x00008000
#define H5FD_LOG_TIME_IO    (H5FD_LOG_TIME_OPEN|H5FD_LOG_TIME_STAT|H5FD_LOG_TIME_READ|H5FD_LOG_TIME_WRITE|H5FD_LOG_TIME_SEEK|H5FD_LOG_TIME_CLOSE)
/* Flag for tracking allocation of space in file */
#define H5FD_LOG_ALLOC      0x00010000
#define H5FD_LOG_ALL        (H5FD_LOG_ALLOC|H5FD_LOG_TIME_IO|H5FD_LOG_NUM_IO|H5FD_LOG_FLAVOR|H5FD_LOG_FILE_IO|H5FD_LOG_LOC_IO)

#ifdef __cplusplus
extern "C" {
#endif

H5_DLL hid_t H5FD_log_init(void);
H5_DLL void H5FD_log_term(void);
H5_DLL herr_t H5Pset_fapl_log(hid_t fapl_id, const char *logfile, unsigned long long flags, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif

