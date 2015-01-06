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

// CommonFG is a protocol class.  Its existence is simply to provide the
// common services that are provided by H5File and Group.  The file or
// group in the context of this class is referred to as 'location'.

#ifndef _CommonFG_H
#define _CommonFG_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class Group;
class H5File;
class ArrayType;
class VarLenType;
class H5_DLLCPP CommonFG {
   public:
	// Creates a new group at this location which can be a file
	// or another group.
	Group createGroup(const char* name, size_t size_hint = 0) const;
	Group createGroup(const H5std_string& name, size_t size_hint = 0) const;

	// Opens an existing group in a location which can be a file
	// or another group.
	Group openGroup(const char* name) const;
	Group openGroup(const H5std_string& name) const;

	// Creates a new dataset at this location.
	DataSet createDataSet(const char* name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT) const;
	DataSet createDataSet(const H5std_string& name, const DataType& data_type, const DataSpace& data_space, const DSetCreatPropList& create_plist = DSetCreatPropList::DEFAULT) const;

	// Opens an existing dataset at this location.
	DataSet openDataSet(const char* name) const;
	DataSet openDataSet(const H5std_string& name) const;

	// Retrieves comment for the HDF5 object specified by its name.
	H5std_string getComment(const char* name, size_t bufsize=256) const;
	H5std_string getComment(const H5std_string& name, size_t bufsize=256) const;

	// Removes the comment for the HDF5 object specified by its name.
	void removeComment(const char* name) const;
	void removeComment(const H5std_string& name) const;

	// Sets the comment for an HDF5 object specified by its name.
	void setComment(const char* name, const char* comment) const;
	void setComment(const H5std_string& name, const H5std_string& comment) const;

	// Returns the value of a symbolic link.
	H5std_string getLinkval(const char* link_name, size_t size=0) const;
	H5std_string getLinkval(const H5std_string& link_name, size_t size=0) const;

	// Returns the number of objects in this group.
	hsize_t getNumObjs() const;

	// Retrieves the name of an object in this group, given the
	// object's index.
	H5std_string getObjnameByIdx(hsize_t idx) const;
	ssize_t getObjnameByIdx(hsize_t idx, char* name, size_t size) const;
	ssize_t getObjnameByIdx(hsize_t idx, H5std_string& name, size_t size) const;

#ifndef H5_NO_DEPRECATED_SYMBOLS
	// Returns the type of an object in this group, given the
	// object's index.
	H5G_obj_t getObjTypeByIdx(hsize_t idx) const;
	H5G_obj_t getObjTypeByIdx(hsize_t idx, char* type_name) const;
	H5G_obj_t getObjTypeByIdx(hsize_t idx, H5std_string& type_name) const;

	// Returns information about an HDF5 object, given by its name,
	// at this location.
	void getObjinfo(const char* name, hbool_t follow_link, H5G_stat_t& statbuf) const;
	void getObjinfo(const H5std_string& name, hbool_t follow_link, H5G_stat_t& statbuf) const;
	void getObjinfo(const char* name, H5G_stat_t& statbuf) const;
	void getObjinfo(const H5std_string& name, H5G_stat_t& statbuf) const;

	// Iterates over the elements of this group - not implemented in
	// C++ style yet.
	int iterateElems(const char* name, int *idx, H5G_iterate_t op, void *op_data);
	int iterateElems(const H5std_string& name, int *idx, H5G_iterate_t op, void *op_data);
#endif /* H5_NO_DEPRECATED_SYMBOLS */

	// Creates a link of the specified type from new_name to current_name;
	// both names are interpreted relative to the specified location id.
	void link(H5L_type_t link_type, const char* curr_name, const char* new_name) const;
	void link(H5L_type_t link_type, const H5std_string& curr_name, const H5std_string& new_name) const;

	// Removes the specified name at this location.
	void unlink(const char* name) const;
	void unlink(const H5std_string& name) const;

	// Mounts the file 'child' onto this location.
	void mount(const char* name, H5File& child, PropList& plist) const;
	void mount(const H5std_string& name, H5File& child, PropList& plist) const;

	// Unmounts the file named 'name' from this parent location.
	void unmount(const char* name) const;
	void unmount(const H5std_string& name) const;

	// Renames an object at this location.
	void move(const char* src, const char* dst) const;
	void move(const H5std_string& src, const H5std_string& dst) const;

	// Opens a generic named datatype in this location.
	DataType openDataType(const char* name) const;
	DataType openDataType(const H5std_string& name) const;

	// Opens a named array datatype in this location.
	ArrayType openArrayType(const char* name) const;
	ArrayType openArrayType(const H5std_string& name) const;

	// Opens a named compound datatype in this location.
	CompType openCompType(const char* name) const;
	CompType openCompType(const H5std_string& name) const;

	// Opens a named enumeration datatype in this location.
	EnumType openEnumType(const char* name) const;
	EnumType openEnumType(const H5std_string& name) const;

	// Opens a named integer datatype in this location.
	IntType openIntType(const char* name) const;
	IntType openIntType(const H5std_string& name) const;

	// Opens a named floating-point datatype in this location.
	FloatType openFloatType(const char* name) const;
	FloatType openFloatType(const H5std_string& name) const;

	// Opens a named string datatype in this location.
	StrType openStrType(const char* name) const;
	StrType openStrType(const H5std_string& name) const;

	// Opens a named variable length datatype in this location.
	VarLenType openVarLenType(const char* name) const;
	VarLenType openVarLenType(const H5std_string& name) const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	/// For subclasses, H5File and Group, to return the correct
	/// object id, i.e. file or group id.
	virtual hid_t getLocId() const = 0;

#endif // DOXYGEN_SHOULD_SKIP_THIS

	/// For subclasses, H5File and Group, to throw appropriate exception.
	virtual void throwException(const H5std_string& func_name, const H5std_string& msg) const = 0;

	// Default constructor.
	CommonFG();

	// Noop destructor.
	virtual ~CommonFG();

}; // end of CommonFG declaration

#ifndef H5_NO_NAMESPACE
}
#endif
#endif

