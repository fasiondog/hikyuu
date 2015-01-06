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

#ifndef _H5Group_H
#define _H5Group_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP Group : public H5Object, public CommonFG {
   public:
	// Close this group.
	virtual void close();

#ifndef H5_NO_DEPRECATED_SYMBOLS
	// Retrieves the type of object that an object reference points to.
	H5G_obj_t getObjType(void *ref, H5R_type_t ref_type = H5R_OBJECT) const;
#endif /* H5_NO_DEPRECATED_SYMBOLS */

	// Retrieves a dataspace with the region pointed to selected.
	DataSpace getRegion(void *ref, H5R_type_t ref_type = H5R_DATASET_REGION) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("Group"); }

	// Throw group exception.
	virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const;

	// for CommonFG to get the file id.
	virtual hid_t getLocId() const;

	// Creates a group by way of dereference.
	Group(H5Object& obj, const void* ref, H5R_type_t ref_type = H5R_OBJECT);
        Group(H5File& h5file, const void* ref, H5R_type_t ref_type = H5R_OBJECT);
        Group(Attribute& attr, const void* ref, H5R_type_t ref_type = H5R_OBJECT);

	// default constructor
	Group();

	// Copy constructor: makes a copy of the original object
	Group(const Group& original);

	// Gets the group id.
	virtual hid_t getId() const;

	// Destructor
	virtual ~Group();

	// Creates a copy of an existing group using its id.
	Group( const hid_t group_id );

   private:
	hid_t id;	// HDF5 group id

   protected:
	// Sets the group id.
	virtual void p_setId(const hid_t new_id);
};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
