// C++ informative line for the emacs editor: -*- C++ -*-
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

// Class VarLenType inherits from DataType and provides wrappers for
// the HDF5 C's Variable-length Datatypes.

#ifndef _H5VarLenType_H
#define _H5VarLenType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP VarLenType : public DataType {
   public:
	// Constructor that creates a variable-length datatype based
	// on the specified base type.
	VarLenType(const DataType* base_type);

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("VarLenType"); }

	// Copy constructor: makes copy of the original object.
	VarLenType( const VarLenType& original );

	// Constructor that takes an existing id
	VarLenType( const hid_t existing_id );

	// Noop destructor
	virtual ~VarLenType();

   protected:
	// Default constructor
	VarLenType();
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
