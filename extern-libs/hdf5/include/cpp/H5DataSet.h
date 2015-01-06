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

// Class DataSet inherits from AbstractDs and provides accesses to a dataset.

#ifndef _H5DataSet_H
#define _H5DataSet_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP DataSet : public H5Object, public AbstractDs {
   public:
	// Close this dataset.
	virtual void close();

	// Extends the dataset with unlimited dimension.
	void extend( const hsize_t* size ) const;

	// Fills a selection in memory with a value
	void fillMemBuf(const void *fill, DataType& fill_type, void *buf, DataType& buf_type, DataSpace& space);
	// Fills a selection in memory with zero
	void fillMemBuf(void *buf, DataType& buf_type, DataSpace& space);

	// Gets the creation property list of this dataset.
	DSetCreatPropList getCreatePlist() const;

	// Returns the address of this dataset in the file.
	haddr_t getOffset() const;

	// Gets the dataspace of this dataset.
	virtual DataSpace getSpace() const;

	// Determines whether space has been allocated for a dataset.
	void getSpaceStatus(H5D_space_status_t& status) const;

	// Returns the amount of storage size required for this dataset.
	virtual hsize_t getStorageSize() const;

	// Returns the in memory size of this attribute's data.
	virtual size_t getInMemDataSize() const;

	// Returns the number of bytes required to store VL data.
	hsize_t getVlenBufSize( DataType& type, DataSpace& space ) const;

	// Reclaims VL datatype memory buffers.
	static void vlenReclaim(const DataType& type, const DataSpace& space, const DSetMemXferPropList& xfer_plist, void* buf );
	static void vlenReclaim(void *buf, const DataType& type, const DataSpace& space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT);

	// Reads the data of this dataset and stores it in the provided buffer.
	// The memory and file dataspaces and the transferring property list
	// can be defaults.
	void read( void* buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT ) const;
        void read( H5std_string& buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT ) const;

	// Writes the buffered data to this dataset.
	// The memory and file dataspaces and the transferring property list
	// can be defaults.
	void write( const void* buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT ) const;
        void write( const H5std_string& buf, const DataType& mem_type, const DataSpace& mem_space = DataSpace::ALL, const DataSpace& file_space = DataSpace::ALL, const DSetMemXferPropList& xfer_plist = DSetMemXferPropList::DEFAULT ) const;

	// Iterates the selected elements in the specified dataspace - not implemented in C++ style yet
        int iterateElems( void* buf, const DataType& type, const DataSpace& space, H5D_operator_t op, void* op_data = NULL );

#ifndef H5_NO_DEPRECATED_SYMBOLS
	// Retrieves the type of object that an object reference points to.
	H5G_obj_t getObjType(void *ref, H5R_type_t ref_type = H5R_OBJECT) const;
#endif /* H5_NO_DEPRECATED_SYMBOLS */

	// Retrieves a dataspace with the region pointed to selected.
	DataSpace getRegion(void *ref, H5R_type_t ref_type = H5R_DATASET_REGION) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("DataSet"); }

	// Creates a dataset by way of dereference.
	DataSet(H5Object& obj, const void* ref, H5R_type_t ref_type = H5R_OBJECT);
	DataSet(H5File& h5file, const void* ref, H5R_type_t ref_type = H5R_OBJECT);
	DataSet(Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT);

	// Default constructor.
	DataSet();

	// Copy constructor.
	DataSet( const DataSet& original );

	// Creates a copy of an existing DataSet using its id.
	DataSet(const hid_t existing_id);

        // Gets the dataset id.
        virtual hid_t getId() const;

	// Destructor: properly terminates access to this dataset.
	virtual ~DataSet();

   private:
	hid_t id;       // HDF5 dataset id

#ifndef DOXYGEN_SHOULD_SKIP_THIS
        // This function contains the common code that is used by
        // getTypeClass and various API functions getXxxType
        // defined in AbstractDs for generic datatype and specific
        // sub-types
	virtual hid_t p_get_type() const;

	// Reads variable or fixed len strings from this dataset.
	void p_read_fixed_len(const hid_t mem_type_id, const hid_t mem_space_id, const hid_t file_space_id, const hid_t xfer_plist_id, H5std_string& strg) const;
	void p_read_variable_len(const hid_t mem_type_id, const hid_t mem_space_id, const hid_t file_space_id, const hid_t xfer_plist_id, H5std_string& strg) const;

   protected:
        // Sets the dataset id.
        virtual void p_setId(const hid_t new_id);
#endif // DOXYGEN_SHOULD_SKIP_THIS
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
