/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdio.h>
#include <string.h>

#include "hdf.h"

#ifdef H4_HAVE_NETCDF
#include "netcdf.h"
#else
#include "hdf4_netcdf.h"
#endif

#include "testcdf.h" /* defines in-memory test cdf structure */
#include "add.h"     /* functions to update in-memory netcdf */
#include "error.h"
#include "emalloc.h"
#include "tests.h"
#include "val.h"

#define LEN_OF(array) ((sizeof array) / (sizeof array[0]))

/*
 * Test ncattput
 *    check that new attribute put works in define mode
 *    check that NC_GLOBAL variable id works
 *    check that changing type of existing attribute works in define mode
 *    check that increasing length of attribute works in define mode
 *    check that changing value of existing attribute works in define mode
 *    try with bad datatype, should fail
 *    try with negative length, should fail
 *    try increasing length of attribute in data mode, should fail
 *    try putting new attribute in data mode, should fail
 *    check that changing type of existing attribute works in data mode
 *    check that decreasing length of attribute works in data mode
 *    check that changing value of existing attribute works in data mode
 *    try with bad variable handle, should fail
 *    try with bad netCDF handle, check error
 */
/* path - name of writable netcdf file to open */
void
test_ncattput(char *path)
{
    int           nerrs   = 0;
    static char   pname[] = "test_ncattput";
    int           cdfid;               /* netcdf id */
    int           ndims;               /* number of dimensions */
    int           nvars;               /* number of variables */
    int           ngatts_prev, ngatts; /* number of global attributes */
    int           xdimid;              /* id of unlimited dimension */
    int           ia, id;
    static char   byte_vals[]   = {'a', 'b'};
    static char   char_vals[]   = "chars";
    static short  short_vals[]  = {-999, 0, 999};
    static nclong long_vals[]   = {10, 20};
    static float  float_vals[]  = {1.5, 2.5, 3.5};
    static double double_vals[] = {4.5, 5.5, 6.5, 7.5};
    /*
     * test attributes; it is important for this test that the size
     * required for the attribute values increases monotonically.
     */
    static struct cdfatt atts[] = {{___, "att0", NC_BYTE, LEN_OF(byte_vals), (void *)byte_vals},
                                   {___, "att1", NC_CHAR, LEN_OF(char_vals), (void *)char_vals},
                                   {___, "att2", NC_SHORT, LEN_OF(short_vals), (void *)short_vals},
                                   {___, "att3", NC_LONG, LEN_OF(long_vals), (void *)long_vals},
                                   {___, "att4", NC_FLOAT, LEN_OF(float_vals), (void *)float_vals},
                                   {___, "att5", NC_DOUBLE, LEN_OF(double_vals), (void *)double_vals}};
    int                  na     = LEN_OF(atts); /* number of test attributes */
    int                  ww_id;                 /* variable id */
    static struct cdfvar ww =                   /* new variable */
        {"ww", NC_LONG, 1, ___, 0};
    static struct cdfatt tmp; /* attribute */

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: cdredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* get count of global attributes */
    if (ncinquire(cdfid, &ndims, &nvars, &ngatts, &xdimid) == -1) {
        error("%s: ncinquire failed", pname);
        ncclose(cdfid);
        return;
    }
    ngatts_prev = ngatts;
    /* in define mode, add global attributes of every type */
    for (ia = 0; ia < na; ia++) {
        if (ncattput(cdfid, NC_GLOBAL, atts[ia].name, atts[ia].type, atts[ia].len, atts[ia].val) == -1) {
            error("%s: ncattput of NC_GLOBAL attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        add_att(test_g, NC_GLOBAL, &atts[ia]); /* keep in-memory netcdf updated */
    }
    /* make sure count of global attributes has been updated */
    if (ncinquire(cdfid, &ndims, &nvars, &ngatts, &xdimid) == -1) {
        error("%s: ncinquire failed", pname);
        ncclose(cdfid);
        return;
    }
    if (ngatts != ngatts_prev + na) {
        error("%s: number of global = %d, expected %d", pname, ngatts, ngatts_prev + na);
        nerrs++;
    }
    /* check with ncattinq and ncattget that NC_GLOBAL attributes put OK */
    for (ia = 0; ia < na; ia++) {
        if (ncattinq(cdfid, NC_GLOBAL, atts[ia].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq of global attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (atts[ia].type != tmp.type || atts[ia].len != tmp.len) {
            error("%s: NC_GLOBAL ncattinq got unexpected type or len", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(atts[ia].len * nctypelen(atts[ia].type));
        if (ncattget(cdfid, NC_GLOBAL, atts[ia].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, atts[ia].val) != 0) {
            error("%s: ncattget got bad values after put of NC_GLOBAL attrs", pname);
            nerrs++;
        }
        free(tmp.val);
    }
    /* add a variable, then variable attributes of every type */
    ww.dims = (int *)emalloc(sizeof(int) * ww.ndims);
    for (id = 0; id < ww.ndims; id++)
        ww.dims[id] = id;
    if ((ww_id = ncvardef(cdfid, ww.name, ww.type, ww.ndims, ww.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &ww); /* keep in-memory netcdf in sync */
    for (ia = 0; ia < na; ia++) {
        if (ncattput(cdfid, ww_id, atts[ia].name, atts[ia].type, atts[ia].len, atts[ia].val) == -1) {
            error("%s: ncattput of variable attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        add_att(test_g, ww_id, &atts[ia]); /* keep in-memory netcdf updated */
    }
    /* check with ncattinq and ncattget that variable attributes put OK */
    for (ia = 0; ia < na; ia++) {
        if (ncattinq(cdfid, ww_id, atts[ia].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq of variable attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (atts[ia].type != tmp.type || atts[ia].len != tmp.len) {
            error("%s: ncattinq for new attribute got bad type or len", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(atts[ia].len * nctypelen(atts[ia].type));
        if (ncattget(cdfid, ww_id, atts[ia].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, atts[ia].val) != 0) {
            error("%s: ncattget got bad values after put of variable attrs", pname);
            nerrs++;
        }
        free(tmp.val);
    }
    /*
     * check that changing type of existing attribute, increasing
     * length of attribute, and changing value of existing attribute
     * work OK in define mode.
     */
    tmp.name = (char *)emalloc(H4_MAX_NC_NAME);
    for (ia = 1; ia < na; ia++) {
        if (ncattput(cdfid, ww_id, atts[ia - 1].name, atts[ia].type, atts[ia].len, atts[ia].val) == -1) {
            error("%s: ncattput of larger attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        tmp.var = atts[ia].var;
        (void)strcpy(tmp.name, atts[ia - 1].name);
        tmp.type = atts[ia].type;
        tmp.len  = atts[ia].len;
        tmp.val  = atts[ia].val;
        add_att(test_g, ww_id, &tmp); /* keep in-memory netcdf updated */
    }
    /* check with ncattinq and ncattget that variable attributes put OK */
    for (ia = 1; ia < na; ia++) {
        if (ncattinq(cdfid, ww_id, atts[ia - 1].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq of larger attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (atts[ia].type != tmp.type || atts[ia].len != tmp.len) {
            error("%s: ncattinq for larger attribute got bad type or len", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(atts[ia].len * nctypelen(atts[ia].type));
        if (ncattget(cdfid, ww_id, atts[ia - 1].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, atts[ia].val) != 0) {
            error("%s: ncattget got bad values after put of larger attrs", pname);
            nerrs++;
        }
        free(tmp.val);
    }
    /* try with bad datatype, should fail */
    if (ncattput(cdfid, ww_id, "bogus_att1", BAD_TYPE, atts[0].len, atts[0].val) != -1) {
        error("%s: ncattput should fail with bad type", pname);
        nerrs++;
    }
    /* try with negative length, should fail */
    if (ncattput(cdfid, ww_id, "bogus_att2", atts[0].type, -1, atts[0].val) != -1) {
        error("%s: ncattput should fail with bad length", pname);
        nerrs++;
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in data mode try increasing length of attribute, should fail */
    if (ncattput(cdfid, ww_id, atts[0].name, atts[0].type, atts[0].len + 10, atts[0].val) != -1) {
        error("%s: ncattput should fail with increased length in data mode", pname);
        nerrs++;
        /* reset to correct length for later tests */
        if (ncattput(cdfid, ww_id, atts[0].name, atts[0].type, atts[0].len, atts[0].val) != -1) {
            error("%s: ncattput failed to reset length in data mode", pname);
            nerrs++;
        }
    }
    /* try creating new attribute in data mode, should fail */
    if (ncattput(cdfid, ww_id, "new_name", atts[0].type, atts[0].len, atts[0].val) != -1) {
        error("%s: ncattput of new attribute in data mode should fail", pname);
        ncclose(cdfid);
        return;
    }
    /*
     * check that changing type of existing attribute, decreasing
     * length of attribute, and changing value of existing attribute
     * work OK in data mode
     */
    for (ia = 0; ia < na - 1; ia++) {
        if (ncattput(cdfid, ww_id, atts[ia + 1].name, atts[ia].type, atts[ia].len, atts[ia].val) == -1) {
            error("%s: ncattput of smaller attribute failed in data mode", pname);
            ncclose(cdfid);
            return;
        }
        tmp.var = atts[ia].var;
        (void)strcpy(tmp.name, atts[ia + 1].name);
        tmp.type = atts[ia].type;
        tmp.len  = atts[ia].len;
        tmp.val  = atts[ia].val;
        add_att(test_g, ww_id, &tmp); /* keep in-memory netcdf updated */
    }
    /* check with ncattinq and ncattget that variable attributes put OK */
    for (ia = 0; ia < na - 1; ia++) {
        if (ncattinq(cdfid, ww_id, atts[ia + 1].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq of variable attribute failed in data mode", pname);
            ncclose(cdfid);
            return;
        }
        if (atts[ia].type != tmp.type || atts[ia].len != tmp.len) {
            error("%s: VARIABLE ncattinq got bad type or len in data mode", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(atts[ia].len * nctypelen(atts[ia].type));
        if (ncattget(cdfid, ww_id, atts[ia + 1].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed in data mode", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, atts[ia].val) != 0) {
            error("%s: ncattget got bad values in data mode", pname);
            nerrs++;
        }
        free(tmp.val);
    }
    /* try with bad variable handle, should fail */
    if (ncattput(cdfid, test_g->nvars, atts[0].name, atts[0].type, atts[0].len, atts[0].val) != -1) {
        error("%s: ncattput should fail with bad variable handle", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    /* try with bad netcdf handle, should fail */
    if (ncattput(cdfid, ww_id, atts[0].name, atts[0].type, atts[0].len, atts[0].val) != -1) {
        error("%s: ncattput should fail with bad netcdf handle", pname);
        ncclose(cdfid);
        return;
    }
    free(tmp.name);
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattinq
 *    check returned values of properly created attributes
 *    try with nonexisting attribute, check error
 *    try with bad variable handle, check error
 *    try with bad netCDF handle, check error
 */
/* path - name of writable netcdf file to open */
void
test_ncattinq(char *path)
{
    int                  nerrs   = 0;
    static char          pname[] = "test_ncattinq";
    int                  cdfid;  /* netcdf id */
    int                  ia, id; /* attribute number */
    nc_type              type;
    int                  len;
    int                  vv_id; /* variable id */
    static struct cdfvar vv =   /* new variable */
        {"vv", NC_SHORT, 2, ___, 0};

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* in data mode, check all attributes against test netcdf */
    for (ia = 0; ia < test_g->natts; ia++) {
        if (ncattinq(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, &type, &len) == -1) {
            error("%s: ncattinq failed", pname);
            ncclose(cdfid);
            return;
        }
        if (type != test_g->atts[ia].type) {
            error("%s: ncattinq returned wrong type", pname);
            ncclose(cdfid);
            return;
        }
        if (len != test_g->atts[ia].len) {
            error("%s: ncattinq returned wrong len", pname);
            ncclose(cdfid);
            return;
        }
    }

    /* enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: cdredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in define mode, add a variable */
    vv.dims = (int *)emalloc(sizeof(int) * vv.ndims);
    for (id = 0; id < vv.ndims; id++)
        vv.dims[id] = id; /* assumes vv.ndims <= test_g->ndims */
    if ((vv_id = ncvardef(cdfid, vv.name, vv.type, vv.ndims, vv.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &vv); /* keep in-memory netcdf in sync */

    /* try with nonexisting attribute, should fail */
    if (ncattinq(cdfid, vv_id, "nonesuch", &type, &len) != -1) {
        error("%s: ncattinq should fail with nonexisting attribute", pname);
        ncclose(cdfid);
        return;
    }
    /* try with bad variable handle, should fail */
    if (ncattinq(cdfid, test_g->nvars, test_g->atts[0].name, &type, &len) != -1) {
        error("%s: ncattinq should fail with bad variable id", pname);
        ncclose(cdfid);
        return;
    }
    /* in define mode check all attributes against test netcdf */
    for (ia = 0; ia < test_g->natts; ia++) {
        if (ncattinq(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, &type, &len) == -1) {
            error("%s: ncattinq in define mode failed", pname);
            ncclose(cdfid);
            return;
        }
        if (type != test_g->atts[ia].type) {
            error("%s: ncattinq in define mode returned wrong type", pname);
            ncclose(cdfid);
            return;
        }
        if (len != test_g->atts[ia].len) {
            error("%s: ncattinq in define mode returned wrong len", pname);
            ncclose(cdfid);
            return;
        }
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    if (ncattinq(cdfid, NC_GLOBAL, test_g->atts[0].name, &type, &len) != -1) {
        error("%s: ncattinq should fail with bad cdfid", pname);
        nerrs++;
    }
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattget
 *    check that NC_GLOBAL variable id works
 *    check in both modes
 *    check that proper call worked after ncattput
 *    try with bad variable handle, check error
 *    try with nonexisting attribute, check error
 *    try with bad netCDF handle, check error
 */
/* path - name of writable netcdf file to open */
void
test_ncattget(char *path)
{
    int                  nerrs = 0;
    int                  cdfid; /* netcdf id */
    int                  ia, id;
    static struct cdfatt tmp;   /* attribute */
    int                  uu_id; /* variable id */
    static struct cdfvar uu =   /* variable */
        {"uu", NC_LONG, 2, ___, 0};
    static nclong        uumax = 1000; /* attribute value */
    static struct cdfatt vmax  =       /* attribute */
        {___, "valid_max", NC_LONG, 1, (void *)&uumax};

    static char pname[] = "test_ncattget";

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: cdredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* add a variable */
    uu.dims = (int *)emalloc(sizeof(int) * uu.ndims);
    for (id = 0; id < uu.ndims; id++)
        uu.dims[id] = id;
    if ((uu_id = ncvardef(cdfid, uu.name, uu.type, uu.ndims, uu.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &uu); /* keep in-memory netcdf in sync */

    /* add an attribute */
    if (ncattput(cdfid, uu_id, vmax.name, vmax.type, vmax.len, vmax.val) == -1) {
        error("%s: ncattput of variable attribute failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, uu_id, &vmax); /* keep in-memory netcdf updated */

    /* in define mode, check all attributes values against test netcdf */
    for (ia = 0; ia < test_g->natts; ia++) {
        if (ncattinq(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq in define mode failed", pname);
            ncclose(cdfid);
            return;
        }
        if (tmp.type != test_g->atts[ia].type) {
            error("%s: ncattinq in define mode returned wrong type", pname);
            ncclose(cdfid);
            return;
        }
        if (tmp.len != test_g->atts[ia].len) {
            error("%s: ncattinq in define mode returned wrong len", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(tmp.len * nctypelen(tmp.type));
        if (ncattget(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed in define mode", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, test_g->atts[ia].val) != 0) {
            error("%s: ncattget got bad values in define mode", pname);
            error("   cdfid=%d, varname=%s, attname=%s, type=%d, len=%d", cdfid,
                  test_g->vars[test_g->atts[ia].var].name, test_g->atts[ia].name, test_g->atts[ia].type,
                  test_g->atts[ia].len);
            fprintf(stderr, "should have got:");
            val_out(test_g->atts[ia].type, test_g->atts[ia].len, test_g->atts[ia].val);
            fprintf(stderr, "    instead got:");
            val_out(tmp.type, tmp.len, tmp.val);
            nerrs++;
        }
        free(tmp.val);
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }

    /* in data mode, check all attributes values against test netcdf */
    for (ia = 0; ia < test_g->natts; ia++) {
        if (ncattinq(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, &tmp.type, &tmp.len) == -1) {
            error("%s: ncattinq failed", pname);
            ncclose(cdfid);
            return;
        }
        if (tmp.type != test_g->atts[ia].type) {
            error("%s: ncattinq returned wrong type", pname);
            ncclose(cdfid);
            return;
        }
        if (tmp.len != test_g->atts[ia].len) {
            error("%s: ncattinq returned wrong len", pname);
            ncclose(cdfid);
            return;
        }
        /* allocate space to hold the attribute value to be retrieved */
        tmp.val = emalloc(tmp.len * nctypelen(tmp.type));
        if (ncattget(cdfid, test_g->atts[ia].var, test_g->atts[ia].name, tmp.val) == -1) {
            error("%s: ncattget of variable attribute failed in data mode", pname);
            ncclose(cdfid);
            return;
        }
        if (val_cmp(tmp.type, tmp.len, tmp.val, test_g->atts[ia].val) != 0) {
            error("%s: ncattget got bad values in data mode", pname);
            error("   cdfid=%d, varname=%s, attname=%s, type=%d, len=%d", cdfid,
                  test_g->vars[test_g->atts[ia].var].name, test_g->atts[ia].name, test_g->atts[ia].type,
                  test_g->atts[ia].len);
            fprintf(stderr, "should have got:");
            val_out(test_g->atts[ia].type, test_g->atts[ia].len, test_g->atts[ia].val);
            fprintf(stderr, "    instead got:");
            val_out(tmp.type, tmp.len, tmp.val);
            nerrs++;
        }
        free(tmp.val);
    }
    /* try with bad variable handle, should fail */
    if (ncattget(cdfid, test_g->nvars, vmax.name, vmax.val) != -1) {
        error("%s: ncattget should fail with bad variable handle", pname);
        ncclose(cdfid);
        return;
    }
    /* try getting non-existent attribute, should fail */
    if (ncattget(cdfid, uu_id, "nonesuch", vmax.val) != -1) {
        error("%s: ncattget should fail with nonexistant attribute", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    /* try with bad netcdf handle, should fail */
    if (ncattget(cdfid, uu_id, vmax.name, vmax.val) != -1) {
        error("%s: ncattput should fail with bad netcdf handle", pname);
        ncclose(cdfid);
        return;
    }
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattcopy
 *    check that NC_GLOBAL variable for source or target works
 *    check that new attribute put works with target in define mode
 *    check that old attribute put works with target in data mode
 *    check that changing type and length of an attribute work OK
 *    try with same cdfid for source and target, different variables
 *    try with same cdfid for source and target, same variable
 *    try with nonexisting attribute, check error
 *    try with bad source or target netCDF handles, check error
 *    try with bad source or target variable handle, check error
 */
/* path1 - name of input netcdf file to open */
/* path2 - name of output netcdf file to create */
void
test_ncattcopy(char *path1, char *path2)
{
    int                  nerrs   = 0;
    static char          pname[] = "test_ncattcopy";
    int                  cdfid, cdfid2; /* netcdf id */
    int                  id;            /* dimension id */
    int                  tt_id;         /* variable id */
    static struct cdfvar tt =           /* new variable for source netcdf */
        {"tt", NC_LONG, 1, ___, 0};
    int                  tu_id, tu2_id; /* variable ids */
    static struct cdfvar tu =           /* new variable for target netcdf */
        {"tu", NC_DOUBLE, 2, ___, 0};
    static double        double_vals[] = {-1., -2.};
    static float         float_vals[]  = {-1., -2.};
    static struct cdfatt att           = /* attribute */
        {___, "att", NC_DOUBLE, LEN_OF(double_vals), (void *)double_vals};
    static struct cdfatt att2 = /* attribute */
        {___, "att", NC_FLOAT, LEN_OF(float_vals), (void *)float_vals};
    static struct cdfatt tmp; /* attribute */

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path1, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* opened OK, enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: ncredef failed on source", pname);
        ncclose(cdfid);
        return;
    }
    /* in define mode, add a global attribute, a variable and an attribute */
    if (ncattput(cdfid, NC_GLOBAL, att.name, att.type, att.len, att.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &att); /* keep in-memory netcdf consistent */
    tt.dims = (int *)emalloc(sizeof(int) * tt.ndims);
    for (id = 0; id < tt.ndims; id++)
        tt.dims[0] = id;
    if ((tt_id = ncvardef(cdfid, tt.name, tt.type, tt.ndims, tt.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &tt); /* keep in-memory netcdf consistent */
    if (ncattput(cdfid, tt_id, att.name, att.type, att.len, att.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, tt_id, &att); /* keep in-memory netcdf consistent */

    tu.dims = (int *)emalloc(sizeof(int) * tu.ndims);
    for (id = 0; id < tu.ndims; id++)
        tu.dims[id] = id;
    if ((tu_id = ncvardef(cdfid, tu.name, tu.type, tu.ndims, tu.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &tu); /* keep in-memory netcdf consistent */
    if (ncattput(cdfid, tu_id, att.name, att.type, att.len, att.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, tu_id, &att); /* keep in-memory netcdf consistent */
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* first (source) netcdf is in data mode */
    /* create second netCDF to copy attributes to */
    if ((cdfid2 = nccreate(path2, NC_CLOBBER)) == -1) {
        error("%s: nccreate failed", pname);
        return;
    }
    /* create dimensions and variable in second netcdf */
    for (id = 0; id < tu.ndims; id++) { /* copy dimensions from source */
        if ((tu.dims[id] = ncdimdef(cdfid2, test_g->dims[id].name, test_g->dims[id].size)) == -1) {
            error("%s: ncdimdef failed", pname);
            ncclose(cdfid);
            ncclose(cdfid2);
            return;
        }
    }
    if ((tu2_id = ncvardef(cdfid2, tu.name, tu.type, tu.ndims, tu.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* try copying NC_GLOBAL attribute from source to target */
    if (ncattcopy(cdfid, NC_GLOBAL, att.name, cdfid2, NC_GLOBAL) == -1) {
        error("%s: ncattcopy on NC_GLOBAL attribute '%s' failed", pname, att.name);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* check that copy worked with ncattinq and ncattget */
    if (ncattinq(cdfid2, NC_GLOBAL, att.name, &tmp.type, &tmp.len) == -1) {
        error("%s: ncattinq of NC_GLOBAL attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (att.type != tmp.type || att.len != tmp.len) {
        error("%s: NC_GLOBAL ncattinq got unexpected type or len", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* allocate space to hold the attribute value to be retrieved */
    tmp.val = emalloc(att.len * nctypelen(att.type));
    if (ncattget(cdfid2, NC_GLOBAL, att.name, tmp.val) == -1) {
        error("%s: ncattget of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, att.val) != 0) {
        error("%s: ncattget got bad values after put of NC_GLOBAL attrs", pname);
        nerrs++;
    }
    free(tmp.val);
    /* try copying variable attribute from source to target */
    if (ncattcopy(cdfid, tt_id, att.name, cdfid2, tu2_id) == -1) {
        error("%s: ncattcopy failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* check that copy worked with ncattinq and ncattget */
    if (ncattinq(cdfid2, tu2_id, att.name, &tmp.type, &tmp.len) == -1) {
        error("%s: ncattinq of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (att.type != tmp.type || att.len != tmp.len) {
        error("%s: variable ncattinq got unexpected type or len", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* allocate space to hold the attribute value to be retrieved */
    tmp.val = emalloc(att.len * nctypelen(att.type));
    if (ncattget(cdfid2, tu2_id, att.name, tmp.val) == -1) {
        error("%s: ncattget of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, att.val) != 0) {
        error("%s: ncattget got bad values after copy of variable attrs", pname);
        nerrs++;
    }
    free(tmp.val);

    /*
     * check that old attribute put works with target in data mode,
     * also checks that changing type and length of an attribute works OK
     */
    if (ncendef(cdfid2) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* change attribute to shorter attribute */
    if (ncattput(cdfid, NC_GLOBAL, att2.name, att2.type, att2.len, att2.val) == -1) {
        error("%s: ncattput of shorter NC_GLOBAL attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    add_att(test_g, NC_GLOBAL, &att2); /* keep in-memory netcdf consistent */
    /* copy shorter attribute on existing attribute */
    if (ncattcopy(cdfid, NC_GLOBAL, att2.name, cdfid2, tu2_id) == -1) {
        error("%s: ncattcopy of shorter attribute on old attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* check that copy worked with ncattinq and ncattget */
    if (ncattinq(cdfid2, tu2_id, att2.name, &tmp.type, &tmp.len) == -1) {
        error("%s: ncattinq of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (att2.type != tmp.type || att2.len != tmp.len) {
        error("%s: variable ncattinq got unexpected type or len", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* allocate space to hold the attribute value to be retrieved */
    tmp.val = emalloc(att2.len * nctypelen(att2.type));
    if (ncattget(cdfid2, tu2_id, att2.name, tmp.val) == -1) {
        error("%s: ncattget of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, att2.val) != 0) {
        error("%s: ncattget got bad values after copy of variable attrs", pname);
        nerrs++;
    }
    free(tmp.val);

    /* try copying with same source and target netcdf, different variables */
    /* copy shorter attribute on existing attribute */
    if (ncattcopy(cdfid, NC_GLOBAL, att2.name, cdfid, tu_id) == -1) {
        error("%s: ncattcopy of shorter NC_GLOBAL attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    add_att(test_g, tu_id, &att2); /* keep in-memory netcdf consistent */
    /* check that copy worked with ncattinq and ncattget */
    if (ncattinq(cdfid, tu_id, att2.name, &tmp.type, &tmp.len) == -1) {
        error("%s: ncattinq of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (att2.type != tmp.type || att2.len != tmp.len) {
        error("%s: variable ncattinq got unexpected type or len", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* allocate space to hold the attribute value to be retrieved */
    tmp.val = emalloc(att2.len * nctypelen(att2.type));
    if (ncattget(cdfid, tu_id, att2.name, tmp.val) == -1) {
        error("%s: ncattget of variable attribute failed", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, att2.val) != 0) {
        error("%s: ncattget got bad values after copy of variable attrs", pname);
        nerrs++;
    }
    free(tmp.val);

    /* try with same cdfid for source and target, same variable */
    if (ncattcopy(cdfid, tu_id, att.name, cdfid, tu_id) == -1) {
        error("%s: ncattcopy failed with identical source and target", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* try with nonexisting attribute, check error */
    if (ncattcopy(cdfid, tt_id, "nonesuch", cdfid, tu_id) != -1) {
        error("%s: ncattcopy should fail with bad attribute name", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    /* try with bad source or target variable handle, check error */
    if (ncattcopy(cdfid, test_g->nvars, att.name, cdfid, tu_id) != -1) {
        error("%s: ncattcopy should fail with bad source variable id", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (ncattcopy(cdfid, tt_id, att.name, cdfid, 2) != -1) {
        error("%s: ncattcopy should fail with bad target variable id", pname);
        ncclose(cdfid);
        ncclose(cdfid2);
        return;
    }
    if (ncclose(cdfid2) == -1) {
        error("%s: ncclose failed", pname);
        ncclose(cdfid);
        return;
    }
    /* try with bad source or target netCDF handles, check error */
    if (ncattcopy(cdfid, tt_id, att.name, cdfid2, tu_id) != -1) {
        error("%s: ncattcopy should fail with bad target netcdf id", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    if (ncattcopy(cdfid, tt_id, att.name, cdfid2, tu_id) != -1) {
        error("%s: ncattcopy should fail with bad netcdf id", pname);
        nerrs++;
    }
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattname
 *    check that NC_GLOBAL variable id works
 *    check in both modes
 *    check that proper call worked after ncattput
 *    try with bad netCDF handle, check error
 *    try with bad variable handle, check error
 *    try with bad attribute number, check error
 */
/* path - name of writable netcdf file to open */
void
test_ncattname(char *path)
{
    int                  nerrs   = 0;
    static char          pname[] = "test_ncattname";
    int                  cdfid;  /* netcdf id */
    struct cdfatt        tmp;    /* attributes */
    int                  ia, ib; /* attribute numbers */
    int                  iv;     /* variable id */
    static short         short_vals[] = {3, 4, 5};
    static struct cdfatt att          = /* attribute */
        {___, ___, NC_SHORT, LEN_OF(short_vals), (void *)short_vals};

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* opened OK, enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: ncredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* for each NC_GLOBAL attribute, get name and compare with expected name */
    att.name = (char *)emalloc(H4_MAX_NC_NAME);
    ib       = 0;
    for (ia = 0; ia < test_g->ngatts; ia++) {
        if (ncattname(cdfid, NC_GLOBAL, ia, att.name) == -1) {
            error("%s: ncattname failed on global attribute", pname);
            ncclose(cdfid);
            return;
        }
        /* find number of next global attribute */
        while (ib < test_g->natts && test_g->atts[ib].var != NC_GLOBAL)
            ib++;
        if (ib >= test_g->natts) {
            error("%s: test problem, expected global attribute not found", pname);
            ncclose(cdfid);
            return;
        }
        if (strcmp(att.name, test_g->atts[ib].name) != 0) {
            error("%s: NC_GLOBAL attribute name `%s' instead of expected `%s'", pname, att.name,
                  test_g->atts[ib].name);
            nerrs++;
        }
        ib++;
    }
    /* for each variable attribute, get name and compare with expected name */
    for (iv = 0; iv < test_g->nvars; iv++) {
        ib = 0;
        for (ia = 0; ia < test_g->vars[iv].natts; ia++) {
            if (ncattname(cdfid, iv, ia, att.name) == -1) {
                error("%s: ncattname failed on variable attribute", pname);
                ncclose(cdfid);
                return;
            }
            /* find number of next attribute */
            while (ib < test_g->natts && test_g->atts[ib].var != iv)
                ib++;
            if (ib >= test_g->natts) {
                error("%s: problem  in test, expected attribute not found", pname);
                ncclose(cdfid);
                return;
            }
            if (strcmp(att.name, test_g->atts[ib].name) != 0) {
                error("%s: variable '%s' name `%s' instead of expected `%s'", pname, test_g->vars[iv].name,
                      att.name, test_g->atts[ib].name);
                nerrs++;
            }
            ib++;
        }
    }
    /* in define mode, add a global attribute */
    (void)strcpy(att.name, "attx");
    if (ncattput(cdfid, NC_GLOBAL, att.name, att.type, att.len, att.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &att); /* keep in-memory netcdf consistent */
    /* test that ncattname works immediately after ncattput */
    tmp.name = (char *)emalloc(H4_MAX_NC_NAME);
    if (ncattname(cdfid, NC_GLOBAL, test_g->ngatts - 1, tmp.name) == -1) {
        error("%s: ncattname failed on variable attribute", pname);
        ncclose(cdfid);
        return;
    }
    if (strcmp(att.name, tmp.name) != 0) {
        error("%s: immediate NC_GLOBAL name `%s' instead of expected `%s'", pname, tmp.name, att.name);
        nerrs++;
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in data mode */
    /* for each NC_GLOBAL attribute, get name and compare with expected name */
    ib = 0;
    for (ia = 0; ia < test_g->ngatts; ia++) {
        if (ncattname(cdfid, NC_GLOBAL, ia, att.name) == -1) {
            error("%s: ncattname failed on global attribute", pname);
            ncclose(cdfid);
            return;
        }
        /* find number of next global attribute */
        while (ib < test_g->natts && test_g->atts[ib].var != NC_GLOBAL)
            ib++;
        if (ib >= test_g->natts) {
            error("%s: test problem, expected global attribute not found", pname);
            ncclose(cdfid);
            return;
        }
        if (strcmp(att.name, test_g->atts[ib].name) != 0) {
            error("%s: NC_GLOBAL attribute name `%s' instead of expected `%s'", pname, att.name,
                  test_g->atts[ib].name);
            nerrs++;
        }
        ib++;
    }
    /* for each variable attribute, get name and compare with expected name */
    for (iv = 0; iv < test_g->nvars; iv++) {
        ib = 0;
        for (ia = 0; ia < test_g->vars[iv].natts; ia++) {
            if (ncattname(cdfid, iv, ia, att.name) == -1) {
                error("%s: ncattname failed on variable attribute", pname);
                ncclose(cdfid);
                return;
            }
            /* find number of next attribute */
            while (ib < test_g->natts && test_g->atts[ib].var != iv)
                ib++;
            if (ib >= test_g->natts) {
                error("%s: problem  in test, expected attribute not found", pname);
                ncclose(cdfid);
                return;
            }
            if (strcmp(att.name, test_g->atts[ib].name) != 0) {
                error("%s: variable '%s' name `%s' instead of expected `%s'", pname, test_g->vars[iv].name,
                      att.name, test_g->atts[ib].name);
                nerrs++;
            }
            ib++;
        }
    }
    /* try with bad variable handle, check error */
    if (ncattname(cdfid, test_g->nvars, 0, att.name) != -1) {
        error("%s: ncattname should fail with bad variable handle", pname);
        ncclose(cdfid);
        return;
    }
    /* try with bad attribute number, check error */
    if (ncattname(cdfid, NC_GLOBAL, -1, att.name) != -1) {
        error("%s: ncattname should fail with negative number", pname);
        ncclose(cdfid);
        return;
    }
    if (ncattname(cdfid, NC_GLOBAL, test_g->ngatts, att.name) != -1) {
        error("%s: ncattname should fail with too-high number", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        nerrs++;
        return;
    }
    /* try with bad netCDF handle, check error */
    if (ncattname(cdfid, NC_GLOBAL, 0, att.name) != -1) {
        error("%s: ncattname shoul fail with bad cdfid", pname);
        nerrs++;
    }
    free(tmp.name);
    free(att.name);
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattrename
 *    check that proper rename worked with ncattinq, ncattget
 *    try renaming to existing attribute name, check error
 *    try with nonexisting attribute, check error
 *    try with bad variable handle, check error
 *    try in data mode, check error
 *    try with bad netCDF handle, check error
 */
/* name of writable netcdf file to open */
void
test_ncattrename(char *path)
{
    int                  nerrs   = 0;
    static char          pname[] = "test_ncattrename";
    int                  cdfid; /* netcdf id */
    static char          newname[]    = "shorter";
    static char          longername[] = "longer_name";
    struct cdfatt        tmp; /* attributes */
    static short         short_vals[] = {3, 4, 5};
    static struct cdfatt atty         = /* attribute */
        {___, "long_name", NC_SHORT, LEN_OF(short_vals), (void *)short_vals};
    static struct cdfatt attz = /* attribute */
        {___, "arggh", NC_SHORT, LEN_OF(short_vals), (void *)short_vals};
    int ynum; /* attribute number */

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* opened OK, enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: cdredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in define mode, add two attributes */
    if (ncattput(cdfid, NC_GLOBAL, atty.name, atty.type, atty.len, atty.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &atty); /* keep in-memory netcdf in sync */
    ynum = test_g->natts - 1;          /* number of attribute just put */
    if (ncattput(cdfid, NC_GLOBAL, attz.name, attz.type, attz.len, attz.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &attz); /* keep in-memory netcdf in sync */

    /* rename first attribute to shorter name */
    if (ncattrename(cdfid, NC_GLOBAL, atty.name, newname) == -1) {
        error("%s: ncattrename failed", pname);
        ncclose(cdfid);
        return;
    }
    (void)strcpy(test_g->atts[ynum].name, newname); /* keep test consistent */
    /* check new name with ncattinq */
    if (ncattinq(cdfid, NC_GLOBAL, newname, &tmp.type, &tmp.len) == -1) {
        error("%s: ncattinq of renamed attribute failed", pname);
        ncclose(cdfid);
        return;
    }
    if (atty.type != tmp.type || atty.len != tmp.len) {
        error("%s: NC_GLOBAL ncattinq got unexpected type or len", pname);
        ncclose(cdfid);
        return;
    }
    /* allocate space to hold the attribute value to be retrieved */
    tmp.val = emalloc(atty.len * nctypelen(atty.type));
    if (ncattget(cdfid, NC_GLOBAL, newname, tmp.val) == -1) {
        error("%s: ncattget of variable attribute failed", pname);
        ncclose(cdfid);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, atty.val) != 0) {
        error("%s: ncattget got bad values after rename attrs", pname);
        nerrs++;
    }
    if (ncattinq(cdfid, NC_GLOBAL, atty.name, &tmp.type, &tmp.len) != -1) {
        error("%s: ncattrename left attribute with old name", pname);
        ncclose(cdfid);
        return;
    }
    /* try to rename second attribute same as first, should fail */
    if (ncattrename(cdfid, NC_GLOBAL, attz.name, newname) != -1) {
        error("%s: ncattrename should have failed with used name", pname);
        ncclose(cdfid);
        return;
    }
    /* try to rename second attribute with a longer name */
    if (ncattrename(cdfid, NC_GLOBAL, attz.name, longername) == -1) {
        error("%s: ncattrename failed with longer name", pname);
        ncclose(cdfid);
        return;
    }
    /* try with bad variable handle, check for failure */
    if (ncattrename(cdfid, test_g->nvars, newname, atty.name) != -1) {
        error("%s: ncattrename should have failed on bad variable id", pname);
        ncclose(cdfid);
        return;
    }
    /* try with bad attribute name, check for failure */
    if (ncattrename(cdfid, NC_GLOBAL, "nonesuch", newname) != -1) {
        error("%s: ncattrename should have failed on bad attribute name", pname);
        ncclose(cdfid);
        return;
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in data mode change name to even shorter and check value */
    if (ncattrename(cdfid, NC_GLOBAL, newname, "short") == -1) {
        error("%s: ncattrename to shorter name failed in data mode", pname);
        ncclose(cdfid);
        return;
    }
    if (ncattrename(cdfid, NC_GLOBAL, "short", "plugh") == -1) {
        error("%s: ncattrename to same length failed in data mode", pname);
        ncclose(cdfid);
        return;
    }
    if (ncattget(cdfid, NC_GLOBAL, "plugh", tmp.val) == -1) {
        error("%s: ncgetatt of renamed attribute failed in data mode", pname);
        ncclose(cdfid);
        return;
    }
    if (val_cmp(tmp.type, tmp.len, tmp.val, atty.val) != 0) {
        error("%s: ncattget got bad values after data mode rename", pname);
        nerrs++;
    }
    free(tmp.val);
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    /* should fail, since bad handle */
    if (ncattrename(cdfid, NC_GLOBAL, newname, atty.name) != -1) {
        error("%s: ncattrename should fail with bad cdfid", pname);
        ncclose(cdfid);
        return;
    }
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}

/*
 * Test ncattdel
 *    check that proper delete worked
 *    try with bad netCDF handle, check error
 *    try with bad variable handle, check error
 *    try with nonexisting attribute, check error
 *    try in data mode, check error
 */
/* path - name of writable netcdf file to open */
void
test_ncattdel(char *path)
{
    int                  nerrs   = 0;
    static char          pname[] = "test_ncattdel";
    int                  cdfid; /* netcdf id */
    static short         short_vals[] = {-1, -2, -3};
    static struct cdfatt yaa          = /* attribute */
        {___, "yet_another_attribute", NC_SHORT, LEN_OF(short_vals), (void *)short_vals};
    int                  id;     /* dimension id */
    int                  yav_id; /* variable id */
    static struct cdfvar yav =   /* new variable for target netcdf */
        {"yet_another_variable", NC_DOUBLE, 2, ___, 0};
    struct cdfvar vtmp;             /* variable */
    struct cdfatt atmp;             /* attribute */
    int           ndims;            /* number of dimensions */
    int           nvars;            /* number of variables */
    int           ngatts1, ngatts2; /* number of global attributes */
    int           natts;            /* number of variable attributes */
    int           xdimid;           /* id of unlimited dimension */

    fprintf(stderr, "*** Testing %s ...\t", &pname[5]);

    if ((cdfid = ncopen(path, NC_WRITE)) == -1) {
        error("%s: ncopen failed", pname);
        return;
    }
    /* opened OK, enter define mode */
    if (ncredef(cdfid) == -1) {
        error("%s: cdredef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in define mode, add global attribute, variable, variable attribute */
    if (ncattput(cdfid, NC_GLOBAL, yaa.name, yaa.type, yaa.len, yaa.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &yaa); /* keep in-memory netcdf in sync */
    yav.dims = (int *)emalloc(sizeof(int) * yav.ndims);
    for (id = 0; id < yav.ndims; id++)
        yav.dims[id] = id;
    if ((yav_id = ncvardef(cdfid, yav.name, yav.type, yav.ndims, yav.dims)) == -1) {
        error("%s: ncvardef failed", pname);
        ncclose(cdfid);
        return;
    }
    add_var(test_g, &yav); /* keep in-memory netcdf consistent */
    if (ncattput(cdfid, yav_id, yaa.name, yaa.type, yaa.len, yaa.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, yav_id, &yaa); /* keep in-memory netcdf consistent */

    /* get number of global attributes, number of attributes for variable */
    if (ncinquire(cdfid, &ndims, &nvars, &ngatts1, &xdimid) == -1) {
        error("%s: ncinquire in data mode failed", pname);
        ncclose(cdfid);
        return;
    }
    vtmp.dims = (int *)emalloc(sizeof(int) * H4_MAX_VAR_DIMS);
    vtmp.name = (char *)emalloc(H4_MAX_NC_NAME);
    if (ncvarinq(cdfid, yav_id, vtmp.name, &vtmp.type, &vtmp.ndims, vtmp.dims, &natts) == -1) {
        error("%s: ncvarinq failed", pname);
        ncclose(cdfid);
        return;
    }

    /* delete global attribute and check that it's gone */
    if (ncattdel(cdfid, NC_GLOBAL, yaa.name) == -1) {
        error("%s: ncattdel failed", pname);
        ncclose(cdfid);
        return;
    }
    del_att(test_g, NC_GLOBAL, &yaa); /* keep in-memory netcdf consistent */
    if (ncinquire(cdfid, &ndims, &nvars, &ngatts2, &xdimid) == -1) {
        error("%s: ncinquire failed", pname);
        ncclose(cdfid);
        return;
    }
    if (ngatts2 != ngatts1 - 1) {
        error("%s: NC_GLOBAL attribute deleted, but ngatts did not decrement", pname);
        ncclose(cdfid);
        return;
    }
    if (ncattinq(cdfid, NC_GLOBAL, yaa.name, &atmp.type, &atmp.len) != -1) {
        error("%s: ncattinq on deleted NC_GLOBAL attribute should fail", pname);
        ncclose(cdfid);
        return;
    }

    /* delete variable attribute and check that it's gone */
    if (ncattdel(cdfid, yav_id, yaa.name) == -1) {
        error("%s: ncattdel failed", pname);
        ncclose(cdfid);
        return;
    }
    del_att(test_g, yav_id, &yaa); /* keep in-memory netcdf consistent */
    if (ncvarinq(cdfid, yav_id, vtmp.name, &vtmp.type, &vtmp.ndims, vtmp.dims, &vtmp.natts) == -1) {
        error("%s: ncvarinq failed", pname);
        ncclose(cdfid);
        return;
    }
    if (vtmp.natts != natts - 1) {
        error("%s: NC_GLOBAL attribute deleted, but ngatts did not decrement", pname);
        ncclose(cdfid);
        return;
    }
    if (ncattinq(cdfid, yav_id, yaa.name, &atmp.type, &atmp.len) != -1) {
        error("%s: ncattinq on deleted variable attribute should fail", pname);
        ncclose(cdfid);
        return;
    }
    /* re-add global attribute, variable, variable attribute */
    if (ncattput(cdfid, NC_GLOBAL, yaa.name, yaa.type, yaa.len, yaa.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, NC_GLOBAL, &yaa); /* keep in-memory netcdf in sync */
    if (ncattput(cdfid, yav_id, yaa.name, yaa.type, yaa.len, yaa.val) == -1) {
        error("%s: ncattput failed", pname);
        ncclose(cdfid);
        return;
    }
    add_att(test_g, yav_id, &yaa); /* keep in-memory netcdf consistent */
    /* try on nonexistent attribute, should fail */
    if (ncattdel(cdfid, yav_id, "nonesuch") != -1) {
        error("%s: ncattdel should fail on bogus attribute", pname);
        ncclose(cdfid);
        return;
    }
    /* try on bad variable id, should fail */
    if (ncattdel(cdfid, test_g->nvars, yaa.name) != -1) {
        error("%s: ncattdel should fail on bad variable id", pname);
        ncclose(cdfid);
        return;
    }
    if (ncendef(cdfid) == -1) {
        error("%s: ncendef failed", pname);
        ncclose(cdfid);
        return;
    }
    /* in data mode, should fail */
    if (ncattdel(cdfid, NC_GLOBAL, yaa.name) != -1) {
        error("%s: ncattdel in data mode should fail", pname);
        ncclose(cdfid);
        return;
    }
    if (ncclose(cdfid) == -1) {
        error("%s: ncclose failed", pname);
        return;
    }
    /* try on bad netcdf handle, should fail */
    if (ncattdel(cdfid, yav_id, yaa.name) != -1) {
        error("%s: ncattdel should fail on bad netcdf id", pname);
        nerrs++;
    }
    free(vtmp.dims);
    free(vtmp.name);
    free(yav.dims);
    if (nerrs > 0)
        fprintf(stderr, "FAILED! ***\n");
    else
        fprintf(stderr, "ok ***\n");
}
