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

#ifndef _H5PropList_H
#define _H5PropList_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

class H5_DLLCPP PropList : public IdComponent {
   public:
	// Default property list
        static const PropList DEFAULT;

	// Creates a property list of a given type or creates a copy of an
	// existing property list giving the property list id.
	PropList(const hid_t plist_id);

	// Make a copy of the given property list using assignment statement
	PropList& operator=( const PropList& rhs );

	// Compares this property list or class against the given list or class.
	bool operator==(const PropList& rhs) const;

	// Close this property list.
	virtual void close();

	// Close a property list class.
	void closeClass() const;

	// Makes a copy of the given property list.
	void copy( const PropList& like_plist );

	// Copies a property from this property list or class to another
	void copyProp( PropList& dest, const char* name) const;
	void copyProp( PropList& dest, const H5std_string& name) const;

	// Copies a property from one property list or property class to another
	void copyProp( PropList& dest, PropList& src, const char* name) const;
	void copyProp( PropList& dest, PropList& src, const H5std_string& name) const;

	// Gets the class of this property list, i.e. H5P_FILE_CREATE,
	// H5P_FILE_ACCESS, ...
	hid_t getClass() const;

	// Return the name of a generic property list class.
	H5std_string getClassName() const;

	// Returns the parent class of a generic property class.
	PropList getClassParent() const;

	// Returns the number of properties in this property list or class.
	size_t getNumProps() const;

	// Query the value of a property in a property list.
	void getProperty(const char* name, void* value) const;
	void getProperty(const H5std_string& name, void* value) const;
	H5std_string getProperty(const char* name) const;
	H5std_string getProperty(const H5std_string& name) const;

	// Set a property's value in a property list.
	void setProperty(const char* name, void* value) const;
	void setProperty(const char* name, const char* charptr) const;
	void setProperty(const char* name, H5std_string& strg) const;
	void setProperty(const H5std_string& name, void* value) const;
	void setProperty(const H5std_string& name, H5std_string& strg) const;

	// Query the size of a property in a property list or class.
	size_t getPropSize(const char *name) const;
	size_t getPropSize(const H5std_string& name) const;

	// Determines whether a property list is a certain class.
	bool isAClass(const PropList& prop_class) const;

	/// Query the existance of a property in a property object.
	bool propExist(const char* name) const;
	bool propExist(const H5std_string& name) const;

	// Removes a property from a property list.
	void removeProp(const char *name) const;
	void removeProp(const H5std_string& name) const;

	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("PropList"); }

	// Default constructor: creates a stub PropList object.
	PropList();

	// Copy constructor: creates a copy of a PropList object.
	PropList(const PropList& original);

	// Gets the property list id.
	virtual hid_t getId() const;

	// Destructor: properly terminates access to this property list.
	virtual ~PropList();

   protected:
	hid_t id;	// HDF5 property list id

	// Sets the property list id.
	virtual void p_setId(const hid_t new_id);
};

#ifndef H5_NO_NAMESPACE
}
#endif
#endif  // _H5PropList_H
