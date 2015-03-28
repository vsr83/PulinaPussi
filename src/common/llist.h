#ifndef __LLIST_H__
#define __LLIST_H__

typedef struct{
  char *name;
  void *next;
  void *data;
} LLItem;

LLItem *LList_new();
LLItem *LList_get (LLItem *cfirst, char *s);

int LList_length  (LLItem *cfirst);
int LList_exists  (LLItem *cfirst, char *s);

void *LList_get_data(LLItem *cfirst, char *s);

int LList_add    (LLItem *cfirst, char *s, void *data);
int LList_free   (LLItem *cfirst);
int LList_print  (LLItem *cfirst);
int LList_remove (LLItem *cfirst, char *s);
int LList_setdata(LLItem *cfirst, char *s, void *data);

#endif
