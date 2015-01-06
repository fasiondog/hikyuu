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

// Class ArrayType inherits from DataType and provides wrappers for the
// HDF5 C's Array Datatypes.

#ifndef _H5ArrayType_H
#define _H5ArrayType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP ArrayType : public DataType {
   public:
	// Constructor that creates a new array data type based on the
	// specified base type.
	ArrayType(const DataType& base_type, int ndims, const hsize_t* dims);

	// Returns the number of dimensions of this array datatype.
	int getArrayNDims();

	// Returns the sizes of dimensions of this array datatype.
	int getArrayDims(hsize_t* dims);

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("ArrayType"); }

	// Copy constructor: makes copy of the original object.
	ArrayType( const ArrayType& original );

	// Constructor that takes an existing id
	ArrayType( const hid_t existing_id );

	// Noop destructor
	virtual ~ArrayType();

   protected:
	// Default constructor
	ArrayType();

   private:
	int rank;		// Rank of the array
	hsize_t* dimensions;	// Sizes of the array dimensions
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
