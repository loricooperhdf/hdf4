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


#include <string.h>
#include <stdio.h>

#include "parse.h"


/*-------------------------------------------------------------------------
 * Function: parse_comp
 *
 * Purpose: read compression info
 *
 * Return: a list of names, the number of names and its compression type
 *
 * Examples: 
 * "AA,B,CDE:RLE" 
 * "*:GZIP 6"
 * "A,B:NONE"
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: July 15, 2003
 *
 *-------------------------------------------------------------------------
 */


obj_list_t* parse_comp(char *str, int *n_objs, comp_info_t *comp)
{
 unsigned    i;
 char        c;
 size_t      len=strlen(str);
 int         j, n, k, end_obj;
 char        obj[MAX_NC_NAME]; 
 char        scomp[10];
 char        stype[5];
 obj_list_t* obj_list=NULL;

 /* initialize compression  info */
 memset(comp,0,sizeof(comp_info_t));

 /* check for the end of object list and number of objects */
 for ( i=0, n=0; i<len; i++)
 {
  c = str[i];
  if ( c==':' )
  {
   end_obj=i;
  }
  if ( c==',' )
  {
   n++;
  }
 }
  
 n++;
 obj_list=HDmalloc(n*sizeof(obj_list_t));
 *n_objs=n;

 /* get object list */
 for ( j=0, k=0, n=0; j<end_obj; j++,k++)
 {
  c = str[j];
  obj[k]=c;
  if ( c==',' || j==end_obj-1) 
  {
   if ( c==',') obj[k]='\0'; else obj[k+1]='\0';
   strcpy(obj_list[n].obj,obj);
   memset(obj,0,sizeof(obj));
   n++;
   k=-1;
  }
 }
 /* nothing after : */
 if (end_obj+1==(int)len)
 {
  if (obj_list) free(obj_list);
  printf("%s\nError: Invalid compression type\n",str);
  exit(1);
 }

 /* get compression type */
 for ( i=end_obj+1, k=0; i<len; i++,k++)
 {
  c = str[i];
  scomp[k]=c;
  if ( c==' ' || i==len-1) 
  {
   if ( c==' ') {      /*one more parameter */
    scomp[k]='\0';     /*cut space */
    stype[0]=str[i+1];
    stype[1]='\0';
    comp->info=atoi(stype);
   }
   else if (i==len-1) { /*no more parameters */
    scomp[k+1]='\0';
   }
   if (HDstrcmp(scomp,"NONE")==0)
    comp->type=COMP_CODE_NONE;
   else if (HDstrcmp(scomp,"RLE")==0)
    comp->type=COMP_CODE_RLE;
   else if (HDstrcmp(scomp,"HUFF")==0)
    comp->type=COMP_CODE_SKPHUFF;
   else if (HDstrcmp(scomp,"GZIP")==0)
    comp->type=COMP_CODE_DEFLATE;
   else if (HDstrcmp(scomp,"JPEG")==0)
    comp->type=COMP_CODE_JPEG;
   else {
    if (obj_list) free(obj_list);
    printf("%s\nError: Invalid compression type\n",str);
    exit(1);
   }
  }
 } /*i*/

 return obj_list;
}


/*-------------------------------------------------------------------------
 * Function: get_scomp
 *
 * Purpose: return the compression type as a string
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

char* get_scomp(int code)
{
 if (code==COMP_CODE_RLE)
  return "RLE";
 else if (code==COMP_CODE_SKPHUFF)
  return "HUFF";
 else if (code==COMP_CODE_DEFLATE)
  return "GZIP";
 else if (code==COMP_CODE_JPEG)
  return "JPEG";
 else if (code==COMP_CODE_NONE)
  return "NONE";
 else
  return "Error in compression type";
} 




/*-------------------------------------------------------------------------
 * Function: parse_chunk
 *
 * Purpose: read chunkink info
 *
 * Return: a list of names, the number of names and its chunking info
 *
 * Examples: 
 * "AA,B,CDE:10X10 
 * "*:10X10"
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: July 17, 2003
 *
 *-------------------------------------------------------------------------
 */


obj_list_t* parse_chunk(char *str, int *n_objs, int32 *chunk_lengths, int *chunk_rank)
{
 obj_list_t* obj_list=NULL;
 unsigned    i;
 char        c;
 size_t      len=strlen(str);
 int         j, n, k, end_obj, c_index;
 char        obj[MAX_NC_NAME]; 
 char        sdim[10];
 
 /* check for the end of object list and number of objects */
 for ( i=0, n=0; i<len; i++)
 {
  c = str[i];
  if ( c==':' )
  {
   end_obj=i;
  }
  if ( c==',' )
  {
   n++;
  }
 }
  
 n++;
 obj_list=HDmalloc(n*sizeof(obj_list_t));
 *n_objs=n;

 /* get object list */
 for ( j=0, k=0, n=0; j<end_obj; j++,k++)
 {
  c = str[j];
  obj[k]=c;
  if ( c==',' || j==end_obj-1) 
  {
   if ( c==',') obj[k]='\0'; else obj[k+1]='\0';
   strcpy(obj_list[n].obj,obj);
   memset(obj,0,sizeof(obj));
   n++;
   k=-1;
  }
 }

 /* nothing after : */
 if (end_obj+1==(int)len)
 {
  if (obj_list) free(obj_list);
  printf("%s\nError: Invalid chunking\n",str);
  exit(1);
 }

 /* get chunk info */
 for ( i=end_obj+1, k=0, c_index=0; i<len; i++,k++)
 {
  c = str[i];
  sdim[k]=c;
  if ( c=='x' || i==len-1) 
  {
   if ( c=='x') {  
    sdim[k]='\0';     
    chunk_lengths[c_index]=atoi(sdim);
    if (chunk_lengths[c_index]==0)
     {
      if (obj_list) free(obj_list);
      printf("%s\nError: Invalid chunking definition\n",str);
      exit(1);
     }
    c_index++;
   }
   else if (i==len-1) { /*no more parameters */
    sdim[k+1]='\0';  
    if (strcmp(sdim,"NONE")==0)
    {
     *chunk_rank=-2;
    }
    else
    {
     chunk_lengths[c_index]=atoi(sdim);
     if (chunk_lengths[c_index]==0)
     {
      if (obj_list) free(obj_list);
      printf("%s\nError: Invalid chunking definition\n",str);
      exit(1);
     }
     *chunk_rank=c_index+1;
    }
   }
  } /*if*/
 } /*i*/

 return obj_list;
}




#if defined (ONE_TABLE)

/*-------------------------------------------------------------------------
 * Function: options_table_init
 *
 * Purpose: 
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void options_table_init( options_table_t **tbl )
{
 int i;
 options_table_t* table = (options_table_t*) malloc(sizeof(options_table_t));
 
 table->size   = 3;
 table->nelems = 0;
 table->objs   = (obj_info_t*) malloc(table->size * sizeof(obj_info_t));
 
 for (i = 0; i < table->size; i++) {
   strcpy(table->objs[i].path,"\0");
   table->objs[i].comp.info  = -1;
   table->objs[i].comp.type  = -1;
   table->objs[i].chunk.rank = -1;
  }
 
 *tbl = table;
}

/*-------------------------------------------------------------------------
 * Function: options_table_free
 *
 * Purpose: free table memory
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void options_table_free( options_table_t *table )
{
 free(table->objs);
 free(table);
}

/*-------------------------------------------------------------------------
 * Function: options_add_chunk
 *
 * Purpose: add a chunking -c option to the option list
 *
 * Return: 
 *
 *-------------------------------------------------------------------------
 */

int options_add_chunk(obj_list_t *obj_list,int n_objs,int32 *chunk_lengths,
                      int chunk_rank,options_table_t *table )
{
 
 int i, j, k, I, added=0, found=0;
 
 if (table->nelems+n_objs >= table->size) {
  table->size += n_objs;
  table->objs = (obj_info_t*)realloc(table->objs, table->size * sizeof(obj_info_t));
  for (i = table->nelems; i < table->size; i++) {
   strcpy(table->objs[i].path,"\0");
   table->objs[i].comp.info  = -1;
   table->objs[i].comp.type  = -1;
   table->objs[i].chunk.rank = -1;
  }
 }
 
 /* search if this object is already in the table; "path" is the key */
 if (table->nelems>0)
 {
  /* go tru the supplied list of names */
  for (j = 0; j < n_objs; j++) 
  {
   /* linear table search */
   for (i = 0; i < table->nelems; i++) 
   {
    /*already on the table */
    if (strcmp(obj_list[j].obj,table->objs[i].path)==0)
    {
     /* already chunk info inserted for this one; exit */
     if (table->objs[i].chunk.rank>0)
     {
      printf("Error: chunk information already inserted for <%s>\n",obj_list[j].obj);
      exit(1);
     }
     /* insert the chunk info */
     else
     {
      table->objs[i].chunk.rank = chunk_rank;
      for (k = 0; k < chunk_rank; k++) 
       table->objs[i].chunk.chunk_lengths[k] = chunk_lengths[k];
      found=1;
      break;
     }
    } /* if */
   } /* i */
   
   if (found==0)
   {
    /* keep the grow in a temp var */
    I = table->nelems + added;  
    added++;
    strcpy(table->objs[I].path,obj_list[j].obj);
    table->objs[I].chunk.rank = chunk_rank;
    for (k = 0; k < chunk_rank; k++) 
     table->objs[I].chunk.chunk_lengths[k] = chunk_lengths[k];
   }
  } /* j */ 
 }
 
 /* first time insertion */
 else
 {
  /* go tru the supplied list of names */
  for (j = 0; j < n_objs; j++) 
  {
   I = table->nelems + added;  
   added++;
   strcpy(table->objs[I].path,obj_list[j].obj);
   table->objs[I].chunk.rank = chunk_rank;
   for (k = 0; k < chunk_rank; k++) 
    table->objs[I].chunk.chunk_lengths[k] = chunk_lengths[k];
  }
 }
 
 table->nelems+= added;
 
 return 0;
}



/*-------------------------------------------------------------------------
 * Function: options_add_comp
 *
 * Purpose: add a compression -t option to the option list
 *
 * Return: 
 *
 *-------------------------------------------------------------------------
 */

int options_add_comp(obj_list_t *obj_list,int n_objs,comp_info_t comp,
                     options_table_t *table )
{
 
 int i, j, I, added=0, found=0;
 
 if (table->nelems+n_objs >= table->size) {
  table->size += n_objs;
  table->objs = (obj_info_t*)realloc(table->objs, table->size * sizeof(obj_info_t));
  for (i = table->nelems; i < table->size; i++) {
   strcpy(table->objs[i].path,"\0");
   table->objs[i].comp.info  = -1;
   table->objs[i].comp.type  = -1;
   table->objs[i].chunk.rank = -1;
  }
 }
 
 /* search if this object is already in the table; "path" is the key */
 if (table->nelems>0)
 {
  /* go tru the supplied list of names */
  for (j = 0; j < n_objs; j++) 
  {
   /* linear table search */
   for (i = 0; i < table->nelems; i++) 
   {
    /*already on the table */
    if (strcmp(obj_list[j].obj,table->objs[i].path)==0)
    {
     /* already COMP info inserted for this one; exit */
     if (table->objs[i].comp.type>0)
     {
      printf("Error: compression information already inserted for <%s>\n",obj_list[j].obj);
      exit(1);
     }
     /* insert the comp info */
     else
     {
      table->objs[i].comp = comp;
      found=1;
      break;
     }
    } /* if */
   } /* i */
   
   if (found==0)
   {
    /* keep the grow in a temp var */
    I = table->nelems + added;  
    added++;
    strcpy(table->objs[I].path,obj_list[j].obj);
    table->objs[I].comp = comp;
   }
  } /* j */ 
 }
 
 /* first time insertion */
 else
 {
  /* go tru the supplied list of names */
  for (j = 0; j < n_objs; j++) 
  {
   I = table->nelems + added;  
   added++;
   strcpy(table->objs[I].path,obj_list[j].obj);
   table->objs[I].comp = comp;
  }
 }
 
 table->nelems+= added;
 
 return 0;
}





/*-------------------------------------------------------------------------
 * Function: options_get_object
 *
 * Purpose: get object from table; "path" is the key
 *
 * Return: 
 *
 *-------------------------------------------------------------------------
 */

obj_info_t* options_get_object(char *path,options_table_t *table)
{
 int i;
 
 for ( i = 0; i < table->nelems; i++) 
 {
  /* found it */
  if (strcmp(table->objs[i].path,path)==0)
  {
   return (&table->objs[i]);
  }
 }
 
 return NULL;
}




#else



/*-------------------------------------------------------------------------
 * Function: comp_list_add
 *
 * Purpose: add a compression -t option to the option list
 *
 * Return: ALL, for compress all, SELECTED for list of names to compress
 *
 *-------------------------------------------------------------------------
 */

int comp_list_add(obj_list_t *obj_list, int n_objs, comp_info_t comp, comp_table_t *table )
{
 int i;

 if (table->nelems == table->size) {
  table->size *= 2;
  table->objs = (obj_comp_t*)realloc(table->objs, table->size * sizeof(obj_comp_t));
  for (i = table->nelems; i < table->size; i++) {
   table->objs[i].obj_list  = NULL;
   table->objs[i].n_objs    = 0;
   table->objs[i].comp.info = -1;
   table->objs[i].comp.type = -1;
  }
 }
 
 i = table->nelems++;
 table->objs[i].n_objs     = n_objs;
 table->objs[i].comp       = comp;
 table->objs[i].obj_list   = obj_list;

 /* searh for the "*" all files character */
 for (i = 0; i < n_objs; i++) 
 {
  if (strcmp("*",obj_list[i].obj)==0)
   return ALL;
 }
   
 return SELECTED;
}

/*-------------------------------------------------------------------------
 * Function: comp_list_init
 *
 * Purpose: initialize the compression -t option list
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void comp_list_init( comp_table_t **tbl )
{
 int i;
 comp_table_t* table = (comp_table_t*) malloc(sizeof(comp_table_t));
 
 table->size   = 3;
 table->nelems = 0;
 table->objs   = (obj_comp_t*) malloc(table->size * sizeof(obj_comp_t));
 
 for (i = 0; i < table->size; i++) {
   table->objs[i].obj_list  = NULL;
   table->objs[i].n_objs    = 0;
   table->objs[i].comp.info = -1;
   table->objs[i].comp.type = -1;
  }
 
 *tbl = table;
}



/*-------------------------------------------------------------------------
 * Function: comp_list_free
 *
 * Purpose: free table memory
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void comp_list_free( comp_table_t *table )
{
 free(table->objs);
 free(table);
}

/*-------------------------------------------------------------------------
 * Function: chunk_list_add
 *
 * Purpose: add a compression -t option to the option list
 *
 * Return: ALL, for compress all, SELECTED for list of names to compress
 *
 *-------------------------------------------------------------------------
 */

int chunk_list_add(obj_list_t *obj_list,int n_objs,int32 *chunk_lengths,
                   int chunk_rank,chunk_table_t *table )
{

 int i, j;

 if (table->nelems == table->size) {
  table->size *= 2;
  table->objs = (obj_chunk_t*)realloc(table->objs, table->size * sizeof(obj_chunk_t));
  for (i = table->nelems; i < table->size; i++) {
   table->objs[i].obj_list  = NULL;
   table->objs[i].n_objs    = 0;
   table->objs[i].rank      = -1;
  }
 }

 i = table->nelems++;
 table->objs[i].n_objs     = n_objs;
 table->objs[i].rank       = chunk_rank;
 for (j = 0; j < chunk_rank; j++) 
  table->objs[i].chunk_lengths[j] = chunk_lengths[j];
 table->objs[i].obj_list   = obj_list;

 /* searh for the "*" all files character */
 for (i = 0; i < n_objs; i++) 
 {
  if (strcmp("*",obj_list[i].obj)==0)
   return ALL;
 }

   
 return SELECTED;
}


/*-------------------------------------------------------------------------
 * Function: chunk_list_init
 *
 * Purpose: initialize the chunking -c option list
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void chunk_list_init( chunk_table_t **tbl )
{
 int i;
 chunk_table_t* table = (chunk_table_t*) malloc(sizeof(chunk_table_t));
 
 table->size   = 3;
 table->nelems = 0;
 table->objs   = (obj_chunk_t*) malloc(table->size * sizeof(obj_chunk_t));
 
 for (i = 0; i < table->size; i++) {
   table->objs[i].obj_list  = NULL;
   table->objs[i].n_objs    = 0;
   table->objs[i].rank      = -1;
  }
 
 *tbl = table;
}


/*-------------------------------------------------------------------------
 * Function: chunk_list_free
 *
 * Purpose: free table memory
 *
 * Return: void
 *
 *-------------------------------------------------------------------------
 */

void chunk_list_free( chunk_table_t *table )
{
 free(table->objs);
 free(table);
}



#endif






#if 0
int options_add_chunk(obj_list_t *obj_list,int n_objs,int32 *chunk_lengths,
                      int chunk_rank,options_table_t *table )
{
 
 int i, j, k, I, added=0;
 
 if (table->nelems == table->size) {
  table->size *= 2;
  table->objs = (obj_info_t*)realloc(table->objs, table->size * sizeof(obj_info_t));
  for (i = table->nelems; i < table->size; i++) {
   strcpy(table->objs[i].path,"\0");
   table->objs[i].comp.info = -1;
   table->objs[i].comp.type = -1;
   table->objs[i].rank      = -1;
  }
 }
 
 /* search if this object is already in the table; "path" is the key */
 if (table->nelems>0)
 {
  for (i = 0; i < table->nelems; i++) 
  {
   /* go tru the supplied list of names */
   for (j = 0; j < n_objs; j++) 
   {
    /* already on the table */
    if (on_table(obj_list[j].obj,table)==1)
    {
     /* already chunk info inserted for this one; exit */
     if (table->objs[i].rank>0)
     {
      printf("Error: chunk information already inserted for <%s>\n",obj_list[j].obj);
      exit(1);
     }
     /* insert the chunk info */
     else
     {
      table->objs[i].rank = chunk_rank;
      for (k = 0; k < chunk_rank; k++) 
       table->objs[i].chunk_lengths[k] = chunk_lengths[k];
     }
    }
    /* not on the table */
    else
    {
     
     /* keep the grow in a temp var */
     I = table->nelems + added;  
     added++;
     strcpy(table->objs[I].path,obj_list[j].obj);
     table->objs[I].rank = chunk_rank;
     for (k = 0; k < chunk_rank; k++) 
      table->objs[I].chunk_lengths[k] = chunk_lengths[k];

     
     
    } /* if */
   } /* j */
  } /* i */ 
 }
 
 /* first time insertion */
 else
  
 {

  /* go tru the supplied list of names */
  for (j = 0; j < n_objs; j++) 
  {
   I = table->nelems + added;  
   added++;
   strcpy(table->objs[I].path,obj_list[j].obj);
   table->objs[I].rank = chunk_rank;
   for (k = 0; k < chunk_rank; k++) 
    table->objs[I].chunk_lengths[k] = chunk_lengths[k];
  }
  
 }

 table->nelems+= added;

 return 0;
}
#endif


