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

#ifndef _H5DataSpace_H
#define _H5DataSpace_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP DataSpace : public IdComponent {
   public:
	// Default DataSpace objects
	static const DataSpace ALL;

	// Creates a dataspace object given the space type
	DataSpace(H5S_class_t type = H5S_SCALAR);

	// Creates a simple dataspace
	DataSpace(int rank, const hsize_t * dims, const hsize_t * maxdims = NULL);

	// Assignment operator
	DataSpace& operator=( const DataSpace& rhs );

	// Closes this dataspace.
	virtual void close();

	// Makes copy of an existing dataspace.
	void copy(const DataSpace& like_space);

	// Copies the extent of this dataspace.
	void extentCopy( DataSpace& dest_space ) const;

	// Gets the bounding box containing the current selection.
	void getSelectBounds( hsize_t* start, hsize_t* end ) const;

	// Gets the number of element points in the current selection.
	hssize_t getSelectElemNpoints() const;

	// Retrieves the list of element points currently selected.
	void getSelectElemPointlist( hsize_t startpoint, hsize_t numpoints, hsize_t *buf ) const;

	// Gets the list of hyperslab blocks currently selected.
	void getSelectHyperBlocklist( hsize_t startblock, hsize_t numblocks, hsize_t *buf ) const;

	// Get number of hyperslab blocks.
	hssize_t getSelectHyperNblocks() const;

	// Gets the number of elements in this dataspace selection.
	hssize_t getSelectNpoints() const;

	// Retrieves dataspace dimension size and maximum size.
	int getSimpleExtentDims( hsize_t *dims, hsize_t *maxdims = NULL ) const;

	// Gets the dimensionality of this dataspace.
	int getSimpleExtentNdims() const;

	// Gets the number of elements in this dataspace.
	// 12/05/00 - changed return type to hssize_t from hsize_t - C API
	hssize_t getSimpleExtentNpoints() const;

	// Gets the current class of this dataspace.
	H5S_class_t getSimpleExtentType() const;

	// Determines if this dataspace is a simple one.
	bool isSimple() const;

	// Sets the offset of this simple dataspace.
	void offsetSimple( const hssize_t* offset ) const;

	// Selects the entire dataspace.
	void selectAll() const;

	// Selects array elements to be included in the selection for
	// this dataspace.
	void selectElements( H5S_seloper_t op, const size_t num_elements, const hsize_t *coord) const;

	// Selects a hyperslab region to add to the current selected region.
	void selectHyperslab( H5S_seloper_t op, const hsize_t *count, const hsize_t *start, const hsize_t *stride = NULL, const hsize_t *block = NULL ) const;

	// Resets the selection region to include no elements.
	void selectNone() const;

	// Verifies that the selection is within the extent of the dataspace.
	bool selectValid() const;

	// Removes the extent from this dataspace.
	void setExtentNone() const;

	// Sets or resets the size of this dataspace.
	void setExtentSimple( int rank, const hsize_t *current_size, const hsize_t *maximum_size = NULL ) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("DataSpace"); }

	// Creates a DataSpace object using an existing dataspace id.
	DataSpace(const hid_t space_id);

	// Copy constructor: makes a copy of the original DataSpace object.
	DataSpace(const DataSpace& original);

	// Gets the dataspace id.
	virtual hid_t getId() const;

	// Destructor: properly terminates access to this dataspace.
	virtual ~DataSpace();

   private:
	hid_t id;       // HDF5 dataspace id

   protected:
	// Sets the dataspace id.
	virtual void p_setId(const hid_t new_id);
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
