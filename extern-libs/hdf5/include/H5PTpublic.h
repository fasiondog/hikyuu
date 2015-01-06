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

#ifndef _H5PTpublic_H
#define _H5PTpublic_H


#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------
 *
 * Create/Open/Close functions
 *
 *-------------------------------------------------------------------------
 */

H5_HLDLL hid_t H5PTcreate_fl ( hid_t loc_id,
                      const char *dset_name,
                      hid_t dtype_id,
                      hsize_t chunk_size,
                      int compression );

#ifdef VLPT_REMOVED
H5_HLDLL hid_t H5PTcreate_vl ( hid_t loc_id,
                      const char *dset_name,
                      hsize_t chunk_size );
#endif /* VLPT_REMOVED */

H5_HLDLL hid_t H5PTopen( hid_t loc_id,
                const char *dset_name );

H5_HLDLL herr_t  H5PTclose( hid_t table_id );


/*-------------------------------------------------------------------------
 *
 * Write functions
 *
 *-------------------------------------------------------------------------
 */

H5_HLDLL herr_t  H5PTappend( hid_t table_id,
                   size_t nrecords,
                   const void * data );

/*-------------------------------------------------------------------------
 *
 * Read functions
 *
 *-------------------------------------------------------------------------
 */


H5_HLDLL herr_t  H5PTget_next( hid_t table_id,
                     size_t nrecords,
                     void * data );

H5_HLDLL herr_t  H5PTread_packets( hid_t table_id,
                         hsize_t start,
                         size_t nrecords,
                         void *data );

/*-------------------------------------------------------------------------
 *
 * Inquiry functions
 *
 *-------------------------------------------------------------------------
 */


H5_HLDLL herr_t  H5PTget_num_packets( hid_t table_id,
                            hsize_t *nrecords );

H5_HLDLL herr_t  H5PTis_valid( hid_t table_id );

#ifdef VLPT_REMOVED
H5_HLDLL herr_t  H5PTis_varlen( hid_t table_id );
#endif /* VLPT_REMOVED */

/*-------------------------------------------------------------------------
 *
 * Packet Table "current index" functions
 *
 *-------------------------------------------------------------------------
 */

H5_HLDLL herr_t  H5PTcreate_index( hid_t table_id );

H5_HLDLL herr_t  H5PTset_index( hid_t table_id,
                             hsize_t pt_index );

H5_HLDLL herr_t  H5PTget_index( hid_t table_id,
                             hsize_t *pt_index );

/*-------------------------------------------------------------------------
 *
 * Memory Management functions
 *
 *-------------------------------------------------------------------------
 */

#ifdef VLPT_REMOVED
H5_HLDLL herr_t  H5PTfree_vlen_readbuff( hid_t table_id,
                               size_t bufflen,
                               void * buff );
#endif /* VLPT_REMOVED */

#ifdef __cplusplus
}
#endif

#endif

