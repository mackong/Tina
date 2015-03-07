/*
 * =====================================================================================
 *
 *       Filename:  mime.c
 *
 *    Description:  get mime type.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 10:13:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdlib.h>     /* bsearch */
#include <strings.h>    /* strcacsecmp */
#include <string.h>     /* strrchr */

/** mime entry  */
struct mime_entry{
    char *ext; /*!< extension */
    char *type; /*!< mime type */
};

struct mime_entry entrys[] = {
#include "mime_types.h"
};

#define NR_OF_ENTRYS (sizeof(entrys)/sizeof(struct mime_entry)) 

/**
    bsearch compare function.
*/
static int comp_entry(const void *e1,const void *e2)
{
    struct mime_entry *me1 = (struct mime_entry*)e1;
    struct mime_entry *me2 = (struct mime_entry*)e2;
    return strcasecmp(me1->ext,me2->ext);
}

char *get_mime_type(const char *filename)
{
    struct mime_entry me_key,*me_res;
    char *dot = strrchr(filename,'.');
    if(dot == NULL) return NULL;
    me_key.ext = strrchr(filename,'.') + 1;
    me_res = bsearch(&me_key,entrys,NR_OF_ENTRYS,sizeof(struct mime_entry),comp_entry);
    if(me_res == NULL){
        return NULL;
    }

    return me_res->type;
}
