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

#ifndef _H5IntType_H
#define _H5IntType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP IntType : public AtomType {
   public:
	// Creates a integer type using a predefined type
	IntType(const PredType& pred_type);

	// Gets the integer datatype of the specified dataset
	IntType(const DataSet& dataset);

	// Retrieves the sign type for an integer type
	H5T_sign_t getSign() const;

	// Sets the sign proprety for an integer type.
	void setSign( H5T_sign_t sign ) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("IntType"); }

	// Default constructor
	IntType();

	// Creates a integer datatype using an existing id
	IntType(const hid_t existing_id);

	// Copy constructor: makes copy of IntType object
	IntType(const IntType& original);

	// Noop destructor.
	virtual ~IntType();
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
