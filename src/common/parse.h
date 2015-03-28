#ifndef _PP_PARSE_H_
#define _PP_PARSE_H_

int   lw10           (int);
char *getbc         (char *, char);
char **split        (char *, char *, int *);
int split_free(char **a, int n);
//char *strcon        (char *, ...);
char *strdup_put    (char *, ...);
char *strdup_printf (char *, ...);
int   Strlenrm(char *s, int c);

#endif
