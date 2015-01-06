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

#ifndef _H5DSetMemXferPropList_H
#define _H5DSetMemXferPropList_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP DSetMemXferPropList : public PropList {
   public:
	static const DSetMemXferPropList DEFAULT;

	// Creates a dataset memory and transfer property list.
	DSetMemXferPropList();

	// Sets type conversion and background buffers.
	void setBuffer( size_t size, void* tconv, void* bkg ) const;

	// Reads buffer settings.
	size_t getBuffer( void** tconv, void** bkg ) const;

	// Sets B-tree split ratios for a dataset transfer property list.
	void setBtreeRatios( double left, double middle, double right ) const;

	// Gets B-tree split ratios for a dataset transfer property list.
	void getBtreeRatios( double& left, double& middle, double& right ) const;

	// Sets the dataset transfer property list status to TRUE or FALSE.
	void setPreserve( bool status ) const;

	// Checks status of the dataset transfer property list.
	bool getPreserve() const;

	// Sets an exception handling callback for datatype conversion.
	void setTypeConvCB( H5T_conv_except_func_t op, void *user_data) const;

	// Gets the exception handling callback for datatype conversion.
	void getTypeConvCB( H5T_conv_except_func_t *op, void **user_data) const;

	// Sets the memory manager for variable-length datatype
	// allocation in H5Dread and H5Dvlen_reclaim.
	void setVlenMemManager( H5MM_allocate_t alloc, void* alloc_info,
				H5MM_free_t free, void* free_info ) const;

	// alloc and free are set to NULL, indicating that system
	// malloc and free are to be used.
	void setVlenMemManager() const;

	// Gets the memory manager for variable-length datatype
	// allocation in H5Dread and H5Tvlen_reclaim.
	void getVlenMemManager( H5MM_allocate_t& alloc, void** alloc_info,
				H5MM_free_t& free, void** free_info ) const;

	// Sets the data transfer property list for the multi-file driver.
	void setMulti(const hid_t *memb_dxpl);

	// Returns multi-file data transfer property list information.
	void getMulti(hid_t *memb_dxpl);

	// Sets the size of a contiguous block reserved for small data.
	void setSmallDataBlockSize(hsize_t size);

	// Returns the current small data block size setting.
	hsize_t getSmallDataBlockSize();

	// Sets number of I/O vectors to be read/written in hyperslab I/O.
	void setHyperVectorSize(size_t vector_size);

	// Returns the number of I/O vectors to be read/written in
	// hyperslab I/O.
	size_t getHyperVectorSize();

	// Enables or disables error-detecting for a dataset reading
	// process.
	void setEDCCheck(H5Z_EDC_t check);

	// Determines whether error-detection is enabled for dataset reads.
	H5Z_EDC_t getEDCCheck();

	///\brief Returns this class name.
	virtual H5std_string fromClass () const { return("DSetMemXferPropList"); }

	// Copy constructor: makes a copy of a DSetMemXferPropList object.
	DSetMemXferPropList(const DSetMemXferPropList& orig);

	// Creates a copy of an existing dataset memory and transfer
	// property list using the property list id.
	DSetMemXferPropList(const hid_t plist_id);

	// Noop destructor
	virtual ~DSetMemXferPropList();
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
