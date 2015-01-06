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
 * Created:             H5MMprivate.h
 *                      Jul 10 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             Private header for memory management.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifndef _H5MMprivate_H
#define _H5MMprivate_H

#include "H5MMpublic.h"

/* Private headers needed by this file */
#include "H5private.h"

#ifdef NDEBUG
#define H5MM_malloc(Z)	HDmalloc(Z)
#define H5MM_calloc(Z)	HDcalloc((size_t)1,Z)
#endif /* NDEBUG */
#define H5MM_free(Z)	HDfree(Z)

/*
 * Library prototypes...
 */
#ifndef NDEBUG
H5_DLL void *H5MM_malloc(size_t size);
H5_DLL void *H5MM_calloc(size_t size);
#endif /* NDEBUG */
H5_DLL void *H5MM_realloc(void *mem, size_t size);
H5_DLL char *H5MM_xstrdup(const char *s);
H5_DLL char *H5MM_strdup(const char *s);
H5_DLL void *H5MM_xfree(void *mem);

#endif
