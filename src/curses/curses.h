#ifndef __PP_CURSES_H__
#define __PP_CURSES_H__

#include <curses.h>
#include <pthread.h>

typedef struct {
  char *name, *curchan;
  WINDOW *w;
  LLItem *channels;

  int height;
  int y;
  /*Height = ci->height / nwindows + additional height*/
  int aheight;

}CIWindow;

typedef struct {
  WINDOW *inputW;
  LLItem *Wlist;
  LLItem *cmdhist;

  int histpos;

  int width, height;
  int nwindows;

  char cmd[512];
  int cmdp;
}CInterface;

#endif
