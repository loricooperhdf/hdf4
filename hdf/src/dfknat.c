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

/*------------------------------------------------------------------
 File:  dfknat.c

 Purpose:
    Routines to support "native mode" conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions:
    DFKnb1b -  Native mode for 8 bit integers
    DFKnb2b -  Native mode for 16 bit integers
    DFKnb4b -  Native mode for 32 bit integers and floats
    DFKnb8b -  Native mode for 64 bit floats

 Remarks:
    These files used to be in dfconv.c, but it got a little too huge,
    so they were broken into a separate file.

 *------------------------------------------------------------------*/

/*****************************************************************************/
/*                                                                           */
/*    All the routines in this file marked as PRIVATE have been marked so    */
/*  for a reason.  *ANY* of these routines may or may nor be supported in    */
/*  the next version of HDF (4.00).  Furthurmore, the names, parameters, or   */
/*  functionality is *NOT* guaranteed to remain the same.                    */
/*    The *ONLY* guarantee possible is that DFKnumin(), and DFKnumout()      */
/*  will not change.  They are *NOT* guaranteed to be implemented in the     */
/*  next version of HDF as function pointers.  They are guaranteed to take   */
/*  the same arguments and produce the same results.                         */
/*    If your programs call any routines in this file except for             */
/*  DFKnumin(), DFKnumout, and/or DFKsetntype(), your code may not work      */
/*  with future versions of HDF and your code will *NOT* be portable.        */
/*                                                                           */
/*****************************************************************************/

#include "hdfi.h"
#include "hconv.h"

/*****************************************************************************/
/* NATIVE MODE NUMBER "CONVERSION" ROUTINES                                  */
/*****************************************************************************/

/************************************************************/
/* DFKnb1b()                                                */
/*   Native mode for 1 byte data items                      */
/************************************************************/
int
DFKnb1b(void *s, void *d, uint32 num_elm, uint32 source_stride, uint32 dest_stride)
{
    int    fast_processing = 0;
    int    in_place        = 0;
    uint32 i;
    uint8 *source = (uint8 *)s;
    uint8 *dest   = (uint8 *)d;

    HEclear();

    if (num_elm == 0) {
        HERROR(DFE_BADCONV);
        return FAIL;
    }

    /* Determine if faster array processing is appropriate */
    if ((source_stride == 0 && dest_stride == 0) || (source_stride == 1 && dest_stride == 1))
        fast_processing = 1;

    /* Determine if the conversion should be inplace */
    if (source == dest)
        in_place = 1;

    if (fast_processing) {
        if (!in_place) {
            memcpy(dest, source, num_elm);
            return 0;
        }
        else
            return 0; /* Nothing to do */
    }
    else {
        *dest = *source;
        for (i = 1; i < num_elm; i++) {
            dest += dest_stride;
            source += source_stride;
            *dest = *source;
        }
    }

    return 0;
}

/************************************************************/
/* DFKnb2b()                                                */
/* -->Native mode for 2 byte data items                     */
/************************************************************/
int
DFKnb2b(void *s, void *d, uint32 num_elm, uint32 source_stride, uint32 dest_stride)
{
    int    fast_processing = 0; /* Default is not fast processing */
    int    in_place        = 0; /* Inplace must be detected */
    uint32 i;
    uint8  buf[2]; /* Inplace processing buffer */
    uint8 *source = (uint8 *)s;
    uint8 *dest   = (uint8 *)d;

    HEclear();

    if (num_elm == 0) {
        HERROR(DFE_BADCONV);
        return FAIL;
    }

    /* Determine if faster array processing is appropriate */
    if ((source_stride == 0 && dest_stride == 0) || (source_stride == 2 && dest_stride == 2))
        fast_processing = 1;

    /* Determine if the conversion should be inplace */
    if (source == dest)
        in_place = 1;

    if (fast_processing) {
        if (!in_place) {
            memcpy(dest, source, num_elm * 2);
            return 0;
        }
        else { /* Nothing to do */
            return 0;
        }
    }

    /* Generic stride processing */
    if (!in_place)
        for (i = 0; i < num_elm; i++) {
            dest[0] = source[0];
            dest[1] = source[1];
            dest += dest_stride;
            source += source_stride;
        }
    else
        for (i = 0; i < num_elm; i++) {
            buf[0]  = source[0];
            buf[1]  = source[1];
            dest[0] = buf[0];
            dest[1] = buf[1];
            dest += dest_stride;
            source += source_stride;
        }

    return 0;
}

/************************************************************/
/* DFKnb4b()                                                */
/* -->Native mode for 4 byte items                          */
/************************************************************/
int
DFKnb4b(void *s, void *d, uint32 num_elm, uint32 source_stride, uint32 dest_stride)
{
    int    fast_processing = 0; /* Default is not fast processing */
    int    in_place        = 0; /* Inplace must be detected */
    uint32 i;
    uint8  buf[4]; /* Inplace processing buffer */
    uint8 *source = (uint8 *)s;
    uint8 *dest   = (uint8 *)d;

    HEclear();

    if (num_elm == 0) {
        HERROR(DFE_BADCONV);
        return FAIL;
    }

    /* Determine if faster array processing is appropriate */
    if ((source_stride == 0 && dest_stride == 0) || (source_stride == 4 && dest_stride == 4))
        fast_processing = 1;

    /* Determine if the conversion should be inplace */
    if (source == dest)
        in_place = 1;

    if (fast_processing) {
        if (!in_place) {
            memcpy(dest, source, num_elm * 4);
            return 0;
        }
        else { /* Nothing to do */
            return 0;
        }
    }

    /* Generic stride processing */
    if (!in_place)
        for (i = 0; i < num_elm; i++) {
            dest[0] = source[0];
            dest[1] = source[1];
            dest[2] = source[2];
            dest[3] = source[3];
            dest += dest_stride;
            source += source_stride;
        }
    else
        for (i = 0; i < num_elm; i++) {
            buf[0]  = source[0];
            buf[1]  = source[1];
            buf[2]  = source[2];
            buf[3]  = source[3];
            dest[0] = buf[0];
            dest[1] = buf[1];
            dest[2] = buf[2];
            dest[3] = buf[3];
            dest += dest_stride;
            source += source_stride;
        }

    return 0;
}

/************************************************************/
/* DFKnb8b()                                                */
/* -->Native mode for 8 byte items                          */
/************************************************************/
int
DFKnb8b(void *s, void *d, uint32 num_elm, uint32 source_stride, uint32 dest_stride)
{
    int    fast_processing = 0; /* Default is not fast processing */
    int    in_place        = 0; /* Inplace must be detected */
    uint32 i;
    uint8  buf[8]; /* Inplace processing buffer */
    uint8 *source = (uint8 *)s;
    uint8 *dest   = (uint8 *)d;

    HEclear();

    if (num_elm == 0) {
        HERROR(DFE_BADCONV);
        return FAIL;
    }

    /* Determine if faster array processing is appropriate */
    if ((source_stride == 0 && dest_stride == 0) || (source_stride == 8 && dest_stride == 8))
        fast_processing = 1;

    /* Determine if the conversion should be inplace */
    if (source == dest)
        in_place = 1;

    if (fast_processing) {
        if (!in_place) {
            memcpy(dest, source, num_elm * 8);
            return 0;
        }
        else {
            return 0; /* No work to do ! */
        }
    }

    /* Generic stride processing */
    if (!in_place)
        for (i = 0; i < num_elm; i++) {
            memcpy(dest, source, 8);
            dest += dest_stride;
            source += source_stride;
        }
    else
        for (i = 0; i < num_elm; i++) {
            memcpy(buf, source, 8);
            memcpy(dest, buf, 8);
            dest += dest_stride;
            source += source_stride;
        }

    return 0;
}
