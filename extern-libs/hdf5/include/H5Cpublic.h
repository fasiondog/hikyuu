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
 * Created:	H5Cpublic.h
 *              June 4, 2005
 *              John Mainzer
 *
 * Purpose:     Public include file for cache functions.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifndef _H5Cpublic_H
#define _H5Cpublic_H

/* Public headers needed by this file */
#include "H5public.h"

#ifdef __cplusplus
extern "C" {
#endif

enum H5C_cache_incr_mode
{
    H5C_incr__off,
    H5C_incr__threshold
};

enum H5C_cache_flash_incr_mode
{
     H5C_flash_incr__off,
     H5C_flash_incr__add_space
};

enum H5C_cache_decr_mode
{
    H5C_decr__off,
    H5C_decr__threshold,
    H5C_decr__age_out,
    H5C_decr__age_out_with_threshold
};

#ifdef __cplusplus
}
#endif
#endif
