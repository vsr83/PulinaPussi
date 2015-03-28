/*
 * wrap.c / PulinaPussi 0.12
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

#include "pp.h"

//#ifdef WIN32
int 
Strcasecmp(char *_s, char *_s2) {
  char *s, *s2;
  int i, r;

  if (!_s || !_s2) return 0;
  s= Strdup(_s);
  s2=Strdup(_s2);

  for (i=0;i<Strlen(s);i++) s[i]= toupper(s[i]);
  for (i=0;i<Strlen(s2);i++) s2[i]= toupper(s2[i]);

  r= Strcmp(s, s2);
  Free(s);
  Free(s2);

  return r;
}
//#endif

#ifdef PP_USEOLDCURSES
int
Pthread_mutex_lock(pthread_mutex_t *mutex) {
  if (pthread_mutex_lock(mutex) != 0) {
    err_exit("pthread_mutex_lock", 1);
  }
  return 0;
}

int
Pthread_mutex_unlock(pthread_mutex_t *mutex) {
  if (pthread_mutex_unlock(mutex) != 0) {
    err_exit("pthread_mutex_unlock", 1);
  }
  return 0;
}

int
Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	       void *(*start_routine)(void *), void *arg) {
  if (pthread_create(thread, attr, start_routine, arg)!=0) {
    err_exit("pthread_create", 1);
  }
  return 0;
}

int
Pthread_join(pthread_t thread, void **value_ptr) {
  if (pthread_join(thread, value_ptr)!=0) {
    err_exit("pthread_join", 1);
  }
  return 0;
}
#endif

void *
Calloc(size_t nmemb, size_t size) {
  void *p;

  p= calloc(nmemb, size);
  if (!p) err_exit("calloc\n", 0);

  return p;
}

char *
Strdup(const char *s) {
  char *p=NULL;

  if (!s) err_exit("Strdup(NULL)\n", 0);
  p= strdup(s);
  if (p==NULL) err_exit("Strdup -- out of memory?\n", 0);

  return p;
}

int
Strcmp(const char *s, const char *s2) {
  if (!s || !s2) err_exit("Strcmp(NULL)\n", 0);
  return strcmp(s, s2);
}

int
Strncmp(const char *s, const char *s2, size_t n) {
  if (!s || !s2 || n<=0) err_exit("Strcmp(NULL)\n", 0);
  return strncmp(s, s2, n);
}


char *
Strchr(const char *s, int c) {
  if (!s) err_exit("Strchr(NULL)\n", 0);
  return strchr(s, c);
}

char *
Strrchr(const char *s, int c) {
  if (!s) err_exit("Strrchr(NULL)\n", 0);
  return strrchr(s, c);
}

size_t
Strlen(const char *s) {
  if (!s) err_exit("Strlen(NULL)\n", 0);
  return strlen(s);
}
