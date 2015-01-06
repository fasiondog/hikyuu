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

#ifndef _H5Attribute_H
#define _H5Attribute_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP Attribute : public AbstractDs, public IdComponent {
   public:
	// Closes this attribute.
	virtual void close();

	// Gets the name of the file, in which this attribute belongs.
	H5std_string getFileName() const;

	// Gets the name of this attribute.
	ssize_t getName( size_t buf_size, H5std_string& attr_name ) const;
	H5std_string getName( size_t buf_size ) const; // returns name, not its length
	H5std_string getName() const; // returns name, no argument

	// Gets a copy of the dataspace for this attribute.
	virtual DataSpace getSpace() const;

	// Returns the amount of storage size required for this attribute.
	virtual hsize_t getStorageSize() const;

	// Returns the in memory size of this attribute's data.
	virtual size_t getInMemDataSize() const;

	// Reads data from this attribute.
	void read( const DataType& mem_type, void *buf ) const;
	void read( const DataType& mem_type, H5std_string& strg ) const;

	// Writes data to this attribute.
	void write(const DataType& mem_type, const void *buf ) const;
	void write(const DataType& mem_type, const H5std_string& strg ) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("Attribute"); }

	// Creates a copy of an existing attribute using the attribute id
	Attribute( const hid_t attr_id );

	// Copy constructor: makes a copy of an existing Attribute object.
	Attribute( const Attribute& original );

	// Default constructor
	Attribute();

	// Gets the attribute id.
	virtual hid_t getId() const;

	// Destructor: properly terminates access to this attribute.
	virtual ~Attribute();

   protected:
	// Sets the attribute id.
	virtual void p_setId(const hid_t new_id);

   private:
	hid_t id;	// HDF5 attribute id

	// This function contains the common code that is used by
	// getTypeClass and various API functions getXxxType
	// defined in AbstractDs for generic datatype and specific
	// sub-types
	virtual hid_t p_get_type() const;

	// Reads variable or fixed len strings from this attribute.
	void p_read_variable_len(const DataType& mem_type, H5std_string& strg) const;
	void p_read_fixed_len(const DataType& mem_type, H5std_string& strg) const;

	// do not inherit H5Object::iterateAttrs
	int iterateAttrs() { return 0; }

	// do not inherit H5Object::renameAttr
	void renameAttr() {}
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
