/*
 * curses.c / PulinaPussi 0.11
 * written by Ville Räisänen <raivil@geek.com> 2001, 2002
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

WINDOW *w, *mw, *debugw;

/**************************
 * Curses_init()
 * Initialize the screen
 **************************/

int
Curses_init() {
  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak(); 
  noecho(); 
 
  if (has_colors()){
    start_color();
    
    init_pair(1, COLOR_BLACK,    COLOR_BLACK);
    init_pair(2, COLOR_BLUE,     COLOR_BLACK);
    init_pair(3, COLOR_GREEN,    COLOR_BLACK);
    init_pair(4, COLOR_RED,      COLOR_BLACK);
    init_pair(5, COLOR_RED,      COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA,  COLOR_BLACK);
    init_pair(7, COLOR_YELLOW,   COLOR_BLACK);
    init_pair(8, COLOR_YELLOW,   COLOR_BLACK);
    init_pair(9, COLOR_GREEN,    COLOR_BLACK);
    init_pair(10,COLOR_CYAN,     COLOR_BLACK);
    init_pair(11,COLOR_CYAN,     COLOR_BLACK);
    init_pair(12,COLOR_BLUE,     COLOR_BLACK);
    init_pair(13,COLOR_MAGENTA,  COLOR_BLACK);
    init_pair(14,COLOR_WHITE,    COLOR_BLACK);
    init_pair(15,COLOR_WHITE,    COLOR_BLACK);
    init_pair(20,COLOR_WHITE,    COLOR_BLUE);
  }



#ifdef DEBUG
  w=  newwin(2, COLS, LINES-2, 0);
  mw= newwin(LINES-16, COLS, 14, 0);
  debugw= newwin(14, COLS, 0, 0);
  scrollok(debugw, TRUE);
#else
  w=  newwin(2, COLS, LINES-2, 0);
  mw= newwin(LINES-2, COLS, 0, 0);
#endif

  keypad(w, TRUE);
  scrollok(w, TRUE);
  scrollok(mw, TRUE);

  return TRUE;
}

int
Debug_msgf(IRCSession *IRCs, char *msg, int freeit) {
#ifdef DEBUG
  if (!msg) return FALSE;

  wprintw(debugw, "%s", msg);
  wrefresh(debugw);

  if (freeit) Free(msg);
  
  return TRUE;
#endif
  return FALSE;
}

/**********************************************
 * Client_msgf(to, oper, s, freeit)
 * Print message in window.
 *
 * int oper;    Operation
 * char *_to;   Origin/Type of the message.
 * char *s;     The message.
 * int freeit;  Free memory allocated for s?
 **********************************************/

int
Client_msgf(IRCSession *IRCs, int oper, char *_to, char *s, int freeit) {
  char *to=NULL;
 
  if (!_to || !s) return FALSE;
  if (oper>2) {
    if (freeit)Free(s);
    return FALSE;
  }
  to= Strdup(_to);

  if (to){
    int i;
    for (i=0;i<Strlen(to);i++)
      to[i]= toupper(to[i]);
  }

  wattron(mw, COLOR_PAIR(10));
  wprintw(mw, "%s ", to);
  wattroff(mw, COLOR_PAIR(10));

  if (to[0]=='[')
    wattron(mw, COLOR_PAIR(10));
  {  
    int i, has_color= FALSE;

    for (i=0;i<Strlen(s);i++) if (s[i]=='\03') has_color=TRUE;
    if (!PPopt_getvalue(IRCs->optlist, "show_colors", NULL)) has_color=FALSE;

    if (has_color) {
      int lcolor=0, color=0;
	
      for (i=0;i<Strlen(s);i++) {
	if (s[i]=='\03' && s[i+1]) {
	  lcolor=color;
	  /*atoi?*/
	  if (isdigit(s[i+1]) && !isdigit(s[i+2])) {
	    color= s[++i]-'0';
	  } else if (isdigit(s[i+1]) && isdigit(s[i+2])) {
	    color= (s[++i]-'0')*10 + s[++i]-'0';
	  }
	  color=color%16;
	  if (color) {
	    wattron(mw, COLOR_PAIR(color));
	  }else {
	    wattroff(mw, COLOR_PAIR(lcolor));
	  }
	}else {
	  wprintw(mw, "%c", s[i]);
	}
      }
      wattroff(mw, COLOR_PAIR(color));

    }else {
      wprintw(mw, "%s", s);
    }
  }

  wrefresh(mw);

  if (to[0]=='[')
    wattroff(mw, COLOR_PAIR(4));
  
  Free(to);
  if (freeit)Free(s);

  return TRUE;
}

/**********************************
 * Client_drawp()
 * Redraw the status window. 
 **********************************/

int
Client_drawp(IRCSession *IRCs) {
  int l=0, l2=0, i;
  
  if (IRCs->curchan) l= Strlen(IRCs->curchan);
  else l=0;
  if (IRCs->ircnick) l2=Strlen(IRCs->ircnick);
  else l2=0;
  
  wattron(w, COLOR_PAIR(20));
  
  for (i=0;i<COLS;i++) {
    mvwaddch(w, 0,i,  ' ');
    mvwaddch(w, 1,i,  ' ');
  }
  if (IRCs->curchan) mvwprintw(w, 0, 0, "%s", IRCs->curchan);
  wattroff(w, COLOR_PAIR(20));
  if (IRCs->ircnick) mvwprintw(w, 1, 0, " %s > %s", IRCs->ircnick, IRCs->pcmd);
  wmove(w, 1, Strlen(IRCs->ircnick) + IRCs->pcmdp + 4);
  wrefresh(w);

  return TRUE;
}

int
Inputf_getch(){
  int ch;
  ch= wgetch(w);
  return ch;
}

int
Inputf_clear(){
  wclear(w);
  wrefresh(w);

  return 1;
}

int
Curses_clearall(){
  wclear(w);
  wclear(mw);
  wrefresh(w);
  wrefresh(mw);
  return TRUE;
}
