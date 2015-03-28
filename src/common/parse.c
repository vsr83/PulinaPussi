/*
 * parse.c / PulinaPussi 0.12
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

int 
Strlenrm(char *s, int c) {
  int i=0;

  while (*s!=0 && *s != c){
    i++;
    *s++;
  }
  return i;
}

char *
Strrm(char *s, int c) {
  char *ts, *rs;
  int i;

  if (!s || *s==c || !*s) return NULL;

  ts= Strdup(s);
  for (i=0;i<strlen(ts);i++){
    if (ts[i]==c) ts[i]=0;
  }
  rs= Strdup(ts);
  Free(ts);
  return rs;
}

int
lw10(int l) {
  int i=0;
  double d;

  if (l<0){
    l*=-1;
    i++;
  }

  d=(double)l;

  while ((d/=10)>=1) i++;

  return i+1;
}

int 
powp(int l, int l2) {
  int i;
  unsigned long d=1;

  for (i=0;i++<l2;d*=l);
  return d;
}

char *ppitoa(int l) {
  char *s;
  int len, d, p, i, sp;

  len= lw10(l)-1;

  d=l;
  if (l<0)d*=-1;
  s= (char*)Calloc(1, len+2);

  p=0;
  if (l<0)s[p++]='-';
  sp=p;

  for (i=0;i<len+1-sp;i++){
    s[p++]= '0'+ d/powp(10, len-sp-i);
    d-= (d/powp(10, len-sp-i))*powp(10, len-sp-i);
  }

  return s;
}

char *
getbc(char *s, char c) {
  int i, p;
  char *r;

  if (!s) return 0;

  p=0;
  for (i=0;i<Strlen(s);i++){
    if (s[i]==c && p==0)p= i;
  }

  if (p==0) return 0;
  
  r= (char*)Calloc(1, p+1);
  memcpy(r, s, p);
  r[p]=0;
  return r;
}

static int 
s_is_dchr(char c, char *chrs) {
  int i;

  if (!chrs) return 0;
  
  for (i=0;i<Strlen(chrs);i++)
    if (c==chrs[i]) return 1;
  return 0;
}

char **
split(char *_s, char *chrs, int *rn) {
  int i;

  char **rv, *s;
  int n;  
  int Start[1025], End[1025];

  *rn=0;

  if (!_s || !chrs) return NULL;
  if (Strlen(_s)>1024) return NULL;

  //  printf("%s\n", _s);

  s= (char*)Calloc(1,  Strlen(_s)+3);
  sprintf(s, "%c%s%c", chrs[0], _s, chrs[0]);

  i=0;
  n= 0;

  Start[n]= 0;
  End[n]= 0;

  while (i<Strlen(s)){

    while (!s_is_dchr(s[i], chrs) && i<Strlen(s)){
      i++;
    }
    if (s_is_dchr(s[i],chrs) && i<Strlen(s)) {

      if (Start[n]) {
	End[n]= i;
	n++;
	Start[n]= 0;
	End[n]=   0;
      }
    }  

    while (s_is_dchr(s[i], chrs) && i<Strlen(s)){
      i++;
    }
    if (!s_is_dchr(s[i],chrs) && i< Strlen(s)){
      Start[n]= i;
      End[n]=   0;
    }    

  }
  if (n==0) {
    Free(s);
    return NULL;
  }
  rv= (char**)Calloc(1, n*(sizeof(char *)));
   
  for (i=0;i<n;i++){
    int size= End[i]- Start[i];
    
    rv[i]=(char*) Calloc(1, size+1);
    memcpy(rv[i], s+Start[i], size);
    //    rv[i][size]=0;
  }

  //  printf("%d items\n", n);

  Free(s);

  *rn= n;
  return rv;
}

int
split_free(char **a, int n) {
  int i;
  
  if (!a) return 0;

  for (i=0;i<n;i++) Free(a[i]);
  Free(a);

  return 1;
}

char *
strdup_printf(char *fmt, ...) {
  va_list ap;
  int d;

  char *ofmt= fmt;

  char c, *p, *s, *rs=NULL;
  int f, len=0, pos=0;

  va_start(ap, fmt);

  while (*fmt) {

    /*IF*/
    if (*fmt=='%'){
      switch(*++fmt){
      case 's':
	s= va_arg(ap, char *);

	if (s==NULL)break;

	len+=Strlen(s);
	f=1;
	break;
      case 'd':
	d= va_arg(ap, int);
	len+=lw10(d);
	break;
      case 'c':
	c= va_arg(ap, int);
	len++;
	break;
      }
      *fmt++;
    }
    else{
      *fmt++;
      len++;
    }
    /*EOIF*/
  }
  va_end(ap);

  rs= (char*)Calloc(1, len+1);

  fmt= ofmt;

  va_start(ap, fmt);
  pos=0;
  while (*fmt) {

    /*IF*/
    if (*fmt=='%'){
      switch(*++fmt){
      case 's':
	s= va_arg(ap, char *);
	if (s==NULL)break;

	memcpy(rs+pos, s, Strlen(s));
	pos+=Strlen(s);
	break;
      case 'd':
	{
	  char *ds= NULL;
	  d= va_arg(ap, int);

	  ds= ppitoa(d);
	  memcpy(rs+pos, ds, Strlen(ds));
	  Free(ds);
	  pos+=lw10(d);
	}
	break;
      case 'c':
	c= va_arg(ap, int);
	rs[pos++]= c;
	break;
      }
      *fmt++;
    }
    else{
      rs[pos]=*fmt;
      *fmt++;
      pos++;
    }
    /*EOIF*/
  }
  va_end(ap);


  return rs;
}
