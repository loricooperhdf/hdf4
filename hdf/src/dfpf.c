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
 * File:    dfpF.c
 * Purpose: C stubs for Palette Fortran routines
 * Invokes: dfp.c dfkit.c
 * Contents:
 *  dpigpal_     : Call DFPgetpal to get palette
 *  dpippal_     : Call DFPputpal to write/overwrite palette in file
 *  dpinpal_     : Call DFPnpals to get number of palettes in file
 *  dpiwref_     : Call DFPwriteref to set ref of pal to write next
 *  dpirref_     : Call DFPreadref to set ref of pal to read next
 *  dprest_      : Call DFPrestart to get palettes afresh in file
 *  dplref_      : Call DFPlastref to get ref of last pal read/written
 *  DFPrestart_  : Call DFPrestart to get palettes afresh in file
 *  DFPlastref_  : Call DFPlastref to get ref of last pal read/written
 * Remarks: none
 *---------------------------------------------------------------------------*/

#include "hdfi.h"
#include "hproto_fortran.h"

/*-----------------------------------------------------------------------------
 * Name:    dpigpal
 * Purpose: call DFPgetpal, get palette
 * Inputs:  filename, fnlen: filename, length of name
 *          pal: space to put palette
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFPgetpal
 *---------------------------------------------------------------------------*/

intf
ndpigpal(_fcd filename, _fcd pal, intf *fnlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFPgetpal(fn, (void *)_fcdtocp(pal));
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dpippal
 * Purpose: Write palette to file
 * Inputs:  filename: name of HDF file
 *          palette: palette to be written to file
 *          overwrite: if 1, overwrite last palette read or written
 *                     if 0, write it as a fresh palette
 *          filemode: if "a", append palette to file
 *                    if "w", create new file
 *          fnlen:  length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF users, programmers, utilities
 * Invokes: DFPputpal
 * Remarks: To overwrite, the filename must be the same as for the previous
 *          call
 *---------------------------------------------------------------------------*/

intf
ndpippal(_fcd filename, _fcd pal, intf *overwrite, _fcd filemode, intf *fnlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFPputpal(fn, (void *)_fcdtocp(pal), (intn)*overwrite, (char *)_fcdtocp(filemode));
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dpinpal
 * Purpose: How many palettes are present in this file?
 * Inputs:  filename, fnlen: name, length of HDF file
 * Returns: number of palettes on success, -1 on failure with DFerror set
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFPnpals
 *---------------------------------------------------------------------------*/

intf
ndpinpal(_fcd filename, intf *fnlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFPnpals(fn);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dpirref
 * Purpose: Set ref of palette to get next
 * Inputs:  filename: file to which this applies
 *          ref: reference number of next get
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFPreadref
 * Remarks: checks if palette with this ref exists
 *---------------------------------------------------------------------------*/

intf
ndpirref(_fcd filename, intf *ref, intf *fnlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFPreadref(fn, (uint16)*ref);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dpiwref
 * Purpose: Set ref of palette to put next
 * Inputs:  filename: file to which this applies
 *          ref: reference number of next put
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFPwriteref
 *---------------------------------------------------------------------------*/

intf
ndpiwref(_fcd filename, intf *ref, intf *fnlen)
{

    char *fn;
    intf  ret;

    fn = HDf2cstring(filename, (intn)*fnlen);
    if (!fn)
        return -1;
    ret = DFPreadref(fn, (uint16)*ref);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dprest
 * Purpose: Do not remember info about file - get again from first palette
 * Inputs:  none
 * Returns: 0 on success
 * Users:   HDF programmers
 * Remarks: Invokes DFPrestart
 *---------------------------------------------------------------------------*/

intf
ndprest(void)
{
    return DFPrestart();
}

/*-----------------------------------------------------------------------------
 * Name:    dplref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFPlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

intf
ndplref(void)
{
    return (intf)DFPlastref();
}

/*-----------------------------------------------------------------------------
 * Name:    dfprestart
 * Purpose: Do not remember info about file - get again from first palette
 * Inputs:  none
 * Returns: 0 on success
 * Users:   HDF programmers
 * Remarks: Invokes DFPrestart
 *---------------------------------------------------------------------------*/

intf
ndfprestart(void)
{
    return DFPrestart();
}

/*-----------------------------------------------------------------------------
 * Name:    dfplastref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFPlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

intf
ndfplastref(void)
{
    return (intf)DFPlastref();
}
