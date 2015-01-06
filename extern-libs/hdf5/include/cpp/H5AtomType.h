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

// Class AtomType is a base class, from which IntType, FloatType, StrType,
// and PredType inherit.  It provides the services that are common to these
// subclasses.  It also inherits from DataType and passes down the
// services that are common to all the datatypes.

#ifndef _H5AtomType_H
#define _H5AtomType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP AtomType : public DataType {
   public:
	// Returns the byte order of an atomic datatype.
	H5T_order_t getOrder() const;
	H5T_order_t getOrder( H5std_string& order_string ) const;

	// Sets the byte ordering of an atomic datatype.
	void setOrder( H5T_order_t order ) const;

	// Retrieves the bit offset of the first significant bit.
	// 12/05/00 - changed return type to int from size_t - C API
	int getOffset() const;

	// Sets the bit offset of the first significant bit.
	void setOffset( size_t offset ) const;

	// Retrieves the padding type of the least and most-significant bit padding.
	void getPad( H5T_pad_t& lsb, H5T_pad_t& msb ) const;

	// Sets the least and most-significant bits padding types
	void setPad( H5T_pad_t lsb, H5T_pad_t msb ) const;

	// Returns the precision of an atomic datatype.
	size_t getPrecision() const;

	// Sets the precision of an atomic datatype.
	void setPrecision( size_t precision ) const;

	// Sets the total size for an atomic datatype.
	void setSize( size_t size ) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("AtomType"); }

	// Copy constructor - makes copy of the original object
	AtomType( const AtomType& original );

	// Noop destructor
	virtual ~AtomType();

   protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// Default constructor
	AtomType();

	// Constructor that takes an existing id
	AtomType( const hid_t existing_id );
#endif // DOXYGEN_SHOULD_SKIP_THIS
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
