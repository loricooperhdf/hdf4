/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

/* Macro to check status value and print error message */
#define CHECK(status, fail_value, name) {if(status == fail_value) { \
    printf("*** Routine %s FAILED at line %d ***\n", name, __LINE__); num_err++;}}
/* BMR - 2/21/99: added macro VERIFY to use in testing SDcheckempty 
   initially, but it should be used wherever appropriate */
#define VERIFY(item, value, test_name) {if(item != value) { \
    printf("*** UNEXPECTED VALUE from %s is %ld at line %4d in %s\n", test_name, (long)item,(int)__LINE__,__FILE__); num_err++;}}

