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

#ifndef _H5Exception_H
#define _H5Exception_H

#include <string>

#ifndef H5_NO_NAMESPACE
namespace H5 {
#ifdef H5_NO_STD
    #define H5std_string ::string
#else
    #define H5std_string std::string
#endif
#endif

class H5_DLLCPP Exception {
   public:
	// Creates an exception with a function name where the failure occurs
	// and an optional detailed message
	Exception(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);

	// Returns a character string that describes the error specified by
	// a major error number.
	H5std_string getMajorString( hid_t err_major_id ) const;

	// Returns a character string that describes the error specified by
	// a minor error number.
	H5std_string getMinorString( hid_t err_minor_id ) const;

	// Returns the detailed message set at the time the exception is thrown
	H5std_string getDetailMsg() const;
	const char* getCDetailMsg() const;	// C string of detailed message
	H5std_string getFuncName() const;	// function name as a string object
	const char* getCFuncName() const;	// function name as a char string

	// Turns on the automatic error printing.
	static void setAutoPrint( H5E_auto2_t& func, void* client_data);

	// Turns off the automatic error printing.
	static void dontPrint();

	// Retrieves the current settings for the automatic error stack
	// traversal function and its data.
	static void getAutoPrint( H5E_auto2_t& func, void** client_data);

	// Clears the error stack for the current thread.
	static void clearErrorStack();

	// Walks the error stack for the current thread, calling the
	// specified function.
	static void walkErrorStack( H5E_direction_t direction,
				H5E_walk2_t func, void* client_data);

	// Prints the error stack in a default manner.
	virtual void printError( FILE* stream = NULL ) const;

	// Default constructor
	Exception();

	// copy constructor
	Exception( const Exception& orig);

	// virtual Destructor
	virtual ~Exception();

   private:
	H5std_string detail_message;
	H5std_string func_name;

   protected:
        // Default value for detail_message
        static const char DEFAULT_MSG[];
};

class H5_DLLCPP FileIException : public Exception {
   public:
	FileIException( const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	FileIException();
	virtual ~FileIException();
};

class H5_DLLCPP GroupIException : public Exception {
   public:
	GroupIException( const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	GroupIException();
	virtual ~GroupIException();
};

class H5_DLLCPP DataSpaceIException : public Exception {
   public:
	DataSpaceIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	DataSpaceIException();
	virtual ~DataSpaceIException();
};

class H5_DLLCPP DataTypeIException : public Exception {
   public:
	DataTypeIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	DataTypeIException();
	virtual ~DataTypeIException();
};

class H5_DLLCPP PropListIException : public Exception {
   public:
	PropListIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	PropListIException();
	virtual ~PropListIException();
};

class H5_DLLCPP DataSetIException : public Exception {
   public:
	DataSetIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	DataSetIException();
	virtual ~DataSetIException();
};

class H5_DLLCPP AttributeIException : public Exception {
   public:
	AttributeIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	AttributeIException();
	virtual ~AttributeIException();
};

class H5_DLLCPP ReferenceException : public Exception {
   public:
	ReferenceException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	ReferenceException();
	virtual ~ReferenceException();
};

class H5_DLLCPP LibraryIException : public Exception {
   public:
	LibraryIException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	LibraryIException();
	virtual ~LibraryIException();
};

class H5_DLLCPP IdComponentException : public Exception {
   public:
	IdComponentException(const H5std_string& func_name, const H5std_string& message = DEFAULT_MSG);
	IdComponentException();
	virtual ~IdComponentException();
};

#ifndef H5_NO_NAMESPACE
}
#endif

#endif // _H5Exception_H
