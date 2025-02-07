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
 * File:    hfilef.c
 * Purpose: C stubs for Fortran low level routines
 * Invokes: hfile.c
 * Contents:
 *  hiopen_:   call Hopen to open HDF file
 *  hclose_:   call Hclose to close HDF file
 *---------------------------------------------------------------------------*/

#include "hdfi.h"
#include "hproto_fortran.h"

/*-----------------------------------------------------------------------------
 * Name:    hiopen
 * Purpose: call Hopen to open HDF file
 * Inputs:  name: name of file to open
 *          acc_mode: access mode - integer with value DFACC_READ etc.
 *          defdds: default number of DDs per header block
 *          namelen: length of name
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers
 * Invokes: Hopen
 * Method:  Convert filename to C string, call Hopen
 *---------------------------------------------------------------------------*/

intf
nhiopen(_fcd name, intf *acc_mode, intf *defdds, intf *namelen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(name, (intn)*namelen);
    if (!fn)
        return FAIL;
    ret = (intf)Hopen(fn, (intn)*acc_mode, (int16)*defdds);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    hclose
 * Purpose: Call DFclose to close HDF file
 * Inputs:  file_id: handle to HDF file to close
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Invokes: Hclose
 *---------------------------------------------------------------------------*/

intf
nhclose(intf *file_id)
{
    return Hclose(*file_id);
}

/*-----------------------------------------------------------------------------
 * Name:    hnumber
 * Purpose: Get number of elements with tag
 * Inputs:  file_id: handle to HDF file to close
 * Returns: the number of objects of type 'tag' else FAIL
 * Users:   HDF Fortran programmers
 * Invokes: Hnumber
 *---------------------------------------------------------------------------*/

intf
nhnumber(intf *file_id, intf *tag)
{
    return Hnumber((int32)*file_id, (uint16)*tag);
}

/*-----------------------------------------------------------------------------
 * Name:    hxisdir
 * Purpose: call HXsetdir to set the directory variable for locating an external file
 * Inputs:  dir: names of directory separated by colons
 * Returns: SUCCEED if no error, else FAIL
 * Users:   HDF Fortran programmers
 * Invokes: HXsetdir
 * Method:  Convert dir to C string, call HXsetdir
 *---------------------------------------------------------------------------*/

intf
nhxisdir(_fcd dir, intf *dirlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(dir, (intn)*dirlen);
    if (!fn)
        return FAIL;
    ret = (intf)HXsetdir(fn);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    hxiscdir
 * Purpose: call HXsetcreatedir to set the directory variable for creating an external file
 * Inputs:  dir: name of directory
 * Returns: SUCCEED if no error, else FAIL
 * Users:   HDF Fortran programmers
 * Invokes: HXsetcreatedir
 * Method:  Convert dir to C string, call HXsetcdir
 *---------------------------------------------------------------------------*/

intf
nhxiscdir(_fcd dir, intf *dirlen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(dir, (intn)*dirlen);
    if (!fn)
        return FAIL;
    ret = (intf)HXsetcreatedir(fn);
    free(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    hddontatexit
 * Purpose: Call HDdont_atexit
 * Inputs:
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Invokes: HDdont_atexit
 *---------------------------------------------------------------------------*/

intf
nhddontatexit(void)
{
    return (intf)(HDdont_atexit());
}
/*-----------------------------------------------------------------------------
 * Name: hglibverc
 * Purpose:  Calls Hgetlibversion
 *
 * Outputs: major_v - major version number
 *          minor_v - minor version number
 *          release - release number
 *          string  - version number text string
 * Returns: SUCCEED (0) if successful and FAIL(-1) otherwise
 *----------------------------------------------------------------------------*/
intf
nhglibverc(intf *major_v, intf *minor_v, intf *release, _fcd string, intf *len)
{
    char  *cstring;
    uint32 cmajor_v;
    uint32 cminor_v;
    uint32 crelease;
    intn   status;

    cstring = NULL;
    if (*len)
        cstring = (char *)malloc((uint32)*len + 1);
    status = Hgetlibversion(&cmajor_v, &cminor_v, &crelease, cstring);

    HDpackFstring(cstring, _fcdtocp(string), *len);

    free(cstring);

    *major_v = (intf)cmajor_v;
    *minor_v = (intf)cminor_v;
    *release = (intf)crelease;

    return (intf)status;
}
/*-----------------------------------------------------------------------------
 * Name: hgfilverc
 * Purpose:  Calls Hgetfileversion
 * Inputs:  file_id - file identifier
 * Outputs: major_v - major version number
 *          minor_v - minor version number
 *          release - release number
 *          string  - version number text string
 * Returns: SUCCEED (0) if successful and FAIL(-1) otherwise
 *----------------------------------------------------------------------------*/
intf
nhgfilverc(intf *file_id, intf *major_v, intf *minor_v, intf *release, _fcd string, intf *len)
{
    char  *cstring;
    uint32 cmajor_v;
    uint32 cminor_v;
    uint32 crelease;
    intn   status;

    cstring = NULL;
    if (*len)
        cstring = (char *)malloc((uint32)*len + 1);
    status = Hgetfileversion((int32)*file_id, &cmajor_v, &cminor_v, &crelease, cstring);

    HDpackFstring(cstring, _fcdtocp(string), *len);

    free(cstring);

    *major_v = (intf)cmajor_v;
    *minor_v = (intf)cminor_v;
    *release = (intf)crelease;

    return (intf)status;
}
/*-----------------------------------------------------------------------------
 * Name:    hiishdf
 * Purpose: call Hishdf function
 * Inputs:  name: Name of the file
 *          namelen: length of name
 * Returns: TRUE(1) on success, FALSE (-1) on failure
 * Users:   HDF Fortran programmers
 * Method:  Convert filename to C string, call Hishdf
 *---------------------------------------------------------------------------*/

intf
nhiishdf(_fcd name, intf *namelen)
{
    char *fn;
    intf  ret;

    fn = HDf2cstring(name, (intn)*namelen);
    if (!fn)
        return FAIL;
    ret = (intf)Hishdf(fn);
    free(fn);
    return ret;
}
/*-----------------------------------------------------------------------------
 * Name:    hconfinfc
 * Purpose: call HCget_config_info
 * Inputs:  coder_type - compression type
 * Outputs: info       - flag to indicate compression status
 *                       0 - compression is not available
 *                       1 - only decoder found
 *                       2 - both decoder and encoder are available
 * Returns: SUCCEED (0)  on success, FALSE (-1) on failure
 *---------------------------------------------------------------------------*/

intf
nhconfinfc(intf *coder_type, intf *info)
{
    comp_coder_t coder_type_c;
    uint32       info_c;
    intn         status;

    coder_type_c = (comp_coder_t)*coder_type;
    status       = HCget_config_info(coder_type_c, &info_c);
    if (status == FAIL)
        return FAIL;
    *info = (intf)info_c;
    return status;
}
