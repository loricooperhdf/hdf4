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


#ifndef HDF_ZIP_TABLE_H__
#define HDF_ZIP_TABLE_H__


#include "hdf.h"
#include "mfhdf.h"

#define PFORMAT  "  %-7s %-7s %-7s\n" /*chunk info, compression info, name*/
#define PFORMAT1 "  %-7s %-7s %-7s"   /*chunk info, compression info, name*/

#ifdef __cplusplus
extern "C" {
#endif


/*struct to store the tag/ref and path of an object 
 the pair tag/ref uniquely identifies an HDF object */
typedef struct obj_info_t {
 int   tag;
 int   ref;
 char  obj_name[MAX_NC_NAME];
 int   flags[2];     
 /*flags that store matching object information
   between the 2 files 
   object exists in file = 1
   does not exist        = 0
 */
} obj_info_t;

/*struct that stores all objects */
typedef struct table_t {
 int        size;
 int        nobjs;
 obj_info_t *objs;
} table_t;


/* table methods */
void  dtable_init(table_t **table);
void  dtable_free(table_t *table);
int   dtable_search(table_t *table, int tag, int ref );
void  dtable_add(table_t *table, int tag, int ref, char* obj_name);
char* dtable_check(table_t *table, char*obj_name);
void  dtable_print(table_t *table);



#ifdef __cplusplus
}
#endif


#endif  /* HDF_ZIP_TABLE_H__ */