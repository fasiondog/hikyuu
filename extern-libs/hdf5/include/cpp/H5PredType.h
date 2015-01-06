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

// PredType holds the definition of all the HDF5 predefined datatypes.
// These types can only be made copy of, not created by H5Tcreate or
// closed by H5Tclose.  They are treated as constants.
/////////////////////////////////////////////////////////////////////

#ifndef _H5PredType_H
#define _H5PredType_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

/* This constant is defined for a workaround to eliminate memory leaks due to
   the library being re-initiated when PredType destructors are invoked.  A
   PredType instant with H5CPP_EXITED as the value of its "id" is constructed
   before the other PredType objects are created.  At exit, when this special
   PredType object is to be destructed, no HDF5 library function will be called
   and the library will be terminated.  -BMR, Mar 30, 2012 */
#define H5CPP_EXITED	-3  // -3 is less likely to be used elsewhere

class H5_DLLCPP PredType : public AtomType {
   public:
	///\brief Returns this class name
	virtual H5std_string fromClass () const { return("PredType"); }

	// Makes a copy of the predefined type and stores the new
	// id in the left hand side object.
	PredType& operator=( const PredType& rhs );

	// Copy constructor - makes copy of the original object
	PredType( const PredType& original );

	// Noop destructor
	virtual ~PredType();

	// Declaration of predefined types; their definition is in H5PredType.cpp
	static const PredType STD_I8BE;
	static const PredType STD_I8LE;
	static const PredType STD_I16BE;
	static const PredType STD_I16LE;
	static const PredType STD_I32BE;
	static const PredType STD_I32LE;
	static const PredType STD_I64BE;
	static const PredType STD_I64LE;
	static const PredType STD_U8BE;
	static const PredType STD_U8LE;
	static const PredType STD_U16BE;
	static const PredType STD_U16LE;
	static const PredType STD_U32BE;
	static const PredType STD_U32LE;
	static const PredType STD_U64BE;
	static const PredType STD_U64LE;
	static const PredType STD_B8BE;
	static const PredType STD_B8LE;
	static const PredType STD_B16BE;
	static const PredType STD_B16LE;
	static const PredType STD_B32BE;
	static const PredType STD_B32LE;
	static const PredType STD_B64BE;
	static const PredType STD_B64LE;
	static const PredType STD_REF_OBJ;
	static const PredType STD_REF_DSETREG;

	static const PredType C_S1;
	static const PredType FORTRAN_S1;

	static const PredType IEEE_F32BE;
	static const PredType IEEE_F32LE;
	static const PredType IEEE_F64BE;
	static const PredType IEEE_F64LE;

	static const PredType UNIX_D32BE;
	static const PredType UNIX_D32LE;
	static const PredType UNIX_D64BE;
	static const PredType UNIX_D64LE;

	static const PredType INTEL_I8;
	static const PredType INTEL_I16;
	static const PredType INTEL_I32;
	static const PredType INTEL_I64;
	static const PredType INTEL_U8;
	static const PredType INTEL_U16;
	static const PredType INTEL_U32;
	static const PredType INTEL_U64;
	static const PredType INTEL_B8;
	static const PredType INTEL_B16;
	static const PredType INTEL_B32;
	static const PredType INTEL_B64;
	static const PredType INTEL_F32;
	static const PredType INTEL_F64;

	static const PredType ALPHA_I8;
	static const PredType ALPHA_I16;
	static const PredType ALPHA_I32;
	static const PredType ALPHA_I64;
	static const PredType ALPHA_U8;
	static const PredType ALPHA_U16;
	static const PredType ALPHA_U32;
	static const PredType ALPHA_U64;
	static const PredType ALPHA_B8;
	static const PredType ALPHA_B16;
	static const PredType ALPHA_B32;
	static const PredType ALPHA_B64;
	static const PredType ALPHA_F32;
	static const PredType ALPHA_F64;

	static const PredType MIPS_I8;
	static const PredType MIPS_I16;
	static const PredType MIPS_I32;
	static const PredType MIPS_I64;
	static const PredType MIPS_U8;
	static const PredType MIPS_U16;
	static const PredType MIPS_U32;
	static const PredType MIPS_U64;
	static const PredType MIPS_B8;
	static const PredType MIPS_B16;
	static const PredType MIPS_B32;
	static const PredType MIPS_B64;
	static const PredType MIPS_F32;
	static const PredType MIPS_F64;

	static const PredType NATIVE_CHAR;
	static const PredType NATIVE_SCHAR;
	static const PredType NATIVE_UCHAR;
	static const PredType NATIVE_SHORT;
	static const PredType NATIVE_USHORT;
	static const PredType NATIVE_INT;
	static const PredType NATIVE_UINT;
	static const PredType NATIVE_LONG;
	static const PredType NATIVE_ULONG;
	static const PredType NATIVE_LLONG;
	static const PredType NATIVE_ULLONG;
	static const PredType NATIVE_FLOAT;
	static const PredType NATIVE_DOUBLE;
	static const PredType NATIVE_LDOUBLE;
	static const PredType NATIVE_B8;
	static const PredType NATIVE_B16;
	static const PredType NATIVE_B32;
	static const PredType NATIVE_B64;
	static const PredType NATIVE_OPAQUE;
	static const PredType NATIVE_HSIZE;
	static const PredType NATIVE_HSSIZE;
	static const PredType NATIVE_HERR;
	static const PredType NATIVE_HBOOL;

	static const PredType NATIVE_INT8;
	static const PredType NATIVE_UINT8;
	static const PredType NATIVE_INT16;
	static const PredType NATIVE_UINT16;
	static const PredType NATIVE_INT32;
	static const PredType NATIVE_UINT32;
	static const PredType NATIVE_INT64;
	static const PredType NATIVE_UINT64;

// LEAST types
#if H5_SIZEOF_INT_LEAST8_T != 0
	static const PredType NATIVE_INT_LEAST8;
#endif /* H5_SIZEOF_INT_LEAST8_T */
#if H5_SIZEOF_UINT_LEAST8_T != 0
	static const PredType NATIVE_UINT_LEAST8;
#endif /* H5_SIZEOF_UINT_LEAST8_T */

#if H5_SIZEOF_INT_LEAST16_T != 0
	static const PredType NATIVE_INT_LEAST16;
#endif /* H5_SIZEOF_INT_LEAST16_T */
#if H5_SIZEOF_UINT_LEAST16_T != 0
	static const PredType NATIVE_UINT_LEAST16;
#endif /* H5_SIZEOF_UINT_LEAST16_T */

#if H5_SIZEOF_INT_LEAST32_T != 0
	static const PredType NATIVE_INT_LEAST32;
#endif /* H5_SIZEOF_INT_LEAST32_T */
#if H5_SIZEOF_UINT_LEAST32_T != 0
	static const PredType NATIVE_UINT_LEAST32;
#endif /* H5_SIZEOF_UINT_LEAST32_T */

#if H5_SIZEOF_INT_LEAST64_T != 0
	static const PredType NATIVE_INT_LEAST64;
#endif /* H5_SIZEOF_INT_LEAST64_T */
#if H5_SIZEOF_UINT_LEAST64_T != 0
	static const PredType NATIVE_UINT_LEAST64;
#endif /* H5_SIZEOF_UINT_LEAST64_T */

// FAST types
#if H5_SIZEOF_INT_FAST8_T != 0
	static const PredType NATIVE_INT_FAST8;
#endif /* H5_SIZEOF_INT_FAST8_T */
#if H5_SIZEOF_UINT_FAST8_T != 0
	static const PredType NATIVE_UINT_FAST8;
#endif /* H5_SIZEOF_UINT_FAST8_T */

#if H5_SIZEOF_INT_FAST16_T != 0
	static const PredType NATIVE_INT_FAST16;
#endif /* H5_SIZEOF_INT_FAST16_T */
#if H5_SIZEOF_UINT_FAST16_T != 0
	static const PredType NATIVE_UINT_FAST16;
#endif /* H5_SIZEOF_UINT_FAST16_T */

#if H5_SIZEOF_INT_FAST32_T != 0
	static const PredType NATIVE_INT_FAST32;
#endif /* H5_SIZEOF_INT_FAST32_T */
#if H5_SIZEOF_UINT_FAST32_T != 0
	static const PredType NATIVE_UINT_FAST32;
#endif /* H5_SIZEOF_UINT_FAST32_T */

#if H5_SIZEOF_INT_FAST64_T != 0
	static const PredType NATIVE_INT_FAST64;
#endif /* H5_SIZEOF_INT_FAST64_T */
#if H5_SIZEOF_UINT_FAST64_T != 0
	static const PredType NATIVE_UINT_FAST64;
#endif /* H5_SIZEOF_UINT_FAST64_T */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// These dummy functions do not inherit from DataType - they'll
	// throw a DataTypeIException if invoked.
	void commit( H5File& loc, const H5std_string& name );
	void commit( H5File& loc, const char* name );
	void commit( H5Object& loc, const H5std_string& name );
	void commit( H5Object& loc, const char* name );
	bool committed();
#endif // DOXYGEN_SHOULD_SKIP_THIS

   private:
	// Added this to work around the atexit/global destructor problem.
	// It'll help to terminate the library after other PredType instances
	// are closed.  -BMR, Mar 30, 2012
	static const PredType AtExit;

   protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// Default constructor
	PredType();

	// Creates a pre-defined type using an HDF5 pre-defined constant
	PredType( const hid_t predtype_id );  // used by the library only

#endif // DOXYGEN_SHOULD_SKIP_THIS

};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
