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

#ifndef _H5Library_H
#define _H5Library_H

#ifndef H5_NO_NAMESPACE
namespace H5 {
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define NOTATEXIT       (-10)   // just in case the HDF5 library use more
	// negative constants. Note: the solution used for the atexit/global
	// destructors is not reliable, and desperately needs improvement
	// It is not even working, inifiteloop message still printed when
	// calling H5close
#endif // DOXYGEN_SHOULD_SKIP_THIS

class H5_DLLCPP H5Library {
   public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	static bool need_cleanup; // indicates if H5close should be called
#endif // DOXYGEN_SHOULD_SKIP_THIS

	// Initializes the HDF5 library.
	static void open();

	// Flushes all data to disk, closes files, and cleans up memory.
	static void close();

	// Instructs library not to install atexit cleanup routine
	static void dontAtExit();

	// Returns the HDF library release number.
	static void getLibVersion( unsigned& majnum, unsigned& minnum, unsigned& relnum );

	// Verifies that the arguments match the version numbers compiled
	// into the library
	static void checkVersion( unsigned majnum, unsigned minnum, unsigned relnum );

	// Walks through all the garbage collection routines for the library,
	// which are supposed to free any unused memory they have allocated.
	static void garbageCollect();

	// Sets limits on the different kinds of free lists.
	static void setFreeListLimits(int reg_global_lim, int reg_list_lim, int
	arr_global_lim, int arr_list_lim, int blk_global_lim, int blk_list_lim);

   private:
	// Default constructor - no instance ever created
	H5Library() {};

};
#ifndef H5_NO_NAMESPACE
}
#endif
#endif
