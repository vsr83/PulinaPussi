#ifndef __PP_WRAP_H__
#define __PP_WRAP_H__

/*wrappers*/
void err_exit(char *, int);
void *Calloc(size_t nmemb, size_t size);

char *Strrm  (char *, int);
char *Strdup (const char *);
char *Strchr (const char *, int);
char *Strrchr(const char *, int);
int   Strcmp (const char *, const char*);
int   Strncmp (const char *, const char*, size_t);
int   Strcasecmp (char *, char *);
size_t Strlen(const char *);

#endif
