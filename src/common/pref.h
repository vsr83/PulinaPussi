#ifndef _PP_PREF_H_
#define _PP_PREF_H_

int   PPopt_add         (LLItem *ll, char *desc, int vtype, int lvalue, 
			 char *svalue);
int   PPopt_setvalue    (LLItem *ll, char *desc, int lvalue, char *svalue);
int   PPopt_addcb(LLItem *ll, char *desc, void (*cb)(void*), void *data);
void *PPopt_getvalue    (LLItem *ll, char *desc, int *vtype);
int   PPopt_initdefaults(IRCSession *IRCs, LLItem *ll);
int   PPopt_print       (IRCSession *IRCs, LLItem *ll, char *ds);
int   PPopt_printall    (IRCSession *IRCs, LLItem *ll);

typedef struct{
  void (*cb) (void*);
  void *cbdata;

  char *desc;
  long vtype;
  long lvalue;
  char *svalue;
} PPopt;

#define OPTTYPE_BOOL   1
#define OPTTYPE_LONG   2
#define OPTTYPE_STRING 3
#define OPTTYPE_MSG    4

#endif
