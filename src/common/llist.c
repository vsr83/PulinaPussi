/*
 * llist.c / PulinaPussi 0.12
 * written by Ville Räisänen <raivil@geek.com> 2001-2003
 * tällätteet kyl kannattais varmaan tehä pärlil :)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include "../pp.h"


/**********************************************
 * LList_new()
 *
 * Create a new list. 
 * Returns a pointer to the first item.
 **********************************************/

LLItem *
LList_new() {
  LLItem *ci;

  ci= (LLItem*)Calloc(1, sizeof(LLItem));
  ci->name= NULL;
  ci->next= NULL;
  ci->data= NULL;

  return ci;
}

/***********************************************
 * LList_add(cfirst, name)
 * Add item to a list.
 *
 * LLItem *cfirst;  Pointer to the first item.
 * char *name;      Name of the new item.
 * void *data;      Pointer to the data of the new item.
 ***********************************************/

int
LList_add(LLItem *cfirst, char *name, void *data) {
  LLItem *cn, *ci= cfirst;
  
  if (!cfirst || !name) return 0;

  while (ci->next != NULL){
    ci= (LLItem*)ci->next;
    if (ci->name && Strcasecmp(ci->name, name)==0) return 0;
  }

  cn= LList_new();
  cn->name= (char*)Strdup(name);
  cn->data= data;
  ci->next= cn;

  return 1;
}

/************************************************
 * LList_exists(cfirst, namel)
 * Checks whether item is found from the list.
 *
 * LLItem *cfirst; Pointer to the first item.
 * char *name;     Name of the item.
 ************************************************/

int 
LList_exists(LLItem *cfirst, char *name) {
  LLItem *ci= cfirst;
  if (!cfirst || !name) return 0;

  while (ci){
    if (ci->name) if (Strcasecmp(name, ci->name)==0) return 1;
    ci= (LLItem*)ci->next;
  }

  return 0;
}

LLItem *
LList_get(LLItem *cfirst, char *s) {
  LLItem *ci= cfirst;
  if (!cfirst || !s) return 0;

  while (ci){
    if (ci->name && Strcasecmp(s, ci->name)==0) return ci;
    ci= (LLItem*)ci->next;
  }

  return NULL;
}

void *
LList_get_data(LLItem *cfirst, char *s) {
  LLItem *ci= cfirst;
  if (!cfirst || !s) return NULL;

  while (ci){
    if (ci->name && Strcasecmp(s, ci->name)==0) return ci->data;
    ci= (LLItem*)ci->next;
  }

  return NULL;
}


LLItem *
LList_get_bydata(LLItem *cfirst, void *data) {
  LLItem *ci= cfirst;
  if (!cfirst || !data) return 0;

  while (ci){
    if (data==ci->data) return ci;
    ci= (LLItem*)ci->next;
  }

  return NULL;
}

int
LList_setdata(LLItem *cfirst, char *s, void *data) {
  LLItem *ci= cfirst;
  if (!cfirst || !s) return 0;

  while (ci){
    if (ci->name) if (Strcasecmp(s, ci->name)==0) {
      ci->data=data;
      return 1;
    }
    ci= (LLItem*)ci->next;
  }
  return 0;
}

/**************************************************
 * LList_print(cfirst) -- NOT USED
 * Prints the contents of the list to STDOUT
 *
 * LLItem cfirst;  Pointer to the first item.
 **************************************************/

int 
LList_print(LLItem *cfirst) {
  LLItem *ci= cfirst;
  if (!cfirst) return 0;

  while (ci){
    if (ci->name) fprintf(stderr, "%s %d\n", ci->name, ci->data);
    ci= (LLItem*)ci->next;
  }

  return 1;
}

/**********************************************
 * LList_remove(cfirst, name)
 * Remove a item from the list.
 *
 * LLItem *cfirst;  Pointer to the first item.
 * char *name;      Name of the item;
 **********************************************/

int 
LList_remove(LLItem *cfirst, char *name) {
  LLItem *ci= cfirst, *cl=NULL;

  if (!cfirst || !name) return 0;

  while (ci) {
    if (ci->name && Strcasecmp(ci->name, name)==0){

      cl->next= ci->next;
      Free(ci->name);
      Free(ci);
      return 1;
    } 

    cl= ci;
    ci= (LLItem*)ci->next;
  }
  return 0;
}

/**********************************************
 * LList_free(cfirst)
 * Free the memory allocated for a list.
 *
 * LLItem *cfirst;  Pointer to the first item.
 **********************************************/

int
LList_free(LLItem *ci) {
  if (!ci) return 0;

  while (ci) {
    LLItem *cl;
    Free(ci->name);
    cl= ci;
    ci= (LLItem*)ci->next;
    Free(cl);
  }
  return 1;
}

/**********************************************
 * LList_length(cfirst)
 * Calculates the length of a list
 *
 * LLItem *cfirst;  Pointer to the first item.
 *
 * Returns the number of items on a list.
 **********************************************/

int
LList_length(LLItem *ci) {
  int clen= -1;
  
  if (!ci) return -1;

  while (ci) {
    ci= (LLItem*)ci->next;
    clen++;
  }
  return clen;
}
