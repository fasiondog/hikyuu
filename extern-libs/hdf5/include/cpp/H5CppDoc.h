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

#ifndef _H5CPPDOC_H
#define _H5CPPDOC_H

//-------------------------------------------------------------------------
// The following section will be used to generate the 'Mainpage'
// and the 'Examples' for the RM.
// ------------------------------------------------------------------------

/*! \mainpage notitle
 *
 * \section intro_sec Introduction
 *
 * The C++ API provides C++ wrappers for the HDF5 C library.
 * It is assumed that the user has knowledge of the HDF5 file format
 * and its components.  If you are not familiar with HDF5 file format,
 * and would like to find out more, please refer to the HDF5 documentation
 * at http://www.hdfgroup.org/HDF5/doc/index.html
 *
 * Because the HDF5 library maps very well to
 * the object oriented design approach, classes in the C++ API can
 * closely represent the interfaces of the HDF5 APIs, as followed:
 *
 * \verbatim
 	HDF5 C APIs				C++ Classes
 	-----------				-----------
 	Attribute Interface (H5A)		Attribute
 	Datasets Interface (H5D)		DataSet
 	Error Interface (H5E)			Exception
 	File Interface (H5F)			H5File
 	Group Interface (H5G)			Group
 	Identifier Interface (H5I)		IdComponent
 	Property List Interface (H5P)		PropList and subclasses
 	Dataspace Interface (H5S)		DataSpace
 	Datatype Interface (H5T)		DataType and subclasses
  \endverbatim
 * \section install_sec Installation
 *
 * Please refer to the file release_docs/INSTALL_Windows.txt
 * under the top directory for information about installing, building,
 * and testing the C++ API.
 *
 *
 */

///	This example shows how to create datasets.
///\par
///\example     create.cpp

///\par
///	This example shows how to write datasets.
///\example     writedata.cpp

///\par
///	This example shows how to read datasets.
///\example     readdata.cpp

///\par
///	This example shows how to create a compound datatype,
///	write an array which has the compound datatype to the file,
///	and read back fields' subsets.
///\example     compound.cpp

///\par
///	This example shows how to work with extendible datasets.
///\example     extend_ds.cpp

///\par
///	This example shows how to read data from a chunked dataset.
///\example     chunks.cpp

///\par
///	This example shows how to work with groups.
///\example     h5group.cpp

#endif
