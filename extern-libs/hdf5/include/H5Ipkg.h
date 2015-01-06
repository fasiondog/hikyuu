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
 *		Thursday, May 15, 2003
 *
 * Purpose:	This file contains declarations which are visible only within
 *		the H5I package.  Source files outside the H5I package should
 *		include H5Iprivate.h instead.
 */
#ifndef H5I_PACKAGE
#error "Do not include this file outside the H5I package!"
#endif

#ifndef _H5Ipkg_H
#define _H5Ipkg_H

/* Get package's private header */
#include "H5Iprivate.h"

/* Other private headers needed by this file */

/**************************/
/* Package Private Macros */
/**************************/

/*
 * Number of bits to use for ID Type in each atom. Increase if more types
 * are needed (though this will decrease the number of available IDs per
 * type). This is the only number that must be changed since all other bit
 * field sizes and masks are calculated from TYPE_BITS.
 */
#define TYPE_BITS	7
#define TYPE_MASK	((1<<TYPE_BITS)-1)

#define H5I_MAX_NUM_TYPES TYPE_MASK

/*
 * Number of bits to use for the Atom index in each atom (assumes 8-bit
 * bytes). We don't use the sign bit.
 */
#define ID_BITS		((sizeof(hid_t)*8)-(TYPE_BITS+1))
#define ID_MASK		((1<<ID_BITS)-1)

/* Map an atom to an ID type number */
#define H5I_TYPE(a)	((H5I_type_t)(((hid_t)(a)>>ID_BITS) & TYPE_MASK))


/****************************/
/* Package Private Typedefs */
/****************************/

/******************************/
/* Package Private Prototypes */
/******************************/

/* Testing functions */
#ifdef H5I_TESTING
H5_DLL ssize_t H5I_get_name_test(hid_t id, char *name/*out*/, size_t size,
    hbool_t *cached);
#endif /* H5I_TESTING */

#endif /*_H5Ipkg_H*/
