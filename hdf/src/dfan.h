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

/*------------------------------------------------------------------------------
 * File:    dfan.h
 * Purpose: header file for the Annotations set
 * Invokes: df.h
 * Contents:
 *  Structure definitions: DFANdirentry, DFANdirhead
 *  Constant definitions: DFAN_LABEL, DFAN_DESC
 *----------------------------------------------------------------------------*/

#ifndef H4_DFAN_H
#define H4_DFAN_H

#include "hdfi.h"

#define DFAN_LABEL 0
#define DFAN_DESC  1

#define DFAN_LAB_BLKSIZE  64  /* default blksize to use for labels */
#define DFAN_DESC_BLKSIZE 512 /* default blksize to use for descriptions */

#define DFAN_DEFENTRIES 16 /* no of dir entries to add at a time */

/*
 * This structure stores an entry in the label/desc directory
 * for a label/desc in the file, it gives the ref of the label/desc,
 * and the tag/ref of the data item to which the label/desc relates
 */
typedef struct {
    uint16 annref;  /* ref of annotation */
    uint16 datatag; /* tag of data */
    uint16 dataref; /* ref of data */
} DFANdirentry;

/*
 * This structure is a head node for the directory, which is organized as
 * as a linked list of arrays.  DFANdirentry is the structure of an
 * array element, while DFANdirhead is the list element
 */
typedef struct DFANdirhead {
    struct DFANdirhead *next;     /* list element */
    int32               nentries; /* Number of entries */
    DFANdirentry       *entries;  /* actually an arbitrary size array */
} DFANdirhead;

#endif /* H4_DFAN_H */
