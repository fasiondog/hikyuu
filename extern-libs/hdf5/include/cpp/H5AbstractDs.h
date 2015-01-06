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

// Class AbstractDs is an abstract base class, from which Attribute and
// DataSet inherit.  It provides the services that are common to both
// Attribute and DataSet.  It also inherits from H5Object and passes down
// the services that H5Object provides.

#ifndef _AbstractDs_H
#define _AbstractDs_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class ArrayType;
class CompType;
class EnumType;
class FloatType;
class IntType;
class StrType;
class VarLenType;
class H5_DLLCPP AbstractDs {
   public:
	// Gets a copy the datatype of that this abstract dataset uses.
	// Note that this datatype is a generic one and can only be accessed
	// via generic member functions, i.e., member functions belong
	// to DataType.  To get specific datatype, i.e. EnumType, FloatType,
	// etc..., use the specific functions, that follow, instead.
	DataType getDataType() const;

	// Gets a copy of the specific datatype of this abstract dataset.
	ArrayType getArrayType() const;
	CompType getCompType() const;
	EnumType getEnumType() const;
	IntType getIntType() const;
	FloatType getFloatType() const;
	StrType getStrType() const;
	VarLenType getVarLenType() const;

	// Gets the size in memory of this abstract dataset.
	virtual size_t getInMemDataSize() const = 0;

	// Gets the dataspace of this abstract dataset - pure virtual.
	virtual DataSpace getSpace() const = 0;

	// Gets the class of the datatype that is used by this abstract
	// dataset.
	H5T_class_t getTypeClass() const;

	// Returns the amount of storage size required for this abstract
	// dataset - pure virtual.
	virtual hsize_t getStorageSize() const = 0;

	// Returns this class name
	virtual H5std_string fromClass() const = 0;

	// Copy constructor
	AbstractDs( const AbstractDs& original );

	// Destructor
	virtual ~AbstractDs();

   protected:
	// Default constructor
	AbstractDs();

	// Constructor that takes an attribute id or a dataset id.
	AbstractDs( const hid_t ds_id );

   private:
	// This member function is implemented by DataSet and Attribute.
	virtual hid_t p_get_type() const = 0;
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif // _AbstractDs_H
