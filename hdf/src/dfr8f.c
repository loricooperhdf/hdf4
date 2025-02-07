/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF/releases/.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-----------------------------------------------------------------------------
 * File:    dfr8F.c
 * Purpose: C stubs for Fortran RIS routines
 * Invokes: dfr8.c dfkit.c
 * Contents:
 *  d8spal:     Set palette to write out with subsequent images
 *  d8first:    Call DFR8restart to reset sequencing to first image
 *  d8igdim:    Call DFR8getdims to get dimensions of next image
 *  d8igimg:    Call DFR8getimage to get next image
 *  d8ipimg:    Call DFR8putimage to write image to new file
 *  d8iaimg:    Call DFR8putimage to add image to existing file
 *  d8irref:    Call DFR8readref to set ref to get next
 *  d8iwref:    Call DFR8writeref to set ref to put next
 *  d8inims:    Call DFR8nimages to get the number of images in the file
 *  d8lref:     Call DFR8lastref to get ref of last image read/written
 *  dfr8lastref:    Call DFR8lastref to get ref of last image read/written
 *  dfr8setpalette: Set palette to write out with subsequent images
 *  dfr8restart:    Call DFR8restart to reset sequencing to first image
 *---------------------------------------------------------------------------*/

#include "hdfi.h"
#include "hproto_fortran.h"

/*-----------------------------------------------------------------------------
 * Name:    d8spal
 * Purpose: Set palette to be written out with subsequent images
 * Inputs:  pal: palette to associate with subsequent images
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8setpalette
 *---------------------------------------------------------------------------*/

intf
nd8spal(_fcd pal)
{
    return DFR8setpalette((uint8 *)_fcdtocp(pal));
}

/*-----------------------------------------------------------------------------
 * Name:    d8first
 * Purpose: Reset sequencing back to first image
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8restart
 *---------------------------------------------------------------------------*/

intf
nd8first(void)
{
    return DFR8restart();
}

/*-----------------------------------------------------------------------------
 * Name:    d8igdim
 * Purpose: Get dimensions of next image using DFR8getdims
 * Inputs:  filename: name of HDF file
 *          xdim, ydim - integers to return dimensions in
 *          ispal - boolean to indicate whether the image includes a palette
 *          lenfn - length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8getdims
 *---------------------------------------------------------------------------*/

intf
nd8igdim(_fcd filename, intf *xdim, intf *ydim, intf *ispal, intf *lenfn)
{
    char *fn;
    intf  ret;
    int32 txdim, tydim;
    intn  tispal;

    fn = HDf2cstring(filename, (intn)*lenfn);
    if (!fn)
        return -1;
    ret = DFR8getdims(fn, &txdim, &tydim, &tispal);
    if (ret != FAIL) {
        *xdim  = txdim;
        *ydim  = tydim;
        *ispal = tispal;
    }
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8igimg
 * Purpose: Get next image using DFR8getimage
 * Inputs:  filename: name of HDF file
 *          image: space provided for returning image
 *          xdim, ydim: dimension of space provided for image
 *          pal: space of 768 bytes for palette
 *          lenfn: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8getimage
 *---------------------------------------------------------------------------*/

intf
nd8igimg(_fcd filename, _fcd image, intf *xdim, intf *ydim, _fcd pal, intf *lenfn)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*lenfn);
    if (!fn)
        return -1;
    ret = DFR8getimage(fn, (uint8 *)_fcdtocp(image), *xdim, *ydim, (uint8 *)_fcdtocp(pal));
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8ipimg
 * Purpose: Write out image to new file
 * Inputs:  filename: name of HDF file
 *          image: image to write out
 *          xdim, ydim: dimensions of image to write out
 *          compress: compression scheme
 *          lenfn: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8putimage
 *---------------------------------------------------------------------------*/

intf
nd8ipimg(_fcd filename, _fcd image, intf *xdim, intf *ydim, intf *compress, intf *lenfn)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*lenfn);
    if (!fn)
        return -1;
    ret = (intf)DFR8putimage(fn, (void *)_fcdtocp(image), (int32)*xdim, (int32)*ydim, (uint16)*compress);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8iaimg
 * Purpose: Add image to existing file
 * Inputs:  filename: name of HDF file
 *          image: image to write out
 *          xdim, ydim: dimensions of image to write out
 *          compress: compression scheme
 *          lenfn: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8addimage
 *---------------------------------------------------------------------------*/

intf
nd8iaimg(_fcd filename, _fcd image, intf *xdim, intf *ydim, intf *compress, intf *lenfn)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*lenfn);
    if (!fn)
        return -1;
    ret = (intf)DFR8addimage(fn, (void *)_fcdtocp(image), (int32)*xdim, (int32)*ydim, (uint16)*compress);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    D8irref
 * Purpose: Set ref of image to get next
 * Inputs:  filename: file to which this applies
 *          ref: reference number of next get
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8Iopen, DFIfind
 * Remarks: checks if image with this ref exists
 *---------------------------------------------------------------------------*/

intf
nd8irref(_fcd filename, intf *ref, intf *fnlen)
{
    char  *fn;
    intf   ret;
    uint16 Ref;

    Ref = (uint16)*ref;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFR8readref(fn, Ref);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8iwref
 * Purpose: Set ref of image to put next
 * Inputs:  filename: file to which this applies
 *          fnlen: length of the filename
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8writeref
 * Remarks:
 *---------------------------------------------------------------------------*/

intf
nd8iwref(_fcd filename, intf *ref, intf *fnlen)
{
    char  *fn;
    intf   ret;
    uint16 Ref;

    Ref = (uint16)*ref;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFR8writeref(fn, Ref);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8inims
 * Purpose: How many images are present in this file?
 * Inputs:  filename: file to which this applies
 *          fnlen: length of HDF file name
 * Returns: number of images on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8nimages
 * Remarks:
 *---------------------------------------------------------------------------*/

intf
nd8inims(_fcd filename, intf *fnlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFR8nimages(fn);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    d8lref
 * Purpose: return reference number of last element read or written
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with error set
 * Users:   Fortran stub routine
 * Invokes: DFR8lastref
 *---------------------------------------------------------------------------*/

intf
nd8lref(void)
{
    return (intf)DFR8lastref();
}

/*-----------------------------------------------------------------------------
 * Name:    d8scomp
 * Purpose: set the compression to use when writing the next image
 * Inputs:
 *      scheme - the type of compression to use
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFR8setcompress
 * Remarks: if the compression scheme is JPEG, this routine sets up default
 *          JPEG parameters to use, if a user wants to change them, d8sjpeg
 *          must be called.
 *---------------------------------------------------------------------------*/

intf
nd8scomp(intf *scheme)
{
    comp_info cinfo; /* Structure containing compression parameters */

    if (*scheme == COMP_JPEG) { /* check for JPEG compression and set defaults */
        cinfo.jpeg.quality        = 75;
        cinfo.jpeg.force_baseline = 1;
    } /* end if */
    return DFR8setcompress((int32)*scheme, &cinfo);
} /* end d8scomp() */

/*-----------------------------------------------------------------------------
 * Name:    d8sjpeg
 * Purpose: change the JPEG compression parameters
 * Inputs:
 *      quality - what the JPEG quality rating should be
 *      force_baseline - whether to force a JPEG baseline file to be written
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFR8setcompress
 * Remarks: none
 *---------------------------------------------------------------------------*/

intf
nd8sjpeg(intf *quality, intf *force_baseline)
{
    comp_info cinfo; /* Structure containing compression parameters */

    cinfo.jpeg.quality        = (intn)*quality;
    cinfo.jpeg.force_baseline = (intn)*force_baseline;
    return DFR8setcompress((int32)COMP_JPEG, &cinfo);
} /* end d8sjpeg() */

/*-----------------------------------------------------------------------------
 * Name:    dfr8lastref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Returns: ref on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8lastref
 * Remarks:
 *---------------------------------------------------------------------------*/

intf
ndfr8lastref(void)
{
    return (intf)DFR8lastref();
}

/*-----------------------------------------------------------------------------
 * Name:    dfr8setpalette
 * Purpose: Set palette to be written out with subsequent images
 * Inputs:  pal: palette to associate with subsequent images
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8setpalette
 *---------------------------------------------------------------------------*/

intf
ndfr8setpalette(_fcd pal)
{
    return DFR8setpalette((uint8 *)_fcdtocp(pal));
}

/*-----------------------------------------------------------------------------
 * Name:    dfr8restart
 * Purpose: Reset sequencing back to first image
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFR8restart
 *---------------------------------------------------------------------------*/

intf
ndfr8restart(void)
{
    return DFR8restart();
}

/*-----------------------------------------------------------------------------
 * Name:    dfr8scompress
 * Purpose: set the compression to use when writing the next image
 * Inputs:
 *      scheme - the type of compression to use
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFR8setcompress
 * Remarks: if the compression scheme is JPEG, this routine sets up default
 *          JPEG parameters to use, if a user wants to change them, dfr8setjpeg
 *          must be called.
 *---------------------------------------------------------------------------*/

intf
ndfr8scompress(intf *scheme)
{
    comp_info cinfo; /* Structure containing compression parameters */

    if (*scheme == COMP_JPEG) { /* check for JPEG compression and set defaults */
        cinfo.jpeg.quality        = 75;
        cinfo.jpeg.force_baseline = 1;
    } /* end if */
    return DFR8setcompress((int32)*scheme, &cinfo);
} /* end dfr8setcompress() */

/*-----------------------------------------------------------------------------
 * Name:    dfr8sjpeg
 * Purpose: change the JPEG compression parameters
 * Inputs:
 *      quality - what the JPEG quality rating should be
 *      force_baseline - whether to force a JPEG baseline file to be written
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFR8setcompress
 * Remarks: none
 *---------------------------------------------------------------------------*/

intf
ndfr8sjpeg(intf *quality, intf *force_baseline)
{
    comp_info cinfo; /* Structure containing compression parameters */

    cinfo.jpeg.quality        = (intn)*quality;
    cinfo.jpeg.force_baseline = (intn)*force_baseline;
    return DFR8setcompress((int32)COMP_JPEG, &cinfo);
} /* end dfr8setjpeg() */
