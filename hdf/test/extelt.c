#ifdef RCSID
static char RcsId[] = "@(#)$Revision$";
#endif
/*
$Header$

$Log$
Revision 1.1  1993/08/16 21:47:55  koziol
Wrapped in changes for final, working version on the PC.

 * Revision 1.2  1993/04/19  23:03:44  koziol
 * General Code Cleanup to reduce/remove compilation warnings on PC
 *
 * Revision 1.1  1993/04/15  20:00:13  koziol
 * Re-named the new tests for MS-DOS compatibility
 *
 * Revision 1.5  1993/03/24  21:40:01  chouck
 * Cleaned Makefile and fixed external element test
 *
 * Revision 1.4  1993/03/17  21:29:43  chouck
 * Updated external elements test and fixed syntax error in anfile test
 *
 * Revision 1.3  1993/02/16  20:51:14  chouck
 * Went back to using -ansi so needed to fix a few casting problems
 *
 * Revision 1.2  1993/01/27  22:41:25  briand
 * Fixed problem with compiling on RS6000.
 *
 * Revision 1.1  1993/01/27  22:04:29  briand
 * Converted test files to work with master test program: testhdf
 *
 * Revision 1.7  1992/07/27  18:40:12  dilg
 * Changed DFACC_ALL to DFACC_RDWR in appropriate places to conform to new
 * handling of access modes by Hopen().
 *
 * Revision 1.6  1992/06/25  18:27:39  chouck
 * Changed output file names
 *
 * Revision 1.5  1992/06/22  23:04:42  chouck
 * Removed calls to fork()
 *
 * Revision 1.4  1992/06/01  18:57:55  chouck
 * Cleaned up output so its clear what is going on
 *
 * Revision 1.3  1992/05/31  15:13:55  mfolk
 * Added several casts (uint8 * and uint16) to keep Convex from complaining.
 *
 * Revision 1.2  1992/05/28  14:24:01  chouck
 * Added casts for calls to Hinquire()
 *
 * Revision 1.1  1992/02/10  20:59:34  chouck
 * Initial revision
 *
*/

#include "tproto.h"
#define TESTFILE_NAME "t.hdf"

#define BUF_SIZE        4096

#ifdef TEST_PC
#define FAR far
#else
#define FAR /* */
#endif

static uint8 FAR outbuf[BUF_SIZE],
    FAR inbuf[BUF_SIZE];

extern int num_errs;
extern int Verbocity;

void test_hextelt()
{
    int32 fid, fid1;
    int32 aid1, aid2;
    int32 fileid, length, offset, posn;
    uint16 tag, ref;
    int16 access, special;
    int i;
    int32 ret;
    intn errors = 0;
#ifdef QAK
    uint8 *outbuf, *inbuf;

    outbuf=HDgetspace(BUF_SIZE);
    inbuf=HDgetspace(BUF_SIZE);
	if(!outbuf || !inbuf) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
#endif

    for (i=0; i<BUF_SIZE; i++)
        outbuf[i] = (char) (i % 256);

    MESSAGE(5,printf("Creating base file %s\n", TESTFILE_NAME););

    fid = Hopen(TESTFILE_NAME, DFACC_CREATE, 0);
    CHECK(fid, FAIL, "Hopen");

    ret = Hnewref(fid);
    CHECK(ret, FAIL, "Hnewref");

    MESSAGE(5,printf("Writing object into base file\n"););
    ret = Hputelement(fid, (uint16) 1000, (uint16) 1, 
                      (uint8 *) "element 1000 1 wrong   ",
                      HDstrlen("element 1000 1 wrong") + 1);
    CHECK(ret, FAIL, "Hputelement");

    MESSAGE(5,printf("Promoting above object to external element in file #1\n"););
    aid1 = HXcreate(fid, 1000, 1, "t1.hdf", (int32) 0, (int32) 0);
    CHECK(aid1, FAIL, "HXcreate");

    ret = Hseek(aid1, HDstrlen("element 1000 1") + 1, DF_START);
    CHECK(ret, FAIL, "Hseek");

    ret = Hwrite(aid1, HDstrlen("correct")+1, (uint8 *) "correct");
    if(ret != (int32)HDstrlen("correct") + 1) {
      fprintf(stderr, "Hwrite failed (code %d)\n", ret);
      HEprint(stderr, 0);
      exit(1);
    }

    ret = Hendaccess(aid1);
    CHECK(ret, FAIL, "Hendaccess");

    MESSAGE(5,printf("Creating an external element in file #2\n"););
    aid1 = HXcreate(fid, 1000, 4, "t2.hdf", (int32) 0, (int32) 0);
    CHECK(aid1, FAIL, "HXcreate");

    MESSAGE(5,printf("Writing 2000 bytes to file #2\n"););
    ret = Hwrite(aid1, 2000, outbuf);
    CHECK(ret, FAIL, "Hwrite");

    ret = Hendaccess(aid1);
    CHECK(ret, FAIL, "Hendaccess");

    ret = Hnewref(fid);
    CHECK(ret, FAIL, "Hnewref");

    MESSAGE(5,printf("Creating an external element in file #3\n"););
    aid1 = HXcreate(fid, 1000, 2, "t3.hdf", (int32) 0, (int32) 0);
    CHECK(aid1, FAIL, "HXcreate");

#define STRING "element 1000 2"

    MESSAGE(5,printf("Writing string '%s' to file #3\n", STRING););
    ret = Hwrite(aid1, HDstrlen(STRING)+1, (uint8 *) STRING);
    if(ret != (int32)HDstrlen(STRING)+1) {
      fprintf(stderr, "Hwrite failed (code %d)\n", ret);
      HEprint(stderr, 0);
      exit(1);
    }

    ret = Hendaccess(aid1);
    CHECK(ret, FAIL, "Hendaccess");

    MESSAGE(5,printf("Creating an overlapping element\n"););
    aid2 = HXcreate(fid, 1001, 2, "t3.hdf", (int32) 8, (int32) 4);
    CHECK(aid2, FAIL, "HXcreate");

    ret = Hgetelement(fid, (uint16) 1001, (uint16) 2, inbuf);
    if(ret != 4) {
      fprintf(stderr, "Incorrect element size returned from Hgetelement: %d\n",
              ret);
      HEprint(stderr, 0);
      exit(1);
    }

    if(inbuf[0] != '1' ||
       inbuf[1] != '0' ||
       inbuf[2] != '0' ||
       inbuf[3] != '0') {
        MESSAGE(3,printf("One or more errors in overlapping element\n"););
        errors++;
    }

    ret = Hendaccess(aid2);
    CHECK(ret, FAIL, "Hendaccess");

    MESSAGE(5,printf("Verifying data that was stored to file #2\n"););
    ret = Hgetelement(fid, (uint16) 1000, (uint16) 4, inbuf);
    if(ret != 2000) {
      fprintf(stderr, "Incorrect element size returned from Hgetelement: %d\n",
              ret);
      HEprint(stderr, 0);
      exit(1);
    }

    for (i=0; i<ret; i++) {
       if (inbuf[i] != outbuf[i]) {
           printf("Wrong data at %d, out %d in %d\n", i, outbuf[i], inbuf[i]);
           errors ++;
         }
       inbuf[i] = '\0';
    }

    MESSAGE(5,printf("Creating an external element in file #4\n"););
    aid1 = HXcreate(fid, 1020, 2, "t4.hdf", (int32) 0, (int32) 0);
    CHECK(aid1, FAIL, "HXcreate");

    MESSAGE(5,printf("Writing %d bytes to file #4\n",BUF_SIZE););
    ret = Hwrite(aid1, BUF_SIZE, outbuf);
    if(ret != BUF_SIZE) {
      fprintf(stderr, "Hwrite failed (code %d)\n", ret);
      HEprint(stderr, 0);
      exit(1);
    }

    ret = Hendaccess(aid1);
    CHECK(ret, FAIL, "Hendaccess");

    ret = Hclose(fid);
    CHECK(ret, FAIL, "Hclose");

    MESSAGE(5,printf("Closing and re-opening base file %s\n", TESTFILE_NAME););
    fid = Hopen(TESTFILE_NAME, DFACC_RDWR, 0);
    CHECK(ret, FAIL, "Hopen");

    ret = Hnewref(fid);
    CHECK(ret, FAIL, "Hnewref");

    aid1 = Hstartread(fid, 1000, 1);
    CHECK(aid1, FAIL, "Hstartread");

    MESSAGE(5,printf("Inquiring about access element in file #1\n"););
    ret = Hinquire(aid1, &fileid, &tag, &ref, &length, &offset, &posn,
                  &access, &special);
    CHECK(ret, FAIL, "Hinquire");

    ret = Hread(aid1, length, inbuf);
    if(ret != length) {
      fprintf(stderr, "Hread failed (code %d)\n", ret);
      HEprint(stderr, 0);
      exit(1);
    }

    if(HDstrcmp((const char *) inbuf, (const char *) "element 1000 1 correct")) {
      fprintf(stderr, "Object stored in file #1 is wrong\n");
      fprintf(stderr, "\t       Is: %s\n", inbuf);
      fprintf(stderr, "\tShould be: element 1000 1 correct\n");
      errors++;
    }

    ret = Hnewref(fid);
    CHECK(ret, FAIL, "Hnewref");

    ret = Hnextread(aid1, 1000, DFREF_WILDCARD, DF_CURRENT);
    CHECK(ret, FAIL, "Hnextread");

    MESSAGE(5,printf("Inquiring about access element in file #2\n"););
    ret = Hinquire(aid1, &fileid, &tag, &ref, &length, &offset, &posn,
                  &access, &special);
    CHECK(ret, FAIL, "Hinquire");

    ret = Hnextread(aid1, DFTAG_WILDCARD, DFREF_WILDCARD, DF_START);
    CHECK(ret, FAIL, "Hnextread");

    MESSAGE(5,printf("Inquiring about access element in file #3\n"););
    ret = Hinquire(aid1, &fileid, &tag, &ref, &length, &offset, &posn,
                  &access, &special);
    CHECK(ret, FAIL, "Hinquire");

    ret = Hnextread(aid1, DFTAG_WILDCARD, 3, DF_CURRENT);
    if(ret != FAIL) {
      fprintf(stderr, "Hnextread call should have failed !!!\n");
      errors++;
    }

    ret = Hnextread(aid1, DFTAG_WILDCARD, 2, DF_CURRENT);
    CHECK(ret, FAIL, "Hnextread");

    MESSAGE(5,printf("Inquiring about access element in file #4\n"););
    ret = Hinquire(aid1, &fileid, &tag, &ref, &length, &offset, &posn,
                  &access, &special);
    CHECK(ret, FAIL, "Hinquire");

    aid2 = Hstartwrite(fid, 1000, 1, 4);
    CHECK(aid2, FAIL, "Hstartwrite");

    ret = Hwrite(aid2, 4, (uint8 *) "ABCD");
    if(ret != 4) {
      fprintf(stderr, "Hwrite failed (code %d)\n", ret);
      HEprint(stderr, 0);
      exit(1);
    }

    ret = Hendaccess(aid1);
    CHECK(ret, FAIL, "Hendaccess");

    ret = Hendaccess(aid2);
    CHECK(ret, FAIL, "Hendaccess");

    fid1 = Hopen(TESTFILE_NAME, DFACC_READ, 0);
    CHECK(fid1, FAIL, "Hopen");

    ret = Hnewref(fid1);
    CHECK(ret, FAIL, "Hnewref");

    ret = Hclose(fid);
    CHECK(ret, FAIL, "Hclose");

    ret = Hclose(fid1);
    CHECK(ret, FAIL, "Hclose");

#ifdef QAK
    HDfreespace(outbuf);
    HDfreespace(inbuf);
#endif
}