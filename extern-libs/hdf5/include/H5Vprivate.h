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

/*
 * Programmer: Robb Matzke <matzke@llnl.gov>
 *             Friday, October 10, 1997
 */
#ifndef H5Vprivate_H
#define H5Vprivate_H

/* Private headers needed by this file */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/

/* Vector-Vector sequence operation callback */
typedef herr_t (*H5V_opvv_func_t)(hsize_t dst_off, hsize_t src_off,
    size_t len, void *udata);

/* Vector comparison functions like Fortran66 comparison operators */
#define H5V_vector_eq_s(N,V1,V2) (H5V_vector_cmp_s (N, V1, V2)==0)
#define H5V_vector_lt_s(N,V1,V2) (H5V_vector_cmp_s (N, V1, V2)<0)
#define H5V_vector_gt_s(N,V1,V2) (H5V_vector_cmp_s (N, V1, V2)>0)
#define H5V_vector_le_s(N,V1,V2) (H5V_vector_cmp_s (N, V1, V2)<=0)
#define H5V_vector_ge_s(N,V1,V2) (H5V_vector_cmp_s (N, V1, V2)>=0)
#define H5V_vector_eq_u(N,V1,V2) (H5V_vector_cmp_u (N, V1, V2)==0)
#define H5V_vector_lt_u(N,V1,V2) (H5V_vector_cmp_u (N, V1, V2)<0)
#define H5V_vector_gt_u(N,V1,V2) (H5V_vector_cmp_u (N, V1, V2)>0)
#define H5V_vector_le_u(N,V1,V2) (H5V_vector_cmp_u (N, V1, V2)<=0)
#define H5V_vector_ge_u(N,V1,V2) (H5V_vector_cmp_u (N, V1, V2)>=0)

/* Other functions */
#define H5V_vector_cpy(N,DST,SRC) {                                           \
    assert (sizeof(*(DST))==sizeof(*(SRC)));				                  \
    if (SRC) HDmemcpy (DST, SRC, (N)*sizeof(*(DST)));                         \
    else HDmemset (DST, 0, (N)*sizeof(*(DST)));                               \
}

#define H5V_vector_zero(N,DST) HDmemset(DST,0,(N)*sizeof(*(DST)))

/* A null pointer is equivalent to a zero vector */
#define H5V_ZERO        NULL

H5_DLL hsize_t H5V_hyper_stride(unsigned n, const hsize_t *size,
				 const hsize_t *total_size,
				 const hsize_t *offset,
				 hsize_t *stride);
H5_DLL htri_t H5V_hyper_disjointp(unsigned n, const hsize_t *offset1,
    const uint32_t *size1, const hsize_t *offset2, const uint32_t *size2);
H5_DLL htri_t H5V_hyper_eq(unsigned n, const hsize_t *offset1,
			    const hsize_t *size1, const hsize_t *offset2,
			    const hsize_t *size2);
H5_DLL herr_t H5V_hyper_fill(unsigned n, const hsize_t *_size,
			      const hsize_t *total_size,
			      const hsize_t *offset, void *_dst,
			      unsigned fill_value);
H5_DLL herr_t H5V_hyper_copy(unsigned n, const hsize_t *size,
			      const hsize_t *dst_total_size,
			      const hsize_t *dst_offset, void *_dst,
			      const hsize_t *src_total_size,
			      const hsize_t *src_offset, const void *_src);
H5_DLL herr_t H5V_stride_fill(unsigned n, hsize_t elmt_size, const hsize_t *size,
			       const hsize_t *stride, void *_dst,
			       unsigned fill_value);
H5_DLL herr_t H5V_stride_copy(unsigned n, hsize_t elmt_size, const hsize_t *_size,
			       const hsize_t *dst_stride, void *_dst,
			       const hsize_t *src_stride, const void *_src);
H5_DLL herr_t H5V_stride_copy_s(unsigned n, hsize_t elmt_size, const hsize_t *_size,
			       const hssize_t *dst_stride, void *_dst,
			       const hssize_t *src_stride, const void *_src);
H5_DLL herr_t H5V_array_fill(void *_dst, const void *src, size_t size,
			      size_t count);
H5_DLL herr_t H5V_array_down(unsigned n, const hsize_t *total_size,
    hsize_t *down);
H5_DLL hsize_t H5V_array_offset_pre(unsigned n,
    const hsize_t *acc, const hsize_t *offset);
H5_DLL hsize_t H5V_array_offset(unsigned n, const hsize_t *total_size,
    const hsize_t *offset);
H5_DLL herr_t H5V_array_calc(hsize_t offset, unsigned n,
    const hsize_t *total_size, hsize_t *coords);
H5_DLL herr_t H5V_chunk_index(unsigned ndims, const hsize_t *coord,
    const uint32_t *chunk, const hsize_t *down_nchunks, hsize_t *chunk_idx);
H5_DLL ssize_t H5V_opvv(size_t dst_max_nseq, size_t *dst_curr_seq, size_t dst_len_arr[],
    hsize_t dst_off_arr[],
    size_t src_max_nseq, size_t *src_curr_seq, size_t src_len_arr[],
    hsize_t src_off_arr[],
    H5V_opvv_func_t op, void *op_data);
H5_DLL ssize_t H5V_memcpyvv(void *_dst,
    size_t dst_max_nseq, size_t *dst_curr_seq, size_t dst_len_arr[], hsize_t dst_off_arr[],
    const void *_src,
    size_t src_max_nseq, size_t *src_curr_seq, size_t src_len_arr[], hsize_t src_off_arr[]);


/*-------------------------------------------------------------------------
 * Function:    H5V_vector_reduce_product
 *
 * Purpose:     Product reduction of a vector.  Vector elements and return
 *              value are size_t because we usually want the number of
 *              elements in an array and array dimensions are always of type
 *              size_t.
 *
 * Return:      Success:        Product of elements
 *
 *              Failure:        1 if N is zero
 *
 * Programmer:  Robb Matzke
 *              Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline hsize_t UNUSED
H5V_vector_reduce_product(unsigned n, const hsize_t *v)
{
    hsize_t                  ret_value = 1;

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOERR here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (n && !v) HGOTO_DONE(0)
    while (n--) ret_value *= *v++;

done:
    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5V_vector_zerop_u
 *
 * Purpose:     Determines if all elements of a vector are zero.
 *
 * Return:      Success:        TRUE if all elements are zero,
 *                              FALSE otherwise
 *
 *              Failure:        TRUE if N is zero
 *
 * Programmer:  Robb Matzke
 *              Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline htri_t UNUSED
H5V_vector_zerop_u(int n, const hsize_t *v)
{
    htri_t      ret_value=TRUE;       /* Return value */

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOERR here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (!v)
        HGOTO_DONE(TRUE)
    while (n--)
	if (*v++)
            HGOTO_DONE(FALSE)

done:
    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5V_vector_zerop_s
 *
 * Purpose:     Determines if all elements of a vector are zero.
 *
 * Return:      Success:        TRUE if all elements are zero,
 *                              FALSE otherwise
 *
 *              Failure:        TRUE if N is zero
 *
 * Programmer:  Robb Matzke
 *              Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline htri_t UNUSED
H5V_vector_zerop_s(int n, const hssize_t *v)
{
    htri_t      ret_value=TRUE;       /* Return value */

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOERR here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (!v)
        HGOTO_DONE(TRUE)
    while (n--)
	if (*v++)
            HGOTO_DONE(FALSE)

done:
    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5V_vector_cmp_u
 *
 * Purpose:     Compares two vectors of the same size and determines if V1 is
 *              lexicographically less than, equal, or greater than V2.
 *
 * Return:      Success:        -1 if V1 is less than V2
 *                              0 if they are equal
 *                              1 if V1 is greater than V2
 *
 *              Failure:        0 if N is zero
 *
 * Programmer:  Robb Matzke
 *              Friday, October 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline int UNUSED
H5V_vector_cmp_u (unsigned n, const hsize_t *v1, const hsize_t *v2)
{
    int ret_value=0;    /* Return value */

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOERR here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (v1 == v2) HGOTO_DONE(0)
    if (v1 == NULL) HGOTO_DONE(-1)
    if (v2 == NULL) HGOTO_DONE(1)
    while (n--) {
        if (*v1 < *v2) HGOTO_DONE(-1)
        if (*v1 > *v2) HGOTO_DONE(1)
        v1++;
        v2++;
    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:	H5V_vector_cmp_s
 *
 * Purpose:     Compares two vectors of the same size and determines if V1 is
 *              lexicographically less than, equal, or greater than V2.
 *
 * Return:      Success:        -1 if V1 is less than V2
 *                              0 if they are equal
 *                              1 if V1 is greater than V2
 *
 *              Failure:        0 if N is zero
 *
 * Programmer:	Robb Matzke
 *              Wednesday, April  8, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline int UNUSED
H5V_vector_cmp_s (unsigned n, const hssize_t *v1, const hssize_t *v2)
{
    int ret_value=0;    /* Return value */

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOERR here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (v1 == v2) HGOTO_DONE(0)
    if (v1 == NULL) HGOTO_DONE(-1)
    if (v2 == NULL) HGOTO_DONE(1)
    while (n--) {
        if (*v1 < *v2) HGOTO_DONE(-1)
        if (*v1 > *v2) HGOTO_DONE(1)
        v1++;
        v2++;
    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
}


/*-------------------------------------------------------------------------
 * Function:    H5V_vector_inc
 *
 * Purpose:     Increments V1 by V2
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Monday, October 13, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5_inline void UNUSED
H5V_vector_inc(int n, hsize_t *v1, const hsize_t *v2)
{
    while (n--) *v1++ += *v2++;
}

/* Lookup table for general log2(n) routine */
static const unsigned char LogTable256[] =
{
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};


/*-------------------------------------------------------------------------
 * Function:    H5V_log2_gen
 *
 * Purpose:     Determines the log base two of a number (i.e. log2(n)).
 *              (i.e. the highest bit set in a number)
 *
 * Note:        This is from the "Bit Twiddling Hacks" at:
 *                  http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
 *
 *              The version on the web-site is for 32-bit quantities and this
 *              version has been extended for 64-bit quantities.
 *
 * Return:      log2(n) (always - no failure condition)
 *
 * Programmer:  Quincey Koziol
 *              Monday, March  6, 2006
 *
 *-------------------------------------------------------------------------
 */
static H5_inline unsigned UNUSED
H5V_log2_gen(uint64_t n)
{
    unsigned r;                         /* r will be log2(n) */
    register unsigned int t, tt, ttt;   /* temporaries */

#ifdef H5_BAD_LOG2_CODE_GENERATED
    if(n > (uint64_t)0x7fffffffffffffff)
        r = 63;
    else {
        n &= (uint64_t)0x7fffffffffffffff;
#endif /* H5_BAD_LOG2_CODE_GENERATED */
        if((ttt = (unsigned)(n >> 32)))
            if((tt = (unsigned)(n >> 48)))
                r = (t = (unsigned)(n >> 56)) ? 56 + (unsigned)LogTable256[t] : 48 + (unsigned)LogTable256[tt & 0xFF];
            else
                r = (t = (unsigned)(n >> 40)) ? 40 + (unsigned)LogTable256[t] : 32 + (unsigned)LogTable256[ttt & 0xFF];
        else
            if((tt = (unsigned)(n >> 16)))
                r = (t = (unsigned)(n >> 24)) ? 24 + (unsigned)LogTable256[t] : 16 + (unsigned)LogTable256[tt & 0xFF];
            else
                /* Added 'uint8_t' cast to pacify PGCC compiler */
                r = (t = (unsigned)(n >> 8)) ? 8 + (unsigned)LogTable256[t] : (unsigned)LogTable256[(uint8_t)n];
#ifdef H5_BAD_LOG2_CODE_GENERATED
    } /* end else */
#endif /* H5_BAD_LOG2_CODE_GENERATED  */

    return(r);
} /* H5V_log2_gen() */


/* Lookup table for specialized log2(n) of power of two routine */
static const unsigned MultiplyDeBruijnBitPosition[32] =
{
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};


/*-------------------------------------------------------------------------
 * Function:    H5V_log2_of2
 *
 * Purpose:     Determines the log base two of a number (i.e. log2(n)).
 *              (i.e. the highest bit set in a number)
 *
 * Note:        **N must be a power of two** and is limited to 32-bit quantities.
 *
 *              This is from the "Bit Twiddling Hacks" at:
 *                  http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
 *
 * Return:      log2(n) (always - no failure condition)
 *
 * Programmer:  Quincey Koziol
 *              Monday, Febraury 27, 2006
 *
 *-------------------------------------------------------------------------
 */
static H5_inline unsigned UNUSED
H5V_log2_of2(uint32_t n)
{
#ifndef NDEBUG
    HDassert(POWER_OF_TWO(n));
#endif /* NDEBUG */
    return(MultiplyDeBruijnBitPosition[(n * (uint32_t)0x077CB531UL) >> 27]);
} /* H5V_log2_of2() */


/*-------------------------------------------------------------------------
 * Function:    H5V_limit_enc_size
 *
 * Purpose:     Determine the # of bytes needed to encode values within a
 *              range from 0 to a given limit
 *
 * Return:      Number of bytes needed
 *
 * Programmer:  Quincey Koziol
 *              Thursday, March 13, 2008
 *
 *-------------------------------------------------------------------------
 */
static H5_inline unsigned UNUSED
H5V_limit_enc_size(uint64_t limit)
{
    return (H5V_log2_gen(limit) / 8) + 1;
} /* end H5V_limit_enc_size() */

#endif /* H5Vprivate_H */

