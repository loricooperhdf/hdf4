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

/*+
   FILE
   hfile.c
   HDF low level file I/O routines

   H-Level Limits
   ==============
   o MAX_ACC access records open at a single time (#define in hfile.h)
   o int16 total tags (fixed)
   o int32 max length and offset of an element in an HDF file (fixed)

   Routine prefix conventions
   ==========================
   HP: private, external
   HI: private, static
   HD: not-private, external (i.e. usable by non-developers)

   "A" will be used to indicate that a routine is for parallel I/O.

   Prefixes now have potentially three parts: (1) the interface, (2) optional
   "A" to indicate parallel, and (3) scope:

   <prefix> :== <interface>|<interface><scope>|<interface>A<scope>
   <interface> :== H|HL|HX|SD|DFSD|DFAN|DFR8|...
   <scope> :== D|P|I

   Examples:  HAP => H interface, parallel, private external
   HAD => H interface, parallel, non-private external
   HI  => H interface, private static

   EXPORTED ROUTINES
   Hopen       -- open or create a HDF file
   Hclose      -- close HDF file
   Hstartread  -- locate and position a read access elt on a tag/ref
   Hnextread   -- locate and position a read access elt on next tag/ref.
   Hexist      -- locate an object in an HDF file
   Hinquire    -- inquire stats of an access elt
   Hstartwrite -- set up a WRITE access elt for a write
   Happendable -- attempt make a dataset appendable
   Hseek       -- position an access element to an offset in data element
   Hread       -- read the next segment from data element
   Hwrite      -- write next data segment to data element
   HDgetc      -- read a byte from data element
   HDputc      -- write a byte to data element
   Hendaccess  -- to dispose of an access element
   Hgetelement -- read in a data element
   Hputelement -- writes a data element
   Hlength     -- returns length of a data element
   Hoffset     -- get offset of data element in the file
   Hishdf      -- tells if a file is an HDF file
   Htrunc      -- truncate a dataset to a length
   Hsync       -- sync file with memory
   Hcache      -- set low-level caching for a file
   HDvalidfid  -- check if a file ID is valid
   HDerr       --  Closes a file and return FAIL.
   Hsetacceesstype -- set the I/O access type (serial, parallel, ...)
                       of a data element
   Hgetlibversion  -- return version info on current HDF library
   Hgetfileversion -- return version info on HDF file
   HPgetdiskblock  -- Get the offset of a free block in the file.
   HPfreediskblock -- Release a block in a file to be reused.
   HDread_drec -- reads a description record
   HDcheck_empty   -- determines if an element has been written with data
   HDget_special_info -- get information about a special element
   HDset_special_info -- reset information about a special element
   HDspecial_type -- return the special type if the given element is special

   File Memory Pool routines
   -------------------------
   Hmpset  -- set pagesize and maximum number of pages to cache on next open/create
   Hmpget  -- get last pagesize and max number of pages cached for open/create

   Special Tag routines
   -------------------
   HDmake_special_tag --
   HDis_special_tag   --
   HDbaset_tag        --

   Macintosh specific Routines(unbuffered C I/O stubs on top of Mac toolbox)
   --------------------------
   mopen  --
   mclose --
   mread  --
   mwrite --
   mlsekk --

   LOCAL ROUTINES
   HIextend_file   -- extend file to current length
   HIget_function_table -- create special function table
   HIgetspinfo          -- return special info
   HIunlock             -- unlock a previously locked file record
   HIget_filerec_node   -- locate a filerec for a new file
   HIrelease_filerec_node -- release a filerec
   HIvalid_magic        -- verify the magic number in a file
   HIget_access_rec     -- allocate a new access record
   HIupdate_version     -- determine whether new version tag should be written
   HIread_version       -- reads a version tag from a file
   + */

#include <errno.h>

#include "hdfi.h"
#include "hfile.h"
#include "glist.h" /* for double-linked lists, stacks and queues */

/*--------------------- Locally defined Globals -----------------------------*/

/* The default state of the file DD caching */
static intn default_cache = TRUE;

/* Whether we've installed the library termination function yet for this interface */
static intn          library_terminate = FALSE;
static Generic_list *cleanup_list      = NULL;

/* Whether to install the atexit routine */
static intn install_atexit = TRUE;

/* Pointer to the access record node free list */
static accrec_t *accrec_free_list = NULL;

#ifdef DISKBLOCK_DEBUG
const uint8 diskblock_header[4] = {0xde, 0xad, 0xbe, 0xef};
const uint8 diskblock_tail[4]   = {0xfe, 0xeb, 0xda, 0xed};
#endif

/*--------------------- Externally defined Globals --------------------------*/
/* Function tables declarations.  These function tables contain pointers
   to functions that help access each type of special element. */

/* Functions for accessing the linked block special
   data element.  For definition of the linked block, see hblocks.c. */
extern funclist_t linked_funcs;

/* Functions for accessing external data elements, or data
   elements that are in some other files.  For definition of the external
   data element, see hextelt.c. */
extern funclist_t ext_funcs;

/* Functions for accessing compressed data elements.
   For definition of the compressed data element, see hcomp.c. */
extern funclist_t comp_funcs;

/* Functions for accessing chunked data elements.
   For definition of the chunked data element, see hchunk.c. */
#include "hchunks.h"

/* Functions for accessing buffered data elements.
   For definition of the buffered data element, see hbuffer.c. */
extern funclist_t buf_funcs;

/* Functions for accessing compressed raster data elements.
   For definition of the compressed raster data element, see hcompri.c. */
extern funclist_t cr_funcs;

/* Table of these function tables for accessing special elements.  The first
   member of each record is the special code for that type of data element. */
functab_t functab[] = {
    {SPECIAL_LINKED, &linked_funcs},
    {SPECIAL_EXT, &ext_funcs},
    {SPECIAL_COMP, &comp_funcs},
    {SPECIAL_CHUNKED, &chunked_funcs},
    {SPECIAL_BUFFERED, &buf_funcs},
    {SPECIAL_COMPRAS, &cr_funcs},
    {0, NULL} /* terminating record; add new record */
              /* before this line */
};

/*
 ** Declaration of private functions.
 */
static intn HIunlock(filerec_t *file_rec);

static filerec_t *HIget_filerec_node(const char *path);

static intn HIrelease_filerec_node(filerec_t *file_rec);

static intn HIvalid_magic(hdf_file_t file);

static intn HIextend_file(filerec_t *file_rec);

static funclist_t *HIget_function_table(accrec_t *access_rec);

static intn HIupdate_version(int32);

static intn HIread_version(int32);

static intn HIcheckfileversion(int32 file_id);

static intn HIsync(filerec_t *file_rec);

static intn HIstart(void);

/*--------------------------------------------------------------------------
NAME
   Hopen -- Opens or creates an HDF file.
USAGE
   int32 Hopen(path, access, ndds)
   char *path;             IN: Name of file to be opened.
   int access;             IN: DFACC_READ, DFACC_WRITE, DFACC_CREATE
                                or any bitwise-or of the above.
   int16 ndds;             IN: Number of dds in a block if this
                                file needs to be created.
RETURNS
   On success returns file id, on failure returns -1.
DESCRIPTION
   Opens an HDF file.  Returns the the file ID on success, or -1
   on failure.

   Access equals DFACC_CREATE means discard existing file and
   create new file.  If access is a bitwise-or of DFACC_CREATE
   and anything else, the file is only created if it does not
   exist.  DFACC_WRITE set in access also means that if the file
   does not exist, it is created.  DFACC_READ is assumed to be
   implied even if it is not set.  DFACC_CREATE implies
   DFACC_WRITE.

   If the file is already opened and access is DFACC_CREATE:
   error DFE_ALROPEN.
   If the file is already opened, the requested access contains
   DFACC_WRITE, and previous open does not allow write: attempt
   to reopen the file with write permission.

   On successful exit,
   * file_rec members are filled in correctly.
   * file is opened with the relevant permission.
   * information about dd's are set up in memory.
   For new file, in addition,
   * the file headers and initial information are set up properly.

--------------------------------------------------------------------------*/
int32
Hopen(const char *path, intn acc_mode, int16 ndds)
{
    filerec_t *file_rec  = NULL; /* File record */
    int        vtag      = 0;    /* write version tag? */
    int32      fid       = FAIL; /* File ID */
    int32      ret_value = SUCCEED;

    /* Clear errors and check args and all the boring stuff. */
    HEclear();
    if (!path || ((acc_mode & DFACC_ALL) != acc_mode))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if (HIstart() == FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    /* Get a space to put the file information.
     * HIget_filerec_node() also copies path into the record. */
    if ((file_rec = HIget_filerec_node(path)) == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL); /* The slots are full. */

    if (file_rec->refcount) { /* File is already opened, check that permission is okay. */
        /* If this request is to create a new file and file is still
         * in use, return error. */
        if (acc_mode == DFACC_CREATE)
            HGOTO_ERROR(DFE_ALROPEN, FAIL);

        if ((acc_mode & DFACC_WRITE) && !(file_rec->access & DFACC_WRITE)) {
            /* If the request includes writing, and if original open does not
               provide for write, then try to reopen file for writing.
               This cannot be done on OS (such as the SXOS) where only one
               open is allowed per file at any time. */
            hdf_file_t f;

            /* Sync. the file before throwing away the old file handle */
            if (HIsync(file_rec) == FAIL)
                HGOTO_ERROR(DFE_INTERNAL, FAIL);

            f = (hdf_file_t)HI_OPEN(file_rec->path, acc_mode);
            if (OPENERR(f))
                HGOTO_ERROR(DFE_DENIED, FAIL);

            /* Replace file_rec->file with new file pointer and
               close old one. */
            if (HI_CLOSE(file_rec->file) == FAIL) {
                HI_CLOSE(f);
                HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
            }
            file_rec->file      = f;
            file_rec->f_cur_off = 0;
            file_rec->last_op   = H4_OP_UNKNOWN;
        }

        /* There is now one more open to this file. */
        file_rec->refcount++;
    }
    else {
        /* Flag to see if file is new and needs to be set up. */
        intn new_file = FALSE;

        /* Open the file, fill in the blanks and all the good stuff. */
        if (acc_mode != DFACC_CREATE) { /* try to open existing file */
            file_rec->file = (hdf_file_t)HI_OPEN(file_rec->path, acc_mode);
            if (OPENERR(file_rec->file)) {
                if (acc_mode & DFACC_WRITE) {
                    /* Seems like the file is not there, try to create it. */
                    new_file = TRUE;
                }
                else
                    HGOTO_ERROR(DFE_BADOPEN, FAIL);
            }
            else {
                /* Open existing file successfully. */
                file_rec->access = acc_mode | DFACC_READ;

                /* Check to see if file is a HDF file. */
                if (!HIvalid_magic(file_rec->file)) {
                    HI_CLOSE(file_rec->file);
                    HGOTO_ERROR(DFE_NOTDFFILE, FAIL);
                }

                file_rec->f_cur_off = 0;
                file_rec->last_op   = H4_OP_UNKNOWN;
                /* Read in all the relevant data descriptor records. */
                if (HTPstart(file_rec) == FAIL) {
                    HI_CLOSE(file_rec->file);
                    HGOTO_ERROR(DFE_BADOPEN, FAIL);
                }
            }
        }
        /* do *not* use else here */
        if (acc_mode == DFACC_CREATE || new_file) { /* create the file */
                                                    /* make user we get a version tag */
            vtag = 1;

            file_rec->file = (hdf_file_t)HI_CREATE(file_rec->path);
            if (OPENERR(file_rec->file)) {
                /* check if the failure was due to "too many open files" */
                if (errno == EMFILE) {
                    HGOTO_ERROR(DFE_TOOMANY, FAIL);
                }
                else
                    HGOTO_ERROR(DFE_BADOPEN, FAIL);
            }

            file_rec->f_cur_off = 0;
            file_rec->last_op   = H4_OP_UNKNOWN;

            /* set up the newly created (and empty) file with
               the magic cookie and initial data descriptor records */
            if (HP_write(file_rec, HDFMAGIC, MAGICLEN) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);

            if (HI_FLUSH(file_rec->file) == FAIL) /* flush the cookie */
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);

            if (HTPinit(file_rec, ndds) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);

            file_rec->maxref = 0;
            file_rec->access = new_file ? acc_mode | DFACC_READ : DFACC_ALL;
        }
        file_rec->refcount = 1;
        file_rec->attach   = 0;

        /* currently, default is caching OFF */
        file_rec->cache = default_cache;
        file_rec->dirty = 0; /* mark all dirty flags off to start */
    }                        /* end else */

    file_rec->version_set = FALSE;

    if ((fid = HAregister_atom(FIDGROUP, file_rec)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* version tags */
    if (vtag == 1) {
        if (HIupdate_version(fid) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */
    else {
        HIread_version(fid); /* ignore return code in case the file doesn't have a version */
    }                        /* end else */

    ret_value = fid;

done:
    if (ret_value == FAIL) { /* Error condition cleanup */
        if (fid != FAIL)
            HAremove_atom(fid);

        /* Chuck the file record we've built */
        if (file_rec != NULL && file_rec->refcount == 0)
            HIrelease_filerec_node(file_rec);
    }

    return ret_value;
} /* Hopen */

/*--------------------------------------------------------------------------
NAME
   Hclose -- close HDF file
USAGE
   intn Hclose(id)
   int id;                 IN: the file id to be closed
RETURNS
   returns SUCCEED (0) if successful and FAIL (-1) if failed.
DESCRIPTION
   closes an HDF file given the file id.  Id is first validated.  If
   there are still access objects attached to the file, an error is
   returned and the file is not closed.

--------------------------------------------------------------------------*/
intn
Hclose(int32 file_id)
{
    filerec_t *file_rec; /* file record pointer */
    intn       ret_value = SUCCEED;

    /* Clear errors and check args and all the boring stuff. */
    HEclear();

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* version tags */
    if ((file_rec->refcount > 0) && (file_rec->version.modified == 1))
        HIupdate_version(file_id);

    /* decrease the reference count */
    if (--file_rec->refcount == 0) {
        /* if file reference count is zero but there are still attached
           access elts, reject this close. */
        if (file_rec->attach > 0) {
            file_rec->refcount++;
            HEreport("There are still %d active aids attached", file_rec->attach);
            HGOTO_ERROR(DFE_OPENAID, FAIL);
        } /* end if */

        /* before closing file, check whether to flush file info */
        if (HIsync(file_rec) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* otherwise, nothing should still be using this file, close it */
        /* ignore any close error */
        HI_CLOSE(file_rec->file);

        if (HTPend(file_rec) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        if (HIrelease_filerec_node(file_rec))
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */

    if (HAremove_atom(file_id) == NULL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
    return ret_value;
} /* Hclose */

/*--------------------------------------------------------------------------
NAME
   Hexist -- locate an object in an HDF file
USAGE
   intn Hexist(file_id ,search_tag, search_ref)
   int32 file_id;           IN: file ID to search in
   uint16 search_tag;       IN: the tag to search for
                                                                (can be DFTAG_WILDCARD)
   uint16 search_ref;       IN: ref to search for
                                                                (can be DFREF_WILDCARD)
RETURNS
   returns SUCCEED (0) if successful and FAIL (-1) otherwise
DESCRIPTION
   Simple interface to Hfind which just determines if a given
   tag/ref pair exists in a file.  Wildcards apply.
GLOBAL VARIABLES
COMMENTS, BUGS, ASSUMPTIONS
        Hfind() does all validity checking, this is just a _very_
        simple wrapper around it.
EXAMPLES
REVISION LOG
--------------------------------------------------------------------------*/
intn
Hexist(int32 file_id, uint16 search_tag, uint16 search_ref)
{
    uint16 find_tag = 0, find_ref = 0;
    int32  find_offset, find_length;
    intn   ret_value;

    ret_value = (Hfind(file_id, search_tag, search_ref, &find_tag, &find_ref, &find_offset, &find_length,
                       DF_FORWARD));
    return ret_value;
} /* end Hexist() */

/*--------------------------------------------------------------------------
NAME
   Hinquire -- inquire stats of an access elt
USAGE
   intn Hinquire(access_id, pfile_id, ptag, pref, plength,
                                   poffset, pposn, paccess, pspecial)
   int access_id;          IN: id of an access elt
   int32 *pfile_id;        OUT: file id
   uint16 *ptag;           OUT: tag of the element pointed to
   uint16 *pref;           OUT: ref of the element pointed to
   int32 *plength;         OUT: length of the element pointed to
   int32 *poffset;         OUT: offset of elt in the file
   int32 *pposn;           OUT: position pointed to within the data elt
   int16 *paccess;         OUT: the access type of this access elt
   int16 *pspecial;        OUT: special code
RETURNS
   returns SUCCEED (0) if the access elt points to some data element,
   otherwise FAIL (-1)
DESCRIPTION
   Inquire statistics of the data element pointed to by access elt and
   the access elt.  The access type is set if the access_id is valid even
   if FAIL is returned.  If access_id is not valid then access is set to
   zero (0). If statistic is not needed, pass NULL for the appropriate
   value.

--------------------------------------------------------------------------*/
intn
Hinquire(int32 access_id, int32 *pfile_id, uint16 *ptag, uint16 *pref, int32 *plength, int32 *poffset,
         int32 *pposn, int16 *paccess, int16 *pspecial)
{
    accrec_t *access_rec; /* access record */
    intn      ret_value = SUCCEED;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if special elt, let special functions handle it */
    if (access_rec->special) {
        ret_value = (int)(*access_rec->special_func->inquire)(access_rec, pfile_id, ptag, pref, plength,
                                                              poffset, pposn, paccess, pspecial);
        goto done;
    }
    if (pfile_id != NULL)
        *pfile_id = access_rec->file_id;
    /* Get the relevant DD information */
    if (HTPinquire(access_rec->ddid, ptag, pref, poffset, plength) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if (pposn != NULL)
        *pposn = access_rec->posn;
    if (paccess != NULL)
        *paccess = (int16)access_rec->access;
    if (pspecial != NULL)
        *pspecial = 0;

done:
    return ret_value;
} /* end Hinquire */

/* ----------------------------- Hfidinquire ----------------------------- */
/*
** NAME
**      Hfidinquire --- Inquire about a file ID
** USAGE
**      int Hfidinquire(file_id)
**      int32 file_id;          IN: handle of file
**      char  *path;            OUT: path of file
**      int32 mode;             OUT: mode file is opened with
** RETURNS
**      returns SUCCEED (0) if successful and FAIL (-1) if failed.
** DESCRIPTION
** GLOBAL VARIABLES
** COMMENTS, BUGS, ASSUMPTIONS
--------------------------------------------------------------------------*/
intn
Hfidinquire(int32 file_id, char **fname, intn *faccess, intn *attach)
{
    filerec_t *file_rec;
    intn       ret_value = SUCCEED;

    HEclear();

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_BADACC, FAIL);

    *fname   = file_rec->path;
    *faccess = file_rec->access;
    *attach  = file_rec->attach;

done:
    return ret_value;
} /* Hfidinquire */

/*--------------------------------------------------------------------------

NAME
   Hstartread -- locate and position a read access elt on a tag/ref
USAGE
   int32 Hstartread(fileid, tag, ref)
   int fileid;             IN: id of file to attach access element to
   int tag;                IN: tag to search for
   int ref;                IN: ref to search for
RETURNS
   returns id of access element if successful, otherwise FAIL (-1)
DESCRIPTION
   Searches the DD's for a particular tag/ref combination.  The
   searching starts from the head of the DD list.  Wildcards can be
   used for tag or ref (DFTAG_WILDCARD, DFREF_WILDCARD) and they match
   any values.  If the search is successful, the access elt is
   positioned to the start of that tag/ref, otherwise it is an error.
   An access element is created and attached to the file.

--------------------------------------------------------------------------*/
int32
Hstartread(int32 file_id, uint16 tag, uint16 ref)
{
    int32 ret; /* AID to return */
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* Call Hstartaccess with the modified base tag */
    if ((ret = Hstartaccess(file_id, BASETAG(tag), ref, DFACC_READ)) == FAIL)
        HGOTO_ERROR(DFE_BADAID, FAIL);

    ret_value = ret;

done:
    return ret_value;
} /* Hstartread() */

/*--------------------------------------------------------------------------
NAME
   Hnextread -- locate and position a read access elt on tag/ref.
USAGE
   intn Hnextread(access_id, tag, ref, origin)
   int32 access_id;         IN: id of a READ access elt
   uint16 tag;              IN: the tag to search for
   uint16 ref;              IN: ref to search for
   int origin;              IN: from where to start searching
RETURNS
   returns SUCCEED (0) if successful and FAIL (-1) otherwise
DESCRIPTION
   Searches for the `next' DD that fits the tag/ref.  Wildcards
   apply.  If origin is DF_START, search from start of DD list,
   if origin is DF_CURRENT, search from current position, otherwise
   origin should be DF_END which searches from end of file.
   If the search is successful, then the access elt
   is positioned at the start of that tag/ref, otherwise, it is not
   modified.
COMMENTS, BUGS, ASSUMPTIONS
DF_END _not_ supported yet!

--------------------------------------------------------------------------*/
intn
Hnextread(int32 access_id, uint16 tag, uint16 ref, intn origin)
{
    filerec_t *file_rec;                 /* file record */
    accrec_t  *access_rec;               /* access record */
    uint16     new_tag = 0, new_ref = 0; /* new tag & ref to access */
    int32      new_off, new_len;         /* offset & length of new tag & ref */
    intn       ret_value = SUCCEED;

    /* clear error stack and check validity of the access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || !(access_rec->access & DFACC_READ) ||
        (origin != DF_START && origin != DF_CURRENT)) /* DF_END is NOT supported yet !!!! */
        HGOTO_ERROR(DFE_ARGS, FAIL);

    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /*
     * if access record used to point to an external element we
     * need to close the file before moving on
     */
    if (access_rec->special) {
        switch (access_rec->special) {
            case SPECIAL_LINKED:
                if (HLPcloseAID(access_rec) == FAIL)
                    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
                break;

            case SPECIAL_EXT:
                if (HXPcloseAID(access_rec) == FAIL)
                    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
                break;

            case SPECIAL_COMP:
                if (HCPcloseAID(access_rec) == FAIL)
                    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
                break;

            case SPECIAL_CHUNKED:
                if (HMCPcloseAID(access_rec) == FAIL)
                    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
                break;

            case SPECIAL_BUFFERED:
                if (HBPcloseAID(access_rec) == FAIL)
                    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
                break;

            default: /* do nothing for other cases currently */
                break;
        } /* end switch */
    }

    if (origin == DF_START) { /* set up variables to start searching from beginning of file */
        new_tag = 0;
        new_ref = 0;
    }
    else { /* origin == CURRENT */
           /* set up variables to start searching from the current position */
        /* Get the old tag & ref */
        if (HTPinquire(access_rec->ddid, &new_tag, &new_ref, NULL, NULL) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    }

    /* go look for the dd */
    if (Hfind(access_rec->file_id, tag, ref, &new_tag, &new_ref, &new_off, &new_len, DF_FORWARD) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Let go of the previous DD id */
    if (HTPendaccess(access_rec->ddid) == FAIL)
        HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

    /* found, so update the access record */
    if ((access_rec->ddid = HTPselect(file_rec, new_tag, new_ref)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    access_rec->appendable = FALSE; /* start data as non-appendable */
    if (new_len == INVALID_OFFSET && new_off == INVALID_LENGTH)
        access_rec->new_elem = TRUE;
    else
        access_rec->new_elem = FALSE;

    /* If special element act upon it accordingly */
    if (HTPis_special(access_rec->ddid)) {
        int32 spec_aid;

        /* special element, call special function to handle */
        if ((access_rec->special_func = HIget_function_table(access_rec)) == NULL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* decrement "attach" to the file_rec */
        HIunlock(file_rec);
        if ((spec_aid = (*access_rec->special_func->stread)(access_rec)) != FAIL) {
            HAremove_atom(spec_aid); /* This is a gross hack! -QAK */
            HGOTO_DONE(SUCCEED);
        } /* end if */
        else {
            HGOTO_DONE(FAIL);
        } /* end if */
    }

    access_rec->special = 0;
    access_rec->posn    = 0;

done:
    return ret_value;
} /* end Hnextread() */

/*--------------------------------------------------------------------------
NAME
   Hstartwrite -- set up a WRITE access elt for a write
USAGE
   int32 Hstartwrite(fileid, tag, ref, len)
   int fileid;             IN: id of file to write to
   int tag;                IN: tag to write to
   int ref;                IN: ref to write to
   long length;            IN: the length of the data element
RETURNS
   returns id of access element if successful and FAIL otherwise
DESCRIPTION
   Set up a WRITE access elt to write out a data element.  The DD list
   of the file is searched first.  If the tag/ref is found, it is
   NOT replaced - the seek position is presumably at 0.
                        If it does not exist, it is created.

--------------------------------------------------------------------------*/
int32
Hstartwrite(int32 file_id, uint16 tag, uint16 ref, int32 length)
{
    accrec_t *access_rec; /* access record */
    int32     ret;        /* AID to return */
    int32     ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* Call Hstartaccess with the modified base tag */
    if ((ret = Hstartaccess(file_id, BASETAG(tag), ref, DFACC_RDWR)) == FAIL)
        HGOTO_ERROR(DFE_BADAID, FAIL);

    access_rec = HAatom_object(ret);

    /* if new element set the length */
    if (access_rec->new_elem && (Hsetlength(ret, length) == FAIL)) {
        Hendaccess(ret);
        HGOTO_ERROR(DFE_BADLEN, FAIL);
    } /* end if */

    ret_value = ret;

done:
    return ret_value;
} /* end Hstartwrite */

/*--------------------------------------------------------------------------
NAME
   Hstartaccess -- set up a access elt for either reading or writing
USAGE
   int32 Hstartaccess(fileid, tag, ref, flags)
   int32 fileid;           IN: id of file to read/write to
   uint16 tag;             IN: tag to read/write to
   uint16 ref;             IN: ref to read/write to
   uint32 flags;           IN: access flags for the data element
RETURNS
   returns id of access element if successful and FAIL otherwise
DESCRIPTION
   Start access to data element for read or write access.  The DD list
   of the file is searched first.  If the tag/ref is found, it is
   NOT replaced - the seek position is presumably at 0.
                        If it does not exist, it is created.

--------------------------------------------------------------------------*/
int32
Hstartaccess(int32 file_id, uint16 tag, uint16 ref, uint32 flags)
{
    intn       ddnew      = FALSE;       /* is the dd a new one? */
    filerec_t *file_rec   = NULL;        /* file record */
    accrec_t  *access_rec = NULL;        /* access record */
    uint16     new_tag = 0, new_ref = 0; /* new tag & ref to access */
    int32      new_off, new_len;         /* offset & length of new tag & ref */
    int32      ret_value = SUCCEED;

    /* clear error stack and check validity of file id */
    HEclear();

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* If writing, can we write to this file? */
    if ((flags & DFACC_WRITE) && !(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* get empty slot in access records */
    access_rec = HIget_access_rec();
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* set up access record to look for the dd */
    access_rec->file_id = file_id;
    if (flags & DFACC_APPENDABLE)
        access_rec->appendable = TRUE; /* start data as appendable */
    else
        access_rec->appendable = FALSE; /* start data as non-appendable */

    /* set the default values for block size and number of blocks for use in */
    /* linked-block creation/conversion; they can be changed by the user via */
    /* VSsetblocksize and VSsetnumblocks - BMR (bug #267 - June 2001) */
    access_rec->block_size = HDF_APPENDABLE_BLOCK_LEN;
    access_rec->num_blocks = HDF_APPENDABLE_BLOCK_NUM;

    access_rec->special_info = NULL; /* reset */

    /* if the DFACC_CURRENT flag is set, start searching for the tag/ref from */
    /* the current location in the DD list */
    if (flags & DFACC_CURRENT || Hfind(access_rec->file_id, tag, ref, &new_tag, &new_ref, &new_off, &new_len,
                                       DF_FORWARD) == FAIL) { /* not in DD list */
        new_tag = tag;
        new_ref = ref;
        new_off = INVALID_OFFSET;
        new_len = INVALID_LENGTH;
    }

    /* get DD id for tag/ref if in DD list using 'new_tag' and 'new_ref' */
    if ((access_rec->ddid = HTPselect(file_rec, new_tag, new_ref)) == FAIL) { /* not in DD list */
        /* can't create data elements with only read access */
        if (!(flags & DFACC_WRITE))
            HGOTO_ERROR(DFE_NOMATCH, FAIL);

        /* dd not found, so have to create new element */
        if ((access_rec->ddid = HTPcreate(file_rec, new_tag, new_ref)) == FAIL)
            HGOTO_ERROR(DFE_NOFREEDD, FAIL);

        ddnew = TRUE; /* mark as new element */
    }
    else /* tag/ref already exists in DD list. */
    {    /* need to update the access_rec block and idx */

        /* If the tag we were looking up is special, and we aren't looking */
        /* for the actual special element information, then use special */
        /* element access to the data... -QAK */
        if (!SPECIALTAG(tag) &&
            HTPis_special(access_rec->ddid) ==
                TRUE) { /* found, if this elt is special, let special function handle it */

            /* get special function table for element */
            access_rec->special_func = HIget_function_table(access_rec);
            if (access_rec->special_func == NULL)
                HGOTO_ERROR(DFE_INTERNAL, FAIL);

            /* call appropriate special startread/startwrite fcn */
            if (!(flags & DFACC_WRITE))
                ret_value = (*access_rec->special_func->stread)(access_rec);
            else
                ret_value = (*access_rec->special_func->stwrite)(access_rec);

            goto done; /* we are done since the special fcn should take
                          of everything. */

        } /* end if special */
    }     /* end else tag/ref exists */

    /* Need to check if the "new" element was written to the file without */
    /* it's length being set.  If that was the case, the offset and length */
    /* will be marked as invalid, and therefore we should mark it as "new" */
    /* again when the element is re-opened -QAK */
    if (!ddnew && new_off == INVALID_OFFSET && new_len == INVALID_LENGTH)
        ddnew = TRUE; /* mark as new element */

    /* update the access record, and the file record */
    access_rec->posn     = 0;
    access_rec->access   = flags; /* keep the access flags around */
    access_rec->file_id  = file_id;
    access_rec->special  = 0;     /* not special */
    access_rec->new_elem = ddnew; /* set the flag indicating whether
                                     this elt is new */
    file_rec->attach++;           /* increment number of elts attached to file */

    /* check current maximum ref for file and update if necessary */
    if (new_ref > file_rec->maxref)
        file_rec->maxref = new_ref;

    /*
     * If this is the first time we are writing to this file
     *    update the version tags as needed */
    if (!file_rec->version_set)
        HIcheckfileversion(file_id);

    ret_value = HAregister_atom(AIDGROUP, access_rec);

done:
    if (ret_value == FAIL) { /* Error condition cleanup */
        if (access_rec != NULL)
            HIrelease_accrec_node(access_rec);
    }

    return ret_value;
} /* end Hstartaccess */

/*--------------------------------------------------------------------------
NAME
   Hsetlength -- set the length of a new HDF element
USAGE
   intn Hsetlength(aid, length)
   int32 aid;           IN: id of element to set the length of
   int32 length;        IN: the length of the element
RETURNS
   SUCCEED/FAIL
DESCRIPTION
   Sets the length of a new data element.  This function is only valid
   when called after Hstartaccess on a new data element and before
   any data is written to that element.

--------------------------------------------------------------------------*/
intn
Hsetlength(int32 aid, int32 length)
{
    accrec_t  *access_rec; /* access record */
    filerec_t *file_rec;   /* file record */
    int32      offset;     /* offset of this data element in file */
    intn       ret_value = SUCCEED;

    /* clear error stack and check validity of file id */
    HEclear();

    if ((access_rec = HAatom_object(aid)) == NULL) /* get the access_rec pointer */
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check whether we are allowed to change the length */
    if (access_rec->new_elem != TRUE)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* place the data element at the end of the file and record its offset */
    if ((offset = HPgetdiskblock(file_rec, length, FALSE)) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);

    /* fill in dd record updating the offset and length of the element */
    if (HTPupdate(access_rec->ddid, offset, length) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* turn off the "new" flag now that we have a length and offset */
    access_rec->new_elem = FALSE;

done:
    return ret_value;
} /* end Hsetlength */

/*--------------------------------------------------------------------------
NAME
   Happendable -- Allow a data set to be appended to without the
        use of linked blocks
USAGE
   intn Happendable(aid)
   int32 aid;              IN: aid of the dataset to make appendable
RETURNS
   returns 0 if dataset is allowed to be appendable, FAIL otherwise
DESCRIPTION
   If a dataset is at the end of a file, allow Hwrite()s to write
   past the end of a file.  Allows expanding datasets without the use
   of linked blocks.

--------------------------------------------------------------------------*/
intn
Happendable(int32 aid)
{
    accrec_t *access_rec; /* access record */
    intn      ret_value = SUCCEED;

    /* clear error stack and check validity of file id */
    HEclear();
    if ((access_rec = HAatom_object(aid)) == NULL) /* get the access_rec pointer */
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* just indicate that the data should be appendable, and only convert */
    /* it when actually asked to modify the data */
    access_rec->appendable = TRUE;

done:
    return ret_value;
} /* end Happendable */

/*--------------------------------------------------------------------------
NAME
   HPisappendable -- Check whether a data set can be appended to without the
        use of linked blocks
USAGE
   intn HPisappendable(aid)
   int32 aid;              IN: aid of the dataset to check appendable
RETURNS
   returns SUCCEED if dataset is allowed to be appendable, FAIL otherwise
DESCRIPTION
   If a dataset is at the end of a file, allow Hwrite()s to write
   past the end of a file.  Allows expanding datasets without the use
   of linked blocks.

--------------------------------------------------------------------------*/
intn
HPisappendable(int32 aid)
{
    accrec_t  *access_rec; /* access record */
    filerec_t *file_rec;   /* file record */
    int32      data_len;   /* length of the data we are checking */
    int32      data_off;   /* offset of the data we are checking */
    intn       ret_value = SUCCEED;

    /* clear error stack and check validity of file id */
    HEclear();
    if ((access_rec = HAatom_object(aid)) == NULL) /* get the access_rec pointer */
        HGOTO_ERROR(DFE_ARGS, FAIL);

    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the offset and length of the dataset */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_len, &data_off) == FAIL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* dataset at end? */
    if (data_len + data_off == file_rec->f_end_off)
        ret_value = SUCCEED;
    else
        ret_value = FAIL;

done:
    return ret_value;
} /* end HPisappendable */

/*--------------------------------------------------------------------------

NAME
   Hseek -- position an access element to an offset in data element
USAGE
   intn Hseek(access_id, offset, origin)
   int32 access_id;        IN: id of access element
   long offset;            IN: offset to seek to
   int origin;             IN: position to seek from by offset, 0: from
                                                   beginning; 1: current position; 2: end of
                                                   data element
RETURNS
   returns FAIL (-1) if fail, SUCCEED (0) otherwise.
DESCRIPTION
   Sets the position of an access element in a data element so that the
   next Hread or Hwrite will start from that position.  origin
   determines the position from which the offset should be added.  This
   routine fails if the access elt is not associated with any data
   element and if the seeked position is outside of the data element.

--------------------------------------------------------------------------*/
intn
Hseek(int32 access_id, int32 offset, intn origin)
{
    accrec_t  *access_rec;          /* access record */
    intn       old_offset = offset; /* save for later potential use */
    filerec_t *file_rec;            /* file record */
    int32      data_len;            /* length of the data we are checking */
    int32      data_off;            /* offset of the data we are checking */
    intn       ret_value = SUCCEED;

    /* clear error stack and check validity of this access id */
    HEclear();

    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || (origin != DF_START && origin != DF_CURRENT && origin != DF_END))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if special elt, use special function */
    if (access_rec->special) { /* yes, call special seek function with proper args */
        ret_value = (intn)(*access_rec->special_func->seek)(access_rec, offset, origin);
        goto done;
    }

    /* Get the data's offset & length */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_off, &data_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* calculate real offset based on the origin */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += data_len;

    /* If we aren't moving the access records position, bypass the next bit of code */
    /* This allows seeking to offset zero in not-yet-existent data elements -QAK */
    if (offset == access_rec->posn)
        HGOTO_DONE(SUCCEED);

    /* Check the range */
    if (offset < 0 || (!access_rec->appendable && offset > data_len)) {
        HEreport("Tried to seek to %d (object length:  %d)", offset, data_len);
        HGOTO_ERROR(DFE_BADSEEK, FAIL);
    }

    /* check if element is appendable and writing past current element length */
    if (access_rec->appendable && offset >= data_len) { /* yes */
        file_rec = HAatom_object(access_rec->file_id);

        /* check if we are at end of file */
        if (data_len + data_off !=
            file_rec->f_end_off) { /* nope, so try to convert element into linked-block element */
            if (HLconvert(access_id, access_rec->block_size, access_rec->num_blocks) == FAIL) {
                access_rec->appendable = FALSE;
                HEreport("Tried to seek to %d (object length:  %d)", offset, data_len);
                HGOTO_ERROR(DFE_BADSEEK, FAIL);
            } /* end if */
            else
            /* successfully converted the element into a linked block */
            /* now loop back and actually seek to the correct position */
            {
                if (Hseek(access_id, old_offset, origin) == FAIL)
                    HGOTO_ERROR(DFE_BADSEEK, FAIL);
            } /* end else */
        }     /* end if */
    }         /* end if */

    /* set the new position */
    access_rec->posn = offset;

done:
    return ret_value;
} /* Hseek() */

/*--------------------------------------------------------------------------

NAME
   Htell -- report position of an access element in a data element
USAGE
   int32 Htell(access_id)
       int32 access_id;        IN: id of access element
RETURNS
   returns FAIL (-1) on error, offset in data element otherwise
DESCRIPTION
    Reports the offset in bytes of an AID in a data element.  Analogous to
    ftell().

--------------------------------------------------------------------------*/
int32
Htell(int32 access_id)
{
    accrec_t *access_rec; /* access record */
    int32     ret_value = SUCCEED;

    /* clear error stack and check validity of this access id */
    HEclear();

    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* return the offset in the AID */
    ret_value = (int32)access_rec->posn;

done:
    return ret_value;
} /* Htell() */

/*--------------------------------------------------------------------------
NAME
   Hread -- read the next segment from data element
USAGE
   int32 Hread(access_id, length, data)
   int32 access_id;        IN: id of READ access element
   int32 length;           IN: length of segment to read in
   char *data;             OUT: pointer to data array to read to
RETURNS
   returns length of segment actually read in if successful and FAIL
   (-1) otherwise
DESCRIPTION
   Read in the next segment in the data element pointed to by the
   access elt.  If length is zero or larger than the remaining bytes
   of the object, read until the end of the object.

--------------------------------------------------------------------------*/
int32
Hread(int32 access_id, int32 length, void *data)
{
    filerec_t *file_rec;   /* file record */
    accrec_t  *access_rec; /* access record */
    int32      data_len;   /* length of the data we are checking */
    int32      data_off;   /* offset of the data we are checking */
    int32      ret_value = SUCCEED;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || data == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Don't allow reading of "new" elements */
    if (access_rec->new_elem == TRUE)
        HGOTO_ERROR(DFE_READERROR, FAIL);

    /* special elt, so call special function */
    if (access_rec->special) { /* yes, call special read function with proper args */
        ret_value = (*access_rec->special_func->read)(access_rec, length, data);
        goto done; /* we are done */
    }

    /* check validity of file record */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* get the dd of this data elt */
    if (length < 0)
        HGOTO_ERROR(DFE_BADSEEK, FAIL);

    /* Get the data's offset & length */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_off, &data_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* seek to position to start reading and read in data */
    if (HPseek(file_rec, access_rec->posn + data_off) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);

    /* length == 0 means to read to end of element, */
    /* if read length exceeds length of elt, read till end of elt */
    if (length == 0 || length + access_rec->posn > data_len)
        length = data_len - access_rec->posn;

    /* read in data */
    if (HP_read(file_rec, data, length) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);

    /* move the position of the access record */
    access_rec->posn += length;

    ret_value = length;

done:
    return ret_value;
} /* Hread */

/*--------------------------------------------------------------------------
NAME
   Hwrite -- write next data segment to data element
USAGE
   int32 Hwrite(access_id, len, data)
   int32 access_id;        IN: id of WRITE access element
   int32 len;              IN: length of segment to write
   const char *data;       IN: pointer to data to write
RETURNS
   returns length of segment successfully written, FAIL (-1) otherwise
DESCRIPTION
   Write the data to data element where the last write or Hseek()
   stopped.  If the space reserved is less than the length to write,
   then only as much as can fit is written.  It is the responsibility
   of the user to insure that no two access elements are writing to the
   same data element.  It is possible to interlace writes to more than
   one data elements in the same file though.
   Calling with length == 0 is an error.

--------------------------------------------------------------------------*/
int32
Hwrite(int32 access_id, int32 length, const void *data)
{
    filerec_t *file_rec;   /* file record */
    accrec_t  *access_rec; /* access record */
    int32      data_len;   /* length of the data we are checking */
    int32      data_off;   /* offset of the data we are checking */
    int32      ret_value = SUCCEED;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || !(access_rec->access & DFACC_WRITE) || data == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if special elt, call special write function */
    if (access_rec->special) {
        ret_value = (*access_rec->special_func->write)(access_rec, length, data);
        goto done; /* we are done */
    }              /* end special */

    /* check validity of file record and get dd ptr */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* check for a "new" element and make it appendable if so.
       Does this mean every element is by default appendable? */
    if (access_rec->new_elem == TRUE) {
        Hsetlength(access_id, length); /* make the initial chunk of data */
        access_rec->appendable = TRUE; /* make it appendable */
    }                                  /* end if */

    /* get the offset and length of the element. This should have
       been set by Hstartwrite(). */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_off, &data_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* check validity of length and write data.
     NOTE: it is an error to attempt write past the end of the elt */
    if (length <= 0 || (!access_rec->appendable && length + access_rec->posn > data_len))
        HGOTO_ERROR(DFE_BADSEEK, FAIL);

    /* check if element is appendable and write length exceeds current
       data element length */
    if (access_rec->appendable && length + access_rec->posn > data_len) { /* yes */

        /* is data element at end of file?
           hmm. not sure about this condition. */
        if (data_len + data_off != file_rec->f_end_off) { /* nope, not at end of file. Try to promote to
                                                         linked-block element. */
            if (HLconvert(access_id, access_rec->block_size, access_rec->num_blocks) == FAIL) {
                access_rec->appendable = FALSE;
                HGOTO_ERROR(DFE_BADSEEK, FAIL);
            } /* end if */
              /* successfully converted the element into a linked block */
              /* now loop back and actually write the data out */
            if ((ret_value = Hwrite(access_id, length, data)) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
            goto done; /* we're finished, wrap things up */
        }              /* end if */

        /* Update the DD with the new length. Note argument of '-2' for
           the offset parameter means not to change the offset in the DD. */
        if (HTPupdate(access_rec->ddid, -2, access_rec->posn + length) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */

    /* seek and write data */
    if (HPseek(file_rec, access_rec->posn + data_off) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);

    if (HP_write(file_rec, data, length) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    /* update end of file pointer? */
    if (file_rec->f_cur_off > file_rec->f_end_off)
        file_rec->f_end_off = file_rec->f_cur_off;

    /* update position of access in elt */
    access_rec->posn += length;

    ret_value = length;

done:
    return ret_value;
} /* end Hwrite */

/*--------------------------------------------------------------------------
NAME
   HDgetc -- read a byte from data element
USAGE
   intn HDgetc(access_id)
   int access_id;          IN: id of READ access element

RETURNS
   returns byte read in from data if successful and FAIL
   (-1) otherwise

DESCRIPTION
        Calls Hread() to read a single byte and reports errors.

--------------------------------------------------------------------------*/
intn
HDgetc(int32 access_id)
{
    uint8 c         = (uint8)FAIL; /* character read in */
    intn  ret_value = SUCCEED;

    if (Hread(access_id, 1, &c) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);

    ret_value = (intn)c;

done:
    return ret_value;
} /* HDgetc */

/*--------------------------------------------------------------------------
NAME

USAGE
   intn HDputc(c,access_id)
   uint8 c;                 IN: byte to write out
   int32 access_id;         IN: id of WRITE access element

RETURNS
   returns byte written out to data if successful and FAIL
   (-1) otherwise

DESCRIPTION
   Calls Hwrite() to write a single byte and reports errors.

--------------------------------------------------------------------------*/
intn
HDputc(uint8 c, int32 access_id)
{
    intn ret_value = SUCCEED;

    if (Hwrite(access_id, 1, &c) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    ret_value = (intn)c;

done:
    return ret_value;
} /* HDputc */

/*--------------------------------------------------------------------------
NAME
   Hendaccess -- to dispose of an access element
USAGE
   intn Hendaccess(access_id)
   int32 access_id;          IN: id of access element to dispose of
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
DESCRIPTION
   Used to dispose of an access element.  If access elements are not
   disposed it will eventually become impossible to allocate new
   ones and close the file.

   If there are active aids Hclose will *NOT* close the file.  This
   is a very common problem when developing new code.

--------------------------------------------------------------------------*/
intn
Hendaccess(int32 access_id)
{
    filerec_t *file_rec;          /* file record */
    accrec_t  *access_rec = NULL; /* access record */
    intn       ret_value  = SUCCEED;

    /* clear error stack and check validity of access id */
    HEclear();
    if ((access_rec = HAremove_atom(access_id)) == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if special elt, call special function */
    if (access_rec->special) {
        ret_value = (*access_rec->special_func->endaccess)(access_rec);
        goto done;
    } /* end if */

    /* check validity of file record */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* update file and access records */
    if (HTPendaccess(access_rec->ddid) == FAIL)
        HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

    file_rec->attach--;
    HIrelease_accrec_node(access_rec);

done:
    if (ret_value == FAIL) { /* Error condition cleanup */
        if (access_rec != NULL)
            HIrelease_accrec_node(access_rec);
    }

    return ret_value;
} /* Hendaccess */

/*--------------------------------------------------------------------------
NAME
   Hgetelement -- read in a data element
USAGE
   int32 Hgetelement(file_id, tag, ref, data)
   int32 file_id;          IN: id of the file to read from
   int16 tag;              IN: tag of data element to read
   int16 ref;              IN: ref of data element to read
   char *data;             OUT: buffer to read into
RETURNS
   returns  number of bytes read if successful, FAIL (-1)
   otherwise
DESCRIPTION
   Read in a data element from a HDF file and puts it into buffer
   pointed to by data.  The space allocated for buffer is assumed to
   be large enough.

--------------------------------------------------------------------------*/
int32
Hgetelement(int32 file_id, uint16 tag, uint16 ref, uint8 *data)
{
    int32 access_id = FAIL; /* access record id */
    int32 length;           /* length of this elt */
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* get the access record, get the length of the elt, read in data,
     and dispose of access record */
    if ((access_id = Hstartread(file_id, tag, ref)) == FAIL)
        HGOTO_ERROR(DFE_NOMATCH, FAIL);

    if ((length = Hread(access_id, (int32)0, data)) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);

    if (Hendaccess(access_id) == FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    ret_value = length;

done:
    if (ret_value == FAIL) { /* Error condition cleanup */
        if (access_id != FAIL)
            Hendaccess(access_id);
    }

    return ret_value;
} /* Hgetelement() */

/*--------------------------------------------------------------------------
NAME
   Hputelement -- writes a data element
USAGE
   int Hputelement(fileid, tag, ref, data, length)
   int32 fileid;             IN: id of file
   int16 tag;                IN: tag of data element to put
   int16 ref;                IN: ref of data element to put
   char *data;               IN: pointer to buffer
   int32 length;             IN: length of data
RETURNS
   returns length of bytes written if successful and FAIL (-1)
   otherwise
DESCRIPTION
   Writes a data element or replaces an existing data element
   in an HDF file.  Uses Hwrite and its associated routines.

--------------------------------------------------------------------------*/
int32
Hputelement(int32 file_id, uint16 tag, uint16 ref, const uint8 *data, int32 length)
{
    int32 access_id = FAIL; /* access record id */
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* get access record, write out data and dispose of access record */
    if ((access_id = Hstartwrite(file_id, (uint16)tag, (uint16)ref, length)) == FAIL)
        HGOTO_ERROR(DFE_NOMATCH, FAIL);

    if ((ret_value = Hwrite(access_id, length, data)) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    if (Hendaccess(access_id) == FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

done:
    if (ret_value == FAIL) { /* Error condition cleanup */
        if (access_id != FAIL)
            Hendaccess(access_id);
    }

    return ret_value;
} /* end Hputelement() */

/*--------------------------------------------------------------------------
NAME
   Hlength -- returns length of a data element
USAGE
   int32 Hlength(fileid, tag, ref)
   int fileid;             IN: id of file
   int tag;                IN: tag of data element
   int ref;                IN: ref of data element
RETURNS
   return the length of a data element or FAIL if there is a problem.
DESCRIPTION
   returns length of data element if it is present in the file.
   Return FAIL (-1) if it is not in the file or an error occurs.

   The current implementation is probably less efficient than it
   could be.  However, because of special elements the code is much
   cleaner this way.

--------------------------------------------------------------------------*/
int32
Hlength(int32 file_id, uint16 tag, uint16 ref)
{
    int32 access_id;        /* access record id */
    int32 length    = FAIL; /* length of elt inquired */
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* get access record, inquire about lebngth and then dispose of
         access record */
    access_id = Hstartread(file_id, tag, ref);
    if (access_id == FAIL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if ((ret_value = HQuerylength(access_id, &length)) == FAIL)
        HERROR(DFE_INTERNAL);

    if (Hendaccess(access_id) == FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    ret_value = length;

done:
    return ret_value;
} /* end Hlength */

/*--------------------------------------------------------------------------
NAME
   Hoffset -- get offset of data element in the file
USAGE
   int32 Hoffset(fileid, tag, ref)
   int32 fileid;           IN: id of file
   uint16 tag;             IN: tag of data element
   uint16 ref;             IN: ref of data element
RETURNS
   returns offset of data element if it is present in the
   file or FAIL (-1) if it is not.

DESCRIPTION
   This should be used for debugging purposes only since
   the user should not have to know the actual offset of
   a data element in a file.

   Like Hlength().  This could be sped up by not going through
   Hstartread() but because of special elements it is easier
   this way

--------------------------------------------------------------------------*/
int32
Hoffset(int32 file_id, uint16 tag, uint16 ref)
{
    int32 access_id;        /* access record id */
    int32 offset    = FAIL; /* offset of elt inquired */
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* get access record, inquire offset, and dispose of access record */
    access_id = Hstartread(file_id, tag, ref);
    if (access_id == FAIL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if ((ret_value = HQueryoffset(access_id, &offset)) == FAIL)
        HERROR(DFE_INTERNAL);

    if (Hendaccess(access_id) == FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    ret_value = offset;

done:
    return ret_value;
} /* Hoffset */

/*--------------------------------------------------------------------------
NAME
   Hishdf -- tells if a file is an HDF file
USAGE
   intn Hishdf(path)
   const char *path;             IN: name of file
RETURNS
   returns TRUE (non-zero) if file is HDF, FALSE (0) otherwise
DESCRIPTION
   This user level routine can be used to determine if a file
   with a given name is an HDF file.  Note, just because a file
   is not an HDF file does not imply that all HDF library
   functions can not work on it.

--------------------------------------------------------------------------*/
intn
Hishdf(const char *filename)
{
    intn       ret;
    hdf_file_t fp;
    intn       ret_value = TRUE;

    /* Search for a matching slot in the already open files. */
    if (HAsearch_atom(FIDGROUP, HPcompare_filerec_path, filename) != NULL)
        HGOTO_DONE(TRUE);

    fp = (hdf_file_t)HI_OPEN(filename, DFACC_READ);
    if (OPENERR(fp)) {
        ret_value = FALSE;
    }
    else {
        ret = HIvalid_magic(fp);
        HI_CLOSE(fp);
        ret_value = (int)ret;
    }

done:
    return ret_value;
} /* Hishdf */

/*--------------------------------------------------------------------------
NAME
   Htrunc -- truncate a data element to a length
USAGE
   int32 Htrunc(aid, len)
   int32 aid;             IN: id of file
   int32 len;             IN: length at which to truncate data element
RETURNS
   return the length of a data element
DESCRIPTION
   truncates a data element in the file.  Return
   FAIL (-1) if it is not in the file or an error occurs.

--------------------------------------------------------------------------*/
int32
Htrunc(int32 aid, int32 trunc_len)
{
    accrec_t *access_rec; /* access record */
    int32     data_len;   /* length of the data we are checking */
    int32     data_off;   /* offset of the data we are checking */
    int32     ret_value = SUCCEED;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(aid);
    if (access_rec == (accrec_t *)NULL || !(access_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_ARGS, FAIL);

        /* Dunno about truncating special elements... -QAK */
#ifdef DONT_KNOW
    /* if special elt, call special function */
    if (access_rec->special) {
        ret_value = (*access_rec->special_func->write)(access_rec, length, data);
        goto done;
    }
#endif

    /* get the offset and length of the dataset */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_off, &data_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* check for actually being able to truncate the data */
    if (data_len > trunc_len) {
        /* set the new length of the dataset.
           Note value of '-2' for the offset parameter means not to update
           the offset in the DD.*/
        if (HTPupdate(access_rec->ddid, -2, trunc_len) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
        if (access_rec->posn > trunc_len) /* move the seek position back */
            access_rec->posn = trunc_len;
        ret_value = trunc_len;
    } /* end if */
    else
        HGOTO_ERROR(DFE_BADLEN, FAIL);

done:
    return ret_value;
} /* end Htrunc() */

/*--------------------------------------------------------------------------
NAME
   HIsync -- sync file with memory
USAGE
   intn HIsync(file_rec)
   filerec_t *file_rec;            IN: file record of file
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
DESCRIPTION
    HIsync() performs the actual sync'ing of the file in memory & on disk.
NOTE

--------------------------------------------------------------------------*/
static intn
HIsync(filerec_t *file_rec)
{
    intn ret_value = SUCCEED;

    /* check whether to flush the file info */
    if (file_rec->cache && file_rec->dirty) {
        /* flush DD blocks if necessary */
        if (file_rec->dirty & DDLIST_DIRTY)
            if (HTPsync(file_rec) == FAIL)
                HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

        /* extend the end of the file if necessary */
        if (file_rec->dirty & FILE_END_DIRTY)
            if (HIextend_file(file_rec) == FAIL)
                HGOTO_ERROR(DFE_CANTFLUSH, FAIL);
        file_rec->dirty = 0; /* file doesn't need to be flushed now */
    }                        /* end if */

done:
    return ret_value;
} /* HIsync */

/*--------------------------------------------------------------------------
NAME
   Hsync -- sync file with memory
USAGE
   intn Hsync(file_id)
   int32 file_id;            IN: id of file
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
DESCRIPTION
   Currently, the on-disk and in-memory representations are always
   the same.  Thus there is no real use for Hsync().  In the future,
   things may be buffered before being written out at which time
   Hsync() will be useful to sync up the on-disk representation.
NOTE
   First tests of caching DD's until close.

--------------------------------------------------------------------------*/
intn
Hsync(int32 file_id)
{
    filerec_t *file_rec; /* file record */
    intn       ret_value = SUCCEED;

    /* check validity of file record and get dd ptr */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* check whether to flush the file info */
    if (HIsync(file_rec) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
    return ret_value;
} /* Hsync */

/*--------------------------------------------------------------------------
NAME
   Hcache -- set low-level caching for a file
USAGE
   intn Hcache(file_id,cache_on)
           int32 file_id;            IN: id of file
           intn cache_on;            IN: whether to cache or not
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
DESCRIPTION
   Set/reset the caching in an HDF file.
   If file_id is set to CACHE_ALL_FILES, then the value of cache_on is
   used to modify the default caching state.
--------------------------------------------------------------------------*/
intn
Hcache(int32 file_id, intn cache_on)
{
    filerec_t *file_rec; /* file record */
    intn       ret_value = SUCCEED;

    if (file_id == CACHE_ALL_FILES) /* check whether to modify the default cache */
    {                               /* set the default caching for all further files Hopen'ed */
        default_cache = (cache_on != 0 ? TRUE : FALSE);
    } /* end if */
    else {
        /* check validity of file record and get dd ptr */
        file_rec = HAatom_object(file_id);
        if (BADFREC(file_rec))
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* check whether to flush the file info */
        if (cache_on == FALSE && file_rec->cache) {
            if (HIsync(file_rec) == FAIL)
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
        } /* end if */
        file_rec->cache = (cache_on != 0 ? TRUE : FALSE);
    } /* end else */

done:
    return ret_value;
} /* Hcache */

/*--------------------------------------------------------------------------
NAME
   HDvalidfid -- check if a file ID is valid
USAGE
   int HDvalidfid(file_id)
   int32 file_id;            IN: id of file
RETURNS
   returns TRUE if valid ID else FALSE
DESCRIPTION
   Determine whether a given int32 is a valid HDF file ID or not

--------------------------------------------------------------------------*/
intn
HDvalidfid(int32 file_id)
{
    filerec_t *file_rec;
    intn       ret_value = TRUE;

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        ret_value = FALSE;

    return ret_value;
} /* HDvalidfid */

/*--------------------------------------------------------------------------
HDerr --  Closes a file and return FAIL.
           Replacement for DFIerr in HDF3.1 and before
--------------------------------------------------------------------------*/
int
HDerr(int32 file_id)
{
    Hclose(file_id);
    return FAIL;
}

/*--------------------------------------------------------------------------
NAME
   Hsetacceesstype -- set the I/O access type (serial, parallel, ...)
                                          of a data element
USAGE
   intn Hsetacceesstype(access_id, accesstype)
   int32 access_id;        IN: id of access element
   uintn accesstype;       IN: I/O access type
RETURNS
   returns FAIL (-1) if fail, SUCCEED (0) otherwise.
DESCRIPTION
   Set the type of I/O for accessing the data element to
   accesstype.

--------------------------------------------------------------------------*/
intn
Hsetaccesstype(int32 access_id, uintn accesstype)
{
    accrec_t *access_rec; /* access record */
    intn      ret_value = SUCCEED;

    /* clear error stack and check validity of this access id */
    HEclear();

    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);
    if (accesstype != DFACC_DEFAULT && accesstype != DFACC_SERIAL && accesstype != DFACC_PARALLEL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (accesstype == access_rec->access_type)
        goto done;

    /* kludge mode on */
    if (accesstype != DFACC_PARALLEL) /* go to PARALLEL only */
    {
        ret_value = FAIL;
        goto done;
    }
    /* if special elt, call special function */
    if (access_rec->special)
        ret_value = HXPsetaccesstype(access_rec);

done:
    return ret_value;
} /* Hsetacceesstype() */

/*--------------------------------------------------------------------------
 NAME
    HDdont_atexit
 PURPOSE
    Indicates to the library that an 'atexit()' routine is _not_ to be installed
 USAGE
    intn HDdont_atexit(void)
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
        This routine indicates to the library that an 'atexit()' cleanip routine
    should not be installed.  The major (only?) purpose for this is in
    situations where the library is dynamically linked into an application and
    is un-linked from the application before 'exit()' gets called.  In those
    situations, a routine installed with 'atexit()' would jump to a routine
    which was no longer in memory, causing errors.
        In order to be effective, this routine _must_ be called before any other
    HDF function calls, and must be called each time the library is loaded/
    linked into the application. (the first time and after it's been un-loaded)
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    If this routine is used, certain memory buffers will not be de-allocated,
    although in theory a user could call HPend on their own...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HDdont_atexit(void)
{
    intn ret_value = SUCCEED;

    if (install_atexit == TRUE)
        install_atexit = FALSE;

    return ret_value;
} /* end HDdont_atexit() */

/*==========================================================================

Internal Routines

==========================================================================*/

/*--------------------------------------------------------------------------
 NAME
    HIstart
 PURPOSE
    Global and H-level initialization routine
 USAGE
    intn HIstart()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Register the global shut-down routine (HPend) for call with atexit
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static intn
HIstart(void)
{
    intn ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine
     *
     * In the past, this had problems with Solaris + gcc
     *
     * XXX: Check to see if this is true with modern Solaris
     */
#if !(defined(__sun) && defined(__GNUC__))
    if (install_atexit == TRUE)
        if (atexit(&HPend) != 0)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);
#endif

    /* Create the file ID and access ID groups */
    if (HAinit_group(FIDGROUP, 64) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if (HAinit_group(AIDGROUP, 256) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    if (cleanup_list == NULL) {
        /* allocate list to hold terminateion fcns */
        if ((cleanup_list = malloc(sizeof(Generic_list))) == NULL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* initialize list */
        if (HDGLinitialize_list(cleanup_list) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    }

done:
    return ret_value;
} /* end HIstart() */

/*--------------------------------------------------------------------------
 NAME
    HPregister_term_func
 PURPOSE
    Registers a termination function in the list of routines to call during
    atexit() termination.
 USAGE
    intn HPregister_term_func(term_func)
        intn (*term_func)();           IN: function to call during axexit()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Adds routines to the linked-list of routines to call when terminating the
    library.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function, or real power-users.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HPregister_term_func(hdf_termfunc_t term_func)
{
    intn ret_value = SUCCEED;
    if (library_terminate == FALSE)
        if (HIstart() == FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    if (HDGLadd_to_list(*cleanup_list, (void *)term_func) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
    return ret_value;
} /* end HPregister_term_func() */

/*--------------------------------------------------------------------------
 NAME
    HPend
 PURPOSE
    Terminate various static buffers and shutdown the library.
 USAGE
    intn HPend()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Walk through the shutdown routines for the various interfaces and
    terminate them all.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function, or real power-users.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void
HPend(void)
{
    hdf_termfunc_t term_func; /* pointer to a termination routine for an interface */

    /* Shutdown the file ID atom group */
    HAdestroy_group(FIDGROUP);

    /* Shutdown the access ID atom group */
    HAdestroy_group(AIDGROUP);

    if ((term_func = (hdf_termfunc_t)HDGLfirst_in_list(*cleanup_list)) != NULL) {
        do {
            (*term_func)();
        } while ((term_func = (hdf_termfunc_t)HDGLnext_in_list(*cleanup_list)) != NULL);
    } /* end if */

    /* can't issue errors if you're free'ing the error stack. */
    HDGLdestroy_list(cleanup_list); /* clear the list of interface cleanup routines */
    /* free allocated list struct */
    free(cleanup_list);
    /* re-initialize */
    cleanup_list = NULL;

    HPbitshutdown();
    HXPshutdown();
    Hshutdown();
    HEshutdown();
    HAshutdown();
    tbbt_shutdown();
} /* end HPend() */

/*--------------------------------------------------------------------------
NAME
   HIextend_file -- extend file to current length
USAGE
   int HIextend_file(file_rec)
           filerec_t  * file_rec        IN: pointer to file structure to extend
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   The routine extends an HDF file to be the length on the f_end_off
   member of the file_rec.  This is mainly written as a function so that
   the functionality is localized.
--------------------------------------------------------------------------*/
static intn
HIextend_file(filerec_t *file_rec)
{
    uint8 temp      = 0;
    intn  ret_value = SUCCEED;

    if (HPseek(file_rec, file_rec->f_end_off) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);
    if (HP_write(file_rec, &temp, 1) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

done:
    return ret_value;
} /* HIextend_file */

/*--------------------------------------------------------------------------
NAME
   HIget_function_table -- create special function table
USAGE
   int HIget_func_table(access_rec)
   accrec_t * access_rec;     IN: access record we are working on
RETURNS
   NULL no matter what (seems odd....)
DESCRIPTION
   Set up the table of special functions for a given special element

--------------------------------------------------------------------------*/
static funclist_t *
HIget_function_table(accrec_t *access_rec)
{
    filerec_t  *file_rec; /* file record */
    int16       spec_code;
    uint8       lbuf[4];          /* temporary buffer */
    uint8      *p;                /* tmp buf ptr */
    int32       data_off;         /* offset of the data we are checking */
    int         i;                /* loop index */
    funclist_t *ret_value = NULL; /* FAIL */

    /* read in the special code in the special elt */
    file_rec = HAatom_object(access_rec->file_id);

    /* get the offset and length of the dataset */
    if (HTPinquire(access_rec->ddid, NULL, NULL, &data_off, NULL) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    if (HPseek(file_rec, data_off) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, NULL);
    if (HP_read(file_rec, lbuf, (int)2) == FAIL)
        HGOTO_ERROR(DFE_READERROR, NULL);

    /* using special code, look up function table in associative table */
    p = &lbuf[0];
    INT16DECODE(p, spec_code);
    access_rec->special = (intn)spec_code;
    for (i = 0; functab[i].key != 0; i++) {
        if (access_rec->special == functab[i].key) {
            ret_value = functab[i].tab;
            break; /* break out of loop */
        }
    }

done:
    return ret_value;
} /* HIget_function_table */

/*--------------------------------------------------------------------------
NAME
   HIgetspinfo -- return special info
USAGE
   int HIgetspinfo(access_rec, tag, ref)
   accrec_t * access_rec;     IN: access record we are working on
   int16      tag;            IN: tag to look for
   int16      ref;            IN: ref to look for
RETURNS
   special_info field or NULL if not found
DESCRIPTION
   given the tag and ref of a given element return the special
   info field of the access element.

   Basically, this function checks if any other AIDs in the file
   have read in the special information for this object.  If so,
   this special information will be reused.  Otherwise, the
   special element handling code needs to read in the information
   from disk
GLOBALS
   Reads from the global access_records

--------------------------------------------------------------------------*/
void *
HIgetspinfo(accrec_t *access_rec)
{
    void *ret_value = NULL; /* FAIL */

    if ((ret_value = HAsearch_atom(AIDGROUP, HPcompare_accrec_tagref, access_rec)) != NULL)
        HGOTO_DONE(((accrec_t *)ret_value)->special_info);

done:
    return ret_value;
} /* HIgetspinfo */

/*--------------------------------------------------------------------------
HIunlock -- unlock a previously locked file record
--------------------------------------------------------------------------*/
static int
HIunlock(filerec_t *file_rec)
{
    /* unlock the file record */
    file_rec->attach--;

    return SUCCEED;
}

/* ------------------------- SPECIAL TAG ROUTINES ------------------------- */
/*
   The HDF tag space is divided as follows based on the 2 highest bits:
   00: Library reserved ordinary tags
   01: Library reserved special tags
   10, 11: User tags.

   It is relatively cheap to operate with special tags within the library
   reserved tags range.  For users to specify special tags and their
   corresponding ordinary tag, the pair has to be added to the
   special_table.

   The special_table contains pairs of each tag and its corresponding
   special tag.  The same table is also used to determine if a tag is
   special.  Add to this table any additional tag/special_tag pairs
   that might be necessary.

 */

/*
   The functionality of these routines is covered by the SPECIALTAG,
   MKSPECIALTAG and BASETAG macros
 */

#ifdef SPECIAL_TABLE

typedef struct special_table_t {
    uint16 tag;
    uint16 special_tag;
} special_table_t;

static special_table_t special_table[] = {
    {0x8010, 0x4000 | 0x8010}, /* dummy */
};

#define SP_TAB_SZ (sizeof(special_table) / sizeof(special_table[0]))

/*--------------------------------------------------------------------------
--------------------------------------------------------------------------*/
uint16
HDmake_special_tag(uint16 tag)
{
    int    i;
    uint16 ret_value = DFTAG_NULL; /* FAIL */

    if (~tag & 0x8000) {
        ret_value = ((uint16)(tag | 0x4000));
        goto done;
    }

    for (i = 0; i < SP_TAB_SZ; i++)
        if (special_table[i].tag == tag) {
            ret_value = (uint16)special_table[i].special_tag;
            break;
        }

done:
    return ret_value;
}

/*--------------------------------------------------------------------------
--------------------------------------------------------------------------*/
intn
HDis_special_tag(uint16 tag)
{
    int  i;
    intn ret_value = FALSE; /* FAIL */

    if (~tag & 0x8000) {
        ret_value = (tag & 0x4000) ? TRUE : FALSE;
        goto done;
    }

    for (i = 0; i < SP_TAB_SZ; i++)
        if (special_table[i].special_tag == tag) {
            ret_value = TRUE;
            break;
        }

done:
    return ret_value;
}

/*--------------------------------------------------------------------------
--------------------------------------------------------------------------*/
uint16
HDbase_tag(uint16 tag)
{
    int    i;
    uint16 ret_value = tag;

    if (~tag & 0x8000) {
        ret_value = ((uint16)(tag & ~0x4000));
        goto done;
    }

    for (i = 0; i < SP_TAB_SZ; i++)
        if (special_table[i].special_tag == tag) {
            ret_value = special_table[i].special_tag;
            break;
        }
done:
    return ret_value;
}

#endif /* SPECIAL_TABLE */

/*--------------------------------------------------------------------------
 NAME
    Hgetlibversion -- return version info for current HDF library
 USAGE
    intn Hgetlibversion(majorv, minorv, release, string)
    uint32 *majorv;     OUT: majorv version number
    uint32 *minorv;     OUT: minorv version number
    uint32 *release;    OUT: release number
    char   string[];    OUT: informational text string (80 chars)
 RETURNS
    returns SUCCEED (0).
 DESCRIPTION
    Copies values from #defines in hfile.h to provided buffers. This
        information is statistically compiled into the HDF library, so
        it is not necessary to have any files open to get this information.

--------------------------------------------------------------------------*/
intn
Hgetlibversion(uint32 *majorv, uint32 *minorv, uint32 *releasev, char *string)
{
    HEclear();

    *majorv   = LIBVER_MAJOR;
    *minorv   = LIBVER_MINOR;
    *releasev = LIBVER_RELEASE;
    HIstrncpy(string, LIBVER_STRING, LIBVSTR_LEN + 1);

    return SUCCEED;
} /* HDgetlibversion */

/*--------------------------------------------------------------------------
 NAME
    Hgetfileversion -- return version info for HDF file
 USAGE
    intn Hgetfileversion(file_id, majorv, minorv, release, string)
    int32 file_id;      IN: handle of file
    uint32 *majorv;     OUT: majorv version number
    uint32 *minorv;     OUT: minorv version number
    uint32 *release;    OUT: release number
    char *string;       OUT: informational text string (80 chars)
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Copies values from file_records[] structure for a given file to
        provided buffers.
 GLOBAL VARIABLES
    Reads file_records[]

--------------------------------------------------------------------------*/
intn
Hgetfileversion(int32 file_id, uint32 *majorv, uint32 *minorv, uint32 *release, char *string)
{
    filerec_t *file_rec;
    intn       ret_value = SUCCEED;

    HEclear();

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (majorv != NULL)
        *majorv = file_rec->version.majorv;
    if (minorv != NULL)
        *minorv = file_rec->version.minorv;
    if (release != NULL)
        *release = file_rec->version.release;
    if (string != NULL)
        HIstrncpy(string, file_rec->version.string, LIBVSTR_LEN + 1);

done:
    return ret_value;
} /* Hgetfileversion */

/*--------------------------------------------------------------------------
 NAME
    HIcheckfileversion -- check version info for HDF file
 USAGE
    intn Hgetfileversion(file_id)
    int32 file_id;      IN: handle of file
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Checks that the file's version is current and update it if it isn't.

--------------------------------------------------------------------------*/
static intn
HIcheckfileversion(int32 file_id)
{
    filerec_t *file_rec;
    uint32     lmajorv = 0, lminorv = 0, lrelease = 0;
    uint32     fmajorv = 0, fminorv = 0, frelease = 0;
    char       string[LIBVSTR_LEN + 1]; /* len 80+1  */
    intn       newver    = 0;
    intn       ret_value = SUCCEED;

    HEclear();

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get file version and set newver condition */
    if (Hgetfileversion(file_id, &fmajorv, &fminorv, &frelease, string) != SUCCEED) {
        newver = 1;
        HEclear();
    } /* end if */

    /* get library version */
    Hgetlibversion(&lmajorv, &lminorv, &lrelease, string);

    /* check whether we need to update the file version tag */
    if (lmajorv > fmajorv || (lmajorv == fmajorv && lminorv > fminorv) ||
        (lmajorv == fmajorv && lminorv == fminorv && lrelease > frelease))
        newver = 1;
    if (newver == 1) {
        file_rec->version.majorv  = lmajorv;
        file_rec->version.minorv  = lminorv;
        file_rec->version.release = lrelease;
        HIstrncpy(file_rec->version.string, string, LIBVSTR_LEN + 1);
        file_rec->version.modified = 1;
    } /* end if */

    file_rec->version_set = TRUE;

done:
    return ret_value;
} /* HIcheckfileversion */

/*--------------------------------------------------------------------------
 NAME
       HIget_filerec_node -- find a filerec for a FILE
 USAGE
       filerec_t *HIget_filerec_node(path)
       char * path;             IN: name of file
 RETURNS
       a file record or else NULL
 DESCRIPTION
       Search the file record array for a matching record, or allocate an
       empty slot.
       The file is considered the same if the path matches exactly.  This
       routine is unable to detect aliases, or how to compare relative and
       absolute paths.

--------------------------------------------------------------------------*/
static filerec_t *
HIget_filerec_node(const char *path)
{
    filerec_t *ret_value = NULL;

    if ((ret_value = HAsearch_atom(FIDGROUP, HPcompare_filerec_path, path)) == NULL) {
        if ((ret_value = (filerec_t *)calloc(1, sizeof(filerec_t))) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);

        if ((ret_value->path = (char *)strdup(path)) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);

        /* Initialize annotation stuff */
        ret_value->an_tree[AN_DATA_LABEL] = NULL;
        ret_value->an_tree[AN_DATA_DESC]  = NULL;
        ret_value->an_tree[AN_FILE_LABEL] = NULL;
        ret_value->an_tree[AN_FILE_DESC]  = NULL;
        ret_value->an_num[AN_DATA_LABEL]  = -1;
        ret_value->an_num[AN_DATA_DESC]   = -1;
        ret_value->an_num[AN_FILE_LABEL]  = -1;
        ret_value->an_num[AN_FILE_DESC]   = -1;
    } /* end if */

done:
    return ret_value;
} /* HIget_filerec_node */

/*--------------------------------------------------------------------------
 NAME
       HIrelease_filerec_node -- release/recycle a filerec
 USAGE
       intn HIrelease_filerec_node(file_rec)
       filerec_t *file_rec;         IN: File record to release
 RETURNS
       SUCCEED/FAIL
 DESCRIPTION
        Release a file record back to the system

--------------------------------------------------------------------------*/
static intn
HIrelease_filerec_node(filerec_t *file_rec)
{
    /* Close file if it's opened */
    if (file_rec->file != NULL)
        HI_CLOSE(file_rec->file);

    /* Free all the components of the file record */
    free(file_rec->path);
    free(file_rec);

    return SUCCEED;
} /* HIrelease_filerec_node */

/*--------------------------------------------------------------------------
 NAME
       HPisfile_in_use -- check if a FILE is currently in use
 USAGE
       intn HPisfile_in_use(path)
       const char * path;             IN: name of file
 RETURNS
       TRUE if the file is in use or FALSE, otherwise.
 DESCRIPTION
        Get its record if the file is opened, then check its
        reference count to decide whether the file is currently in use.

--------------------------------------------------------------------------*/
intn
HPisfile_in_use(const char *path)
{
    filerec_t *file_rec  = NULL;
    intn       ret_value = FALSE;

    /* Search for the record of a file named "path". */
    file_rec = (filerec_t *)HAsearch_atom(FIDGROUP, HPcompare_filerec_path, path);

    /* If the file is not found, it can't be in use, return FALSE */
    if (file_rec == NULL)
        ret_value = FALSE;
    else if (file_rec->refcount) /* file is in use if ref count is not 0 */
        ret_value = TRUE;

    return ret_value;
} /* HPisfile_in_use */

/*--------------------------------------------------------------------------
 NAME
       HPcompare_filerec_path -- compare filerec objects for the atom API
 USAGE
       intn HPcompare_filerec_path(obj, key)
       const void * obj;             IN: pointer to the file record
       const void * key;             IN: pointer to the name of file
 RETURNS
       TRUE if the key matches the obj, FALSE otherwise
 DESCRIPTION
       Look inside the file record for the atom API and compare the the
       paths.
--------------------------------------------------------------------------*/
intn
HPcompare_filerec_path(const void *obj, const void *key)
{
    const filerec_t *frec      = obj;
    const char      *fname     = key;
    intn             ret_value = FALSE; /* set default as FALSE */

    /* check args */
    if (frec != NULL && fname != NULL) {
        /* check bad file record */
        if (BADFREC(frec))
            ret_value = FALSE;
        else {
            if (!strcmp(frec->path, fname))
                ret_value = TRUE;
            else
                ret_value = FALSE;
        }
    }

    return ret_value;
} /* HPcompare_filerec_path */

/*--------------------------------------------------------------------------
 NAME
       HPcompare_accrec_tagref -- compare accrec objects for the atom API
 USAGE
       intn HPcompare_accrec_tagref(obj, key)
       const void * rec1;            IN: pointer to the access record #1
       const void * rec2;            IN: pointer to the access record #2
 RETURNS
       TRUE if tag/ref of rec1 matches the tag/ref of rec2, FALSE otherwise
 DESCRIPTION
       Look inside the access record for the atom API and compare the the
       paths.
--------------------------------------------------------------------------*/
intn
HPcompare_accrec_tagref(const void *rec1, const void *rec2)
{
    uint16 tag1, ref1;        /* tag/ref of access record #1 */
    uint16 tag2, ref2;        /* tag/ref of access record #2 */
    intn   ret_value = FALSE; /* FAIL */

    if (rec1 != rec2) {
        if (HTPinquire(((const accrec_t *)rec1)->ddid, &tag1, &ref1, NULL, NULL) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FALSE);

        if (HTPinquire(((const accrec_t *)rec2)->ddid, &tag2, &ref2, NULL, NULL) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FALSE);

        if (((const accrec_t *)rec1)->file_id == ((const accrec_t *)rec2)->file_id && tag1 == tag2 &&
            ref1 == ref2)
            HGOTO_DONE(TRUE);
    } /* end if */

done:
    return ret_value;
} /* HPcompare_accrec_tagref */

/*--------------------------------------------------------------------------
 NAME
       HIvalid_magic -- verify the magic number in a file
 USAGE
       int32 HIvalid_magic(path)
       hdf_file_t file;             IN: FILE pointer
 RETURNS
       TRUE if valid magic number else FALSE
 DESCRIPTION
       Given an open file pointer, see if the first four bytes of the
       file are the HDF "magic number" HDFMAGIC

--------------------------------------------------------------------------*/
static intn
HIvalid_magic(hdf_file_t file)
{
    char b[MAGICLEN];       /* Temporary buffer */
    intn ret_value = FALSE; /* FAIL */

    /* Seek to beginning of the file. */
    if (HI_SEEK(file, 0) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FALSE);

    /* Read in magic cookie and compare. */
    if (HI_READ(file, b, MAGICLEN) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FALSE);

    if (NSTREQ(b, HDFMAGIC, MAGICLEN))
        ret_value = TRUE;

done:
    return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
    HIget_access_rec -- allocate a new access record
 USAGE
    int HIget_access_rec(void)
 RETURNS
    returns access_record pointer or NULL if failed.
 DESCRIPTION
        Return an pointer to a new access_rec to use for a new AID.

--------------------------------------------------------------------------*/
accrec_t *
HIget_access_rec(void)
{
    accrec_t *ret_value = NULL;

    HEclear();

    /* Grab from free list if possible */
    if (accrec_free_list != NULL) {
        ret_value        = accrec_free_list;
        accrec_free_list = accrec_free_list->next;
    } /* end if */
    else {
        if ((ret_value = (accrec_t *)malloc(sizeof(accrec_t))) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
    } /* end else */

    /* Initialize to zeros */
    memset(ret_value, 0, sizeof(accrec_t));

done:
    return ret_value;
} /* HIget_access_rec */

/******************************************************************************
 NAME
     HIrelease_accrec_node - Releases an atom node

 DESCRIPTION
    Puts an accrec node into the free list

 RETURNS
    No return value

*******************************************************************************/
void
HIrelease_accrec_node(accrec_t *acc)
{
    /* Insert the atom at the beginning of the free list */
    acc->next        = accrec_free_list;
    accrec_free_list = acc;
} /* end HIrelease_accrec_node() */

/*--------------------------------------------------------------------------
 PRIVATE    PRIVATE     PRIVATE     PRIVATE     PRIVATE
 NAME
    HIupdate_version -- determine whether new version tag should be written
 USAGE
    int HIupdate_version(file_id)
    int32 file_id;      IN: handle of file
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Writes out version numbers of current library as file version.
 GLOBAL VARIABLES
    Resets modified field of version field of appropriate file_records[]
    entry.

--------------------------------------------------------------------------*/
static int
HIupdate_version(int32 file_id)
{
    /* uint32 lmajorv, lminorv, lrelease; */
    uint8 /*lstring[81], */ lversion[LIBVER_LEN];
    filerec_t              *file_rec;
    int                     i;
    int                     ret_value = SUCCEED;

    HEclear();

    /* Check args */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* copy in-memory version to file */
    Hgetlibversion(&(file_rec->version.majorv), &(file_rec->version.minorv), &(file_rec->version.release),
                   file_rec->version.string);

    {
        uint8 *p;

        p = lversion;
        UINT32ENCODE(p, file_rec->version.majorv);
        UINT32ENCODE(p, file_rec->version.minorv);
        UINT32ENCODE(p, file_rec->version.release);
        HIstrncpy((char *)p, file_rec->version.string, LIBVSTR_LEN);
        i = (int)strlen((char *)p);
        memset(&p[i], 0, LIBVSTR_LEN - i);
    }

    if (Hputelement(file_id, (uint16)DFTAG_VERSION, (uint16)1, lversion, (int32)LIBVER_LEN) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    file_rec->version.modified = 0;

done:
    return ret_value;
} /* HIupdate_version */

/*--------------------------------------------------------------------------
 PRIVATE    PRIVATE     PRIVATE     PRIVATE     PRIVATE
 NAME
    HIread_version -- reads a version tag from a file
 USAGE
    int HIread_version(file_id)
    int32 file_id;      IN: handle of file
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Reads a version tag from the specified file into the version fields
    of the appropriate filerec_t.  On failure, zeros are put in the version
    number fields and NULLS in the string.
 GLOBAL VARIABLES
    Writes to version fields of appropriate file_records[] entry.

--------------------------------------------------------------------------*/
static int
HIread_version(int32 file_id)
{
    filerec_t *file_rec;
    int        ret_value = SUCCEED;
    uint8      fversion[LIBVER_LEN];
    memset(fversion, 0, sizeof(fversion));

    HEclear();

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (Hgetelement(file_id, (uint16)DFTAG_VERSION, (uint16)1, fversion) == FAIL) {
        file_rec->version.majorv  = 0;
        file_rec->version.minorv  = 0;
        file_rec->version.release = 0;
        strcpy(file_rec->version.string, "");
        file_rec->version.modified = 0;
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    }
    else {
        uint8 *p;

        p = fversion;
        UINT32DECODE(p, file_rec->version.majorv);
        UINT32DECODE(p, file_rec->version.minorv);
        UINT32DECODE(p, file_rec->version.release);
        HIstrncpy(file_rec->version.string, (char *)p, LIBVSTR_LEN);
    }
    file_rec->version.modified = 0;

done:
    return ret_value;
} /* HIread_version */

/*-----------------------------------------------------------------------
NAME
   HPgetdiskblock --- Get the offset of a free block in the file.
USAGE
   int32 HPgetdiskblock(file_rec, block_size)
   filerec_t *file_rec;     IN: ptr to the file record
   int32 block_size;        IN: size of the block needed
   intn moveto;             IN: whether to move the file position
                                to the allocated position or leave
                                it undefined.
RETURNS
   returns offset of block in the file if successful, FAIL (-1) if failed.
DESCRIPTION
   Used to "allocate" space in the file.  Currently, it just appends
   blocks to the end of the file willy-nilly.  At some point in the
   future, this could be changed to use a "real" free-list of empty
   blocks in the file and dole those out.

-------------------------------------------------------------------------*/
int32
HPgetdiskblock(filerec_t *file_rec, int32 block_size, intn moveto)
{
    uint8 temp;
    int32 ret_value = SUCCEED;

    /* check for valid arguments */
    if (file_rec == NULL || block_size < 0)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef DISKBLOCK_DEBUG
    block_size += (DISKBLOCK_HSIZE + DISKBLOCK_TSIZE);
    /* get the offset of the allocated block */
    ret_value = file_rec->f_end_off + DISKBLOCK_HSIZE;
#else  /* DISKBLOCK_DEBUG */
    /* get the offset of the allocated block */
    ret_value = file_rec->f_end_off;
#endif /* DISKBLOCK_DEBUG */

    /* reserve the space by marking the end of the element */
    if (block_size > 0) {
#ifdef DISKBLOCK_DEBUG
        if (file_rec->cache)
            file_rec->dirty |= FILE_END_DIRTY;
        else {
            /* Write the debugging head & tail to the file block allocated */
            if (HPseek(file_rec, file_rec->f_end_off) == FAIL)
                HGOTO_ERROR(DFE_SEEKERROR, FAIL);
            if (HP_write(file_rec, diskblock_header, DISKBLOCK_HSIZE) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
            if (HPseek(file_rec, file_rec->f_end_off + block_size - DISKBLOCK_TSIZE) == FAIL)
                HGOTO_ERROR(DFE_SEEKERROR, FAIL);
            if (HP_write(file_rec, diskblock_tail, DISKBLOCK_TSIZE) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
        }               /* end else */
#else                   /* DISKBLOCK_DEBUG */
        if (file_rec->cache)
            file_rec->dirty |= FILE_END_DIRTY;
        else {
            if (HPseek(file_rec, ret_value + block_size - 1) == FAIL)
                HGOTO_ERROR(DFE_SEEKERROR, FAIL);
            if (HP_write(file_rec, &temp, 1) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
        } /* end else */
#endif                  /* DISKBLOCK_DEBUG */
    }                   /* end if */
    if (moveto == TRUE) /* move back to the beginning of the element */
    {
        if (HPseek(file_rec, ret_value) == FAIL)
            HGOTO_ERROR(DFE_SEEKERROR, FAIL);
    } /* end if */

    /* incr. offset of end of file */
    file_rec->f_end_off += block_size;

done:
    return ret_value;
} /* HPgetdiskblock() */

/*-----------------------------------------------------------------------
NAME
   HPfreediskblock --- Release a block in a file to be reused.
USAGE
   intn HPfreediskblock(file_rec, block_off, block_size)
   filerec_t *file_rec;     IN: ptr to the file record
   int32 block_off;         IN: offset of the block to release
   int32 block_size;        IN: size of the block to release
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) if failed.
DESCRIPTION
   Used to "release" space in the file.  Currently, it does nothing.
   At some point in the future, this could be changed to add the block
   to a "real" free-list of empty blocks in the file and manage those.

-------------------------------------------------------------------------*/
intn
HPfreediskblock(filerec_t *file_rec, int32 block_off, int32 block_size)
{
    intn ret_value = SUCCEED;

    (void)file_rec;
    (void)block_off;
    (void)block_size;

    return ret_value;
} /* HPfreediskblock() */

/*--------------------------------------------------------------------------
 NAME
       HDget_special_info -- get information about a special element
 USAGE
       intn HDget_special_info(access_id, info_block)
       int32 access_id;        IN: id of READ access element
       sp_info_block_t * info_block;
                               OUT: information about the special element
 RETURNS
       SUCCEED / FAIL
 DESCRIPTION
       Fill in the given info_block with information about the special
       element.  Return FAIL if it is not a special element AND set
       the 'key' field to FAIL in info_block.

--------------------------------------------------------------------------*/
int32
HDget_special_info(int32 access_id, sp_info_block_t *info_block)
{
    accrec_t *access_rec; /* access record */
    int32     ret_value = FAIL;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || info_block == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* special elt, so call special function */
    if (access_rec->special)
        ret_value = (*access_rec->special_func->info)(access_rec, info_block);
    else /* else is not special so FAIL */
        info_block->key = FAIL;

done:
    return ret_value;
} /* HDget_special_info */

/*--------------------------------------------------------------------------
 NAME
       HDset_special_info -- reset information about a special element
 USAGE
       intn HDet_special_info(access_id, info_block)
       int32 access_id;        IN: id of READ access element
       sp_info_block_t * info_block;
                               IN: information about the special element
 RETURNS
       SUCCEED / FAIL
 DESCRIPTION
       Attempt to replace the special information for the given element
       with new information.  This routine should be used to rename
       external elements for example.  Doing things like changing the
       blocking of a linekd block element are beyond the scope of this
       routine.

--------------------------------------------------------------------------*/
int32
HDset_special_info(int32 access_id, sp_info_block_t *info_block)
{
    accrec_t *access_rec; /* access record */
    int32     ret_value = FAIL;

    /* clear error stack and check validity of access id */
    HEclear();
    access_rec = HAatom_object(access_id);
    if (access_rec == (accrec_t *)NULL || info_block == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* special elt, so call special function */
    if (access_rec->special)
        ret_value = (*access_rec->special_func->reset)(access_rec, info_block);

    /* else is not special so fail */
done:
    return ret_value;
} /* HDset_special_info */

/*--------------------------------------------------------------------------
 NAME
    Hshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn Hshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the H routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hshutdown(void)
{
    accrec_t *curr;

    /* Release the free-list if it exists */
    if (accrec_free_list != NULL) {
        while (accrec_free_list != NULL && accrec_free_list != accrec_free_list->next) {
            curr             = accrec_free_list;
            accrec_free_list = accrec_free_list->next;
            curr->next       = NULL;
            free(curr);
        }
    }

    return SUCCEED;
} /* end Hshutdown() */

/* #define HFILE_SEEKINFO */
#ifdef HFILE_SEEKINFO
static uint32 seek_taken       = 0;
static uint32 seek_avoided     = 0;
static uint32 write_force_seek = 0;
static uint32 read_force_seek  = 0;

void
Hdumpseek(void)
{
    printf("Seeks taken=%lu\n", (unsigned long)seek_taken);
    printf("Seeks avoided=%lu\n", (unsigned long)seek_avoided);
    printf("# of times write forced a seek=%lu\n", (unsigned long)write_force_seek);
    printf("# of times read forced a seek=%lu\n", (unsigned long)read_force_seek);
} /* Hdumpseek() */
#endif /* HFILE_SEEKINFO */

/*--------------------------------------------------------------------------
 NAME
    HP_read
 PURPOSE
    Alias for HI_READ on HDF files.
 USAGE
    intn HP_read(file_rec,buf,bytes)
        filerec_t * file_rec;   IN: Pointer to the HDF file record
        void * buf;              IN: Pointer to the buffer to read data into
        int32 bytes;            IN: # of bytes to read
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Function to wrap around HI_READ
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only be called by HDF low-level routines
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HP_read(filerec_t *file_rec, void *buf, int32 bytes)
{
    intn ret_value = SUCCEED;

    /* Check for switching file access operations */
    if (file_rec->last_op == H4_OP_WRITE || file_rec->last_op == H4_OP_UNKNOWN) {
#ifdef HFILE_SEEKINFO
        read_force_seek++;
#endif /* HFILE_SEEKINFO */
        file_rec->last_op = H4_OP_UNKNOWN;
        if (HPseek(file_rec, file_rec->f_cur_off) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */

    if (HI_READ(file_rec->file, buf, bytes) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);
    file_rec->f_cur_off += bytes;
    file_rec->last_op = H4_OP_READ;
done:
    return ret_value;
} /* end HP_read() */

/*--------------------------------------------------------------------------
 NAME
    HPseek
 PURPOSE
    Alias for HI_SEEK on HDF files.
 USAGE
    intn HPseek(file_rec,offset)
        filerec_t * file_rec;   IN: Pointer to the HDF file record
        int32 offset;           IN: offset in the file to go to
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Function to wrap around HI_SEEK
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only be called by HDF low-level routines
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HPseek(filerec_t *file_rec, int32 offset)
{
    intn ret_value = SUCCEED;

#ifdef HFILE_SEEKINFO
    printf("%s: file_rec=%p, last_offset=%ld, offset=%ld, last_op=%d", __func__, file_rec,
           (long)file_rec->f_cur_off, (long)offset, (int)file_rec->last_op);
#endif /* HFILE_SEEKINFO */
    if (file_rec->f_cur_off != offset || file_rec->last_op == H4_OP_UNKNOWN) {
#ifdef HFILE_SEEKINFO
        seek_taken++;
        printf(" taken: %d\n", (int)seek_taken);
#endif /* HFILE_SEEKINFO */
        if (HI_SEEK(file_rec->file, offset) == FAIL)
            HGOTO_ERROR(DFE_SEEKERROR, FAIL);
        file_rec->f_cur_off = offset;
        file_rec->last_op   = H4_OP_SEEK;
    } /* end if */
#ifdef HFILE_SEEKINFO
    else {
        seek_avoided++;
        printf(" avoided: %d\n", (int)seek_avoided);
    }
#endif /* HFILE_SEEKINFO */

done:
    return ret_value;
} /* end HPseek() */

/*--------------------------------------------------------------------------
 NAME
    HP_write
 PURPOSE
    Alias for HI_WRITE on HDF files.
 USAGE
    intn HP_write(file_rec,buf,bytes)
        filerec_t * file_rec;   IN: Pointer to the HDF file record
        void * buf;              IN: Pointer to the buffer to write
        int32 bytes;            IN: # of bytes to write
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Function to wrap around HI_WRITE
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only be called by HDF low-level routines
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HP_write(filerec_t *file_rec, const void *buf, int32 bytes)
{
    intn ret_value = SUCCEED;

    /* Check for switching file access operations */
    if (file_rec->last_op == H4_OP_READ || file_rec->last_op == H4_OP_UNKNOWN) {
#ifdef HFILE_SEEKINFO
        write_force_seek++;
#endif /* HFILE_SEEKINFO */
        file_rec->last_op = H4_OP_UNKNOWN;
        if (HPseek(file_rec, file_rec->f_cur_off) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */

    if (HI_WRITE(file_rec->file, buf, bytes) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
    file_rec->f_cur_off += bytes;
    file_rec->last_op = H4_OP_WRITE;

done:
    return ret_value;
} /* end HP_write() */

/*--------------------------------------------------------------------------
 NAME
    HDread_drec -- reads a description record
 USAGE
    int32 HDread_drec(file_id, data_id, drec_buf)
    int32 file_id;		IN: id of file
    atom_t data_id;		IN: id of an element
    uint8** drec_buf		OUT: buffer containing special info header
 RETURNS
    Returns the length of the info read
 DESCRIPTION
    This private function contains code that was repeated in several places
    throughout the library.  It gets access to the element's description
    record and read the special info header.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
    03-20-2010 BMR: Factored out repeated code
--------------------------------------------------------------------------*/
int32
HPread_drec(int32 file_id, atom_t data_id, uint8 **drec_buf)
{
    int32  drec_len = 0;       /* length of the description record */
    int32  drec_aid = -1;      /* description record access id */
    uint16 drec_tag, drec_ref; /* description record tag/ref */
    int32  ret_value = 0;

    /* get the info for the dataset (description record) */
    if (HTPinquire(data_id, &drec_tag, &drec_ref, NULL, &drec_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    if ((*drec_buf = (uint8 *)malloc(drec_len)) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* get the special info header */
    drec_aid = Hstartaccess(file_id, MKSPECIALTAG(drec_tag), drec_ref, DFACC_READ);
    if (drec_aid == FAIL)
        HGOTO_ERROR(DFE_BADAID, FAIL);
    if (Hread(drec_aid, 0, *drec_buf) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);
    if (Hendaccess(drec_aid) == FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    ret_value = drec_len;

done:
    return ret_value;
} /* HPread_drec */

/*--------------------------------------------------------------------------
 NAME
    HDcheck_empty -- determines if an element has been written with data
 USAGE
    int32 HDcheck_empty(file_id, tag, ref, *emptySDS)
    int32 file_id;             IN: id of file
    uint16 tag;                IN: tag of data element
    uint16 ref;                IN: ref of data element
    intn *emptySDS;	      OUT: TRUE if data element is empty
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    If the data element is special, gets the compressed or chunked description
    record and retrieves the special tag.  If the special tag indicates that
    the data element is compressed, then this function will retrieve the data
    length.  If the special tag indicates the data element is chunked, then
    retrieve the vdata chunk table to get its number of records.

    Uses the data length or number of records to determine the value for
    'emptySDS'.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
    10-30-2004 BMR: This function was added for SDcheckempty
    08-28-2007 BMR: The old code of this function failed when szip library
                didn't present (bugzilla 842.)  Modified to read info directly
                from file.
--------------------------------------------------------------------------*/
int32
HDcheck_empty(int32 file_id, uint16 tag, uint16 ref, intn *emptySDS /* TRUE if data element is empty */)
{
    int32      length;         /* length of the element's data */
    atom_t     data_id = FAIL; /* dd ID of existing regular element */
    filerec_t *file_rec;       /* file record pointer */
    uint8     *local_ptbuf = NULL, *p;
    int16      sptag       = -1; /* special tag read from desc record */
    int32      ret_value   = SUCCEED;

    /* clear error stack */
    HEclear();

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get access element from dataset's tag/ref */
    if ((data_id = HTPselect(file_rec, tag, ref)) != FAIL) {
        int32 dlen = 0, doff = 0; /* offset/length of the description record */

        /* Get the info pointed to by this dd, which could point to data or
           description record, or neither */
        if (HTPinquire(data_id, NULL, NULL, &doff, &dlen) == FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* doff/dlen = -1 means no data had been written */
        if (doff == INVALID_OFFSET && dlen == INVALID_LENGTH) {
            *emptySDS = TRUE;
        }

        /* if the element is not special, that means dataset's tag/ref
           specifies the actual data that was written to the dataset, so
           we don't need to check further */
        else if (HTPis_special(data_id) == FALSE) {
            *emptySDS = FALSE;
        }
        else {
            int32 rec_len = 0;

            /* Get the compression header (description record) */
            rec_len = HPread_drec(file_id, data_id, &local_ptbuf);
            if (rec_len <= 0)
                HGOTO_ERROR(DFE_INTERNAL, FAIL);

            /* get special tag */
            p = local_ptbuf;
            INT16DECODE(p, sptag);

            /* if it is a compressed element, get the data length and set
                flag emptySDS appropriately */
            if (sptag == SPECIAL_COMP) {
                /* skip 2byte header_version */
                p = p + 2;
                INT32DECODE(p, length); /* get _uncompressed_ data length */

                /* set flag specifying whether the dataset is empty */
                *emptySDS = length == 0 ? TRUE : FALSE;
            }

            /* if it is a chunked element, get the number of records in
               the chunk table (vdata) to determine emptySDS value */
            else if (sptag == SPECIAL_CHUNKED) {
                uint16 chk_tbl_tag, chk_tbl_ref; /* chunk table tag/ref */
                int32  vdata_id  = -1;           /* chunk table id */
                int32  n_records = 0;            /* number of records in chunk table */

                /* skip 4byte header len, 1byte chunking version, 4byte flag, */
                /* 4byte elm_tot_length, 4byte chunk_size and 4byte nt_size */
                p = p + 4 + 1 + 4 + 4 + 4 + 4;
                UINT16DECODE(p, chk_tbl_tag);
                UINT16DECODE(p, chk_tbl_ref);

                /* make sure it is really the vdata */
                if (chk_tbl_tag == DFTAG_VH) {
                    /* attach to the chunk table vdata and get its num of records */
                    if ((vdata_id = VSattach(file_id, chk_tbl_ref, "r")) == FAIL)
                        HGOTO_ERROR(DFE_CANTATTACH, FAIL);

                    if (VSinquire(vdata_id, &n_records, NULL, NULL, NULL, NULL) == FAIL)
                        HGOTO_ERROR(DFE_INTERNAL, FAIL);
                    if (VSdetach(vdata_id) == FAIL)
                        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

                    /* set flag specifying whether the dataset is empty */
                    *emptySDS = n_records == 0 ? TRUE : FALSE;
                } /* it is a vdata */
                else
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }
            /* need to check about other special cases - BMR 08/28/2007 */
        } /* else, data_id is special */

        /* end access to the aid */
        if (HTPendaccess(data_id) == FAIL)
            HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
    } /* end if data_id != FAIL */
    else {
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    }

done:
    free(local_ptbuf);

    return ret_value;
} /* end HDcheck_empty() */

/* ------------------------------- Hgetntinfo ------------------------------ */
/*
NAME
   Hgetntinfo -- retrieves some information of a number type in text format
USAGE
   intn Hgetntinfo(numbertype, nt_info)
   int32 numbertype;      IN: HDF-supported number type
   hdf_ntinfo_t *nt_info; OUT: structure containing number type's info
RETURNS
   FAIL if there is no match for the number type, otherwise, SUCCEED.
DESCRIPTION
   Load the structure hdf_ntinfo_t with the number type's name and byte
   order in array of characters format.  When the "default:" is reached,
   it means that a supported number type is missing from the switch statement
   or an unrecognized value is encountered.  The type will be verified and
   added or appropriate error handling will be added.  The structure
   hdf_ntinfo_t is defined in hdf.h.

   Design note: Passing the struct hdf_ntinfo_t into this function instead
   of individual strings will allow expandability without changing the
   function's prototype in the event of more information is desired.
   -BMR (Sep 2010)

---------------------------------------------------------------------------*/
intn
Hgetntinfo(const int32 numbertype, hdf_ntinfo_t *nt_info)
{
    /* Clear error stack */
    HEclear();

    /* Get byte order string */
    if ((DFNT_LITEND & numbertype) > 0) {
        strcpy(nt_info->byte_order, "littleEndian");
    }
    else
        strcpy(nt_info->byte_order, "bigEndian");

    /* Get type name string; must mask native and little-endian to make
       sure we get standard type */
    switch ((numbertype & ~DFNT_NATIVE) & ~DFNT_LITEND) {
        case DFNT_UCHAR8:
            strcpy(nt_info->type_name, "uchar8");
            break;
        case DFNT_CHAR8:
            strcpy(nt_info->type_name, "char8");
            break;
        case DFNT_FLOAT32:
            strcpy(nt_info->type_name, "float32");
            break;
        case DFNT_FLOAT64:
            strcpy(nt_info->type_name, "float64");
            break;
        case DFNT_FLOAT128:
            strcpy(nt_info->type_name, "float128");
            break;
        case DFNT_INT8:
            strcpy(nt_info->type_name, "int8");
            break;
        case DFNT_UINT8:
            strcpy(nt_info->type_name, "uint8");
            break;
        case DFNT_INT16:
            strcpy(nt_info->type_name, "int16");
            break;
        case DFNT_UINT16:
            strcpy(nt_info->type_name, "uint16");
            break;
        case DFNT_INT32:
            strcpy(nt_info->type_name, "int32");
            break;
        case DFNT_UINT32:
            strcpy(nt_info->type_name, "uint32");
            break;
        case DFNT_INT64:
            strcpy(nt_info->type_name, "int64");
            break;
        case DFNT_UINT64:
            strcpy(nt_info->type_name, "uint64");
            break;
        case DFNT_INT128:
            strcpy(nt_info->type_name, "int128");
            break;
        case DFNT_UINT128:
            strcpy(nt_info->type_name, "uint128");
            break;
        case DFNT_CHAR16:
            strcpy(nt_info->type_name, "char16");
            break;
        case DFNT_UCHAR16:
            strcpy(nt_info->type_name, "uchar16");
            break;
        default:
            return FAIL;
    } /* end switch */
    return SUCCEED;
} /* Hgetntinfo */
