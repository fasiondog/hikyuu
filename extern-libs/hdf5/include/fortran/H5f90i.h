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


#ifndef _H5f90i_H
#define _H5f90i_H

/*
 * Include generated header.  This header defines integer types,
 * so this file only needs to define _fcd.
 */
#include "H5f90i_gen.h"

/* Define _fcd.  These are the same on every system
 * but UNICOS.
 */
#define _fcdtocp(desc) (desc)

#if (defined (UNICOS) || defined (_UNICOS)) && !defined(__crayx1)

#include <fortran.h>

/*typedef char*              _fcd;*/

#else

typedef char              *_fcd;

#endif

#endif /* _H5f90i_H */
