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

#ifndef _H5FileAccPropList_H
#define _H5FileAccPropList_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

// class for file access properties
class H5_DLLCPP FileAccPropList : public PropList {
   public:
	static const FileAccPropList DEFAULT;

	// Creates a file access property list.
	FileAccPropList();

	// Modifies this property list to use the H5FD_STDIO driver
	void setStdio() const;

	// Set file driver for this property list
	void setDriver(hid_t new_driver_id, const void *new_driver_info) const;

	// Returns a low-level file driver identifier.
	hid_t getDriver() const;

	// Sets offset for family driver.
	void setFamilyOffset(hsize_t offset) const;

	// Gets offset for family driver.
	hsize_t getFamilyOffset() const;

	// Modifies this file access property list to use the sec2 driver.
	void setSec2() const;

	// Modifies this file access property list to use the H5FD_CORE
	// driver.
	void setCore (size_t increment, hbool_t backing_store) const;

	// Queries H5FD_CORE driver properties.
	void getCore (size_t& increment, hbool_t& backing_store) const;

	// Sets this file access properties list to the family driver.
	void setFamily( hsize_t memb_size, const FileAccPropList& memb_plist ) const;

	// Returns information about the family file access property list.
	void getFamily(hsize_t& memb_size, FileAccPropList& memb_plist) const;
	FileAccPropList getFamily(hsize_t& memb_size) const;

	// Emulates the old split file driver,
	void setSplit( FileAccPropList& meta_plist, FileAccPropList& raw_plist,
	     const char* meta_ext = ".meta", const char* raw_ext = ".raw" ) const;
	void setSplit( FileAccPropList& meta_plist, FileAccPropList& raw_plist,
	     const H5std_string& meta_ext, const H5std_string& raw_ext ) const;

#ifdef H5_HAVE_STREAM // for Stream Virtual File Driver
	// Modifies this file access property list to use the Stream driver.
	void setStream(H5FD_stream_fapl_t &fapl) const;

	// Retrieves the streaming I/O driver settings
	H5FD_stream_fapl_t getStream() const;
#endif

	// Sets the maximum size of the data sieve buffer.
	void setSieveBufSize(size_t bufsize) const;

	// Returns the current settings for the data sieve buffer size
	// property
	size_t getSieveBufSize() const;

	// Sets the minimum size of metadata block allocations.
	void setMetaBlockSize(hsize_t &block_size) const;

	// Returns the current metadata block size setting.
	hsize_t getMetaBlockSize() const;

	// Modifies this file access property list to use the logging driver.
	void setLog(const char *logfile, unsigned flags, size_t buf_size) const;
	void setLog(const H5std_string& logfile, unsigned flags, size_t buf_size) const;

	// Sets alignment properties of this file access property list
	void setAlignment( hsize_t threshold = 1, hsize_t alignment = 1 ) const;

	// Retrieves the current settings for alignment properties from
	// this property list.
	void getAlignment( hsize_t& threshold, hsize_t& alignment ) const;

	// Sets data type for multi driver.
	void setMultiType(H5FD_mem_t dtype) const;

	// Returns the data type property for MULTI driver.
	H5FD_mem_t getMultiType() const;

	// Sets the meta data cache and raw data chunk cache parameters.
	void setCache( int mdc_nelmts, size_t rdcc_nelmts, size_t rdcc_nbytes, double rdcc_w0 ) const;

	// Queries the meta data cache and raw data chunk cache parameters.
	void getCache( int& mdc_nelmts, size_t& rdcc_nelmts, size_t& rdcc_nbytes, double& rdcc_w0 ) const;

	// Sets the degree for the file close behavior.
	void setFcloseDegree(H5F_close_degree_t degree);

	// Returns the degree for the file close behavior.
	H5F_close_degree_t getFcloseDegree();

	// Sets garbage collecting references flag.
	void setGcReferences( unsigned gc_ref = 0 ) const;

	// Returns garbage collecting references setting.
	unsigned getGcReferences() const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("FileAccPropList"); }

	// Copy constructor: creates a copy of a FileAccPropList object.
	FileAccPropList( const FileAccPropList& original );

	// Creates a copy of an existing file access property list
	// using the property list id.
	FileAccPropList (const hid_t plist_id);

	// Noop destructor
	virtual ~FileAccPropList();
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
