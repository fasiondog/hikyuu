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

#ifndef _H5StrType_H
#define _H5StrType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP StrType : public AtomType {
   public:
	// Creates a string type using a predefined type
	StrType(const PredType& pred_type);

	// Creates a string type with specified length - may be obsolete
	StrType(const PredType& pred_type, const size_t& size);

	// Creates a string type with specified length
	StrType(const int dummy, const size_t& size);

        // Gets the string datatype of the specified dataset
	StrType(const DataSet& dataset);

	// Retrieves the character set type of this string datatype.
	H5T_cset_t getCset() const;

	// Sets character set to be used.
	void setCset(H5T_cset_t cset) const;

	// Retrieves the string padding method for this string datatype.
	H5T_str_t getStrpad() const;

	// Defines the storage mechanism for character strings.
	void setStrpad(H5T_str_t strpad) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("StrType"); }

	// default constructor
	StrType();

	// Creates a string datatype using an existing id
	StrType(const hid_t existing_id);

	// Copy constructor - makes a copy of the original object
	StrType(const StrType& original);

	// Noop destructor.
	virtual ~StrType();
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
