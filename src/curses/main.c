/*
 * main.c / PulinaPussi 0.12
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

enum {
  CI_EVENT_RESIZE =1,
  CI_EVENT_NEWWINDOWS =2
};

#define PP_USECURSES

#include "../pp.h"
#include "curses.h"

/*Make GCC happy.*/

static void        Curses_update_screen(IRCSession *IRCs);
static void        Curses_close();
static int         Curses_init(IRCSession *IRCs);
static int         Curses_inputw_draw(IRCSession *IRCs);
static int         Curses_window_draw(IRCSession *IRCs, CIWindow *w);
static CIWindow   *Curses_window_getbyname(IRCSession *IRCs, char *name);
static CIWindow   *Curses_window_getbychannel(IRCSession *IRCs, char *name);
static int         Curses_window_message(IRCSession *IRCs, CIWindow *w, char *msg);

static int         Curses_window_new(IRCSession *IRCs, char *name);
static int         Curses_window_channel_add(IRCSession *IRCs, char *name, 
					     char *cname);
static int         Curses_input_handle(IRCSession *IRCs);
static void        Curses_getinput(IRCSession *IRCs);

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cht = PTHREAD_COND_INITIALIZER;


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

static void
Curses_close() {
  endwin();
}

static void
Curses_update_screen(IRCSession *IRCs) {
  int event=0;

  CInterface *ci= (CInterface *)IRCs->interface;

  if (ci->width != getmaxx(stdscr) || ci->height != getmaxy(stdscr)){
    event+= CI_EVENT_RESIZE;
  }
  
  if (ci->nwindows != LList_length(ci->Wlist)){
    event+= CI_EVENT_NEWWINDOWS;
  }

  /*SCREEN SIZE CHANGED*/
  if (event & CI_EVENT_RESIZE){
    LLItem *wi= ci->Wlist;
    ci->width =getmaxx(stdscr);
    ci->height=getmaxy(stdscr);
    
    while (wi!=NULL) {
      if (wi->name) {
	Curses_window_draw(IRCs, (CIWindow*)wi->data);
      }
      wi= (LLItem*) wi->next;
    }
  }

  /*New/Deleted/Modified Windows*/
  if (event & CI_EVENT_NEWWINDOWS || (event & CI_EVENT_RESIZE && ci->nwindows)){
    char s[512];
    int avgheight, n=0;
    LLItem *li;

    ci->nwindows= LList_length(ci->Wlist);
    avgheight= (ci->height-2) / ci->nwindows;

    sprintf(s, "%d windows found\navgheight= %d\n", ci->nwindows, avgheight);
    Curses_window_message(IRCs,Curses_window_getbychannel(IRCs, "#testi"), s);    
    
    li= ci->Wlist;

    while (li) {

      if (li->data) {
	CIWindow *w= (CIWindow *) li->data;

	if (n== ci->nwindows-1) {
	  w->height= ci->height-avgheight*n-2, ci->width;
	} else {
	  w->height= avgheight;
	}
	wresize(w->w, w->height, ci->width); 

	mvwin(w->w, n*avgheight, 0);
	w->y= n*avgheight;

	//	wclear(w->w);
	//wrefresh(w->w);
	Curses_window_draw(IRCs, w);


	sprintf(s, "%d/%d %s size - %d - %d s(%dx%d)\n", n,
		ci->nwindows,w->name, n*avgheight, n*avgheight+avgheight,
		ci->width, ci->height);
	Curses_window_message(IRCs,Curses_window_getbychannel(IRCs, "#testi"), s);

	n++;
      }       
      li= (LLItem *)li->next;
    }

    wresize(ci->inputW, 2, ci->width);
    mvwin(ci->inputW, ci->height-2, 0);
    wsetscrreg(ci->inputW, 1, 1);
    
    wclear(ci->inputW);
    wrefresh(ci->inputW);    
  }

  Curses_inputw_draw(IRCs);
}

static int
Curses_init(IRCSession *IRCs) {
  CInterface *ci;
  IRCs->interface = Calloc(1, sizeof(CInterface));
  ci= (CInterface *)IRCs->interface;
  
  ci->cmdhist= LList_new();
  ci->histpos= -1;

  initscr();
  keypad(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();

  if (has_colors()) {
    start_color();

    init_pair(1 ,COLOR_BLACK,  COLOR_BLACK);
    init_pair(2 ,COLOR_BLUE,   COLOR_BLACK);
    init_pair(3 ,COLOR_GREEN,  COLOR_BLACK);
    init_pair(4 ,COLOR_RED  ,  COLOR_BLACK);
    init_pair(5 ,COLOR_RED  ,  COLOR_BLACK);

    init_pair(6, COLOR_MAGENTA,COLOR_BLACK);
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    init_pair(8, COLOR_YELLOW, COLOR_BLACK);
    init_pair(9, COLOR_GREEN,  COLOR_BLACK);
    init_pair(10,COLOR_CYAN,   COLOR_BLACK);

    init_pair(11,COLOR_CYAN,   COLOR_BLACK);
    init_pair(12,COLOR_BLUE,   COLOR_BLACK);
    init_pair(13,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(14,COLOR_WHITE,  COLOR_BLACK);
    init_pair(15,COLOR_WHITE,  COLOR_BLACK);

    init_pair(20,COLOR_BLACK,  COLOR_WHITE);
    init_pair(21,COLOR_WHITE,  COLOR_BLUE);
  };

  ci->inputW= newwin(2, COLS, LINES-2, 0);
  ci->Wlist= (LLItem*)LList_new();
  ci->nwindows= 0;

  keypad(ci->inputW, TRUE);
  scrollok(ci->inputW, TRUE);
  wsetscrreg(ci->inputW, 1, 1);

  signal(SIGINT, (void(*)(int))Curses_close);

  Curses_update_screen(IRCs);
}

#define MAX_WINDOWS 5

static void
Curses_refreshall(IRCSession *IRCs) {
  LLItem *wlist;
  CInterface *ci= (CInterface *)IRCs->interface;
  wlist= ci->Wlist;

  while (wlist) {
    CIWindow *w= (CIWindow*)wlist->data;
    if (w) {
      wrefresh(w->w);
    }
    wlist= (LLItem*) wlist->next;
  }
}

static int
Curses_inputw_draw(IRCSession *IRCs) {
  CInterface *ci= (CInterface *)IRCs->interface;
  int i;
  
  if (!ci) return FALSE;

  wattron(ci->inputW, A_REVERSE | A_BOLD);
  
  for (i=0;i<ci->width;i++){
    mvwaddch(ci->inputW, 0, i,' ');
    mvwaddch(ci->inputW, 1, i,' ');
  }
  mvwprintw(ci->inputW, 0, 0, "- %s -", IRCs->curchan);
  wattroff(ci->inputW, A_REVERSE | A_BOLD);
  mvwprintw(ci->inputW, 1, 0, "%s> %s", IRCs->ircnick, ci->cmd);
  wrefresh(ci->inputW);
  return TRUE;
}

static int
Curses_window_draw(IRCSession *IRCs, CIWindow *w) {
  int i;
  CInterface *ci= (CInterface *)IRCs->interface;

  if (!ci || !w) return FALSE;

  //  wattron(w->w, COLOR_PAIR(20));
  wattron(w->w, A_REVERSE | A_DIM);
  
  for (i=0;i<ci->width;i++)
    mvwaddch(w->w, 0, i,' ');
  mvwprintw(w->w, 0, 0," [%s] - (%d)", w->name, LList_length(w->channels));
  {
    LLItem *li = w->channels;
    int origy= w->w->_cury;

    while (li) {
      /*If text required to display whole chanlist exceeds one line, don't 
        display everything.*/
      if (li->name) {
	if (w->w->_curx + Strlen(li->name) +5< ci->width) {
	  wprintw(w->w, " - %s", li->name);
	} else {
	  mvwprintw(w->w, 0, ci->width-3, "->");
	}
      }
      li= (LLItem*) li->next;
    }
  }
  wattroff(w->w, A_REVERSE | A_DIM);
  // wattroff(w->w, COLOR_PAIR(20));
  wrefresh(w->w);

  wsetscrreg(w->w, 1, getmaxy(w->w)-1);

  return TRUE;
}

static CIWindow *
Curses_window_getbyname(IRCSession *IRCs, char *name) {
  CInterface *ci= (CInterface *)IRCs->interface;

  if (!ci || !name) return NULL;
  return (CIWindow*)LList_get_data(ci->Wlist, name);
}

static CIWindow *
Curses_window_getbychannel(IRCSession *IRCs, char *name) {
  LLItem *li;
  CInterface *ci= (CInterface *)IRCs->interface;

  if (!ci || !name) return NULL;

  li= ci->Wlist;

  while (li) {
    if (li->data) {
      CIWindow *w= (CIWindow *) li->data;
      if (LList_get(w->channels, name)) return w;
    }    
    li= (LLItem *)li->next;
  }
  return NULL;
}

static int
Curses_window_message(IRCSession *IRCs, CIWindow *w, char *msg) {
  int has_color=FALSE, i;
  CInterface *ci= (CInterface *)IRCs->interface;

  if (!ci || !w || !msg) return FALSE;

  for (i=0;i<Strlen(msg);i++) if (msg[i]=='\03') has_color= TRUE;

  wmove(w->w, getmaxy(w->w)-1, 0);

  if (has_color) {
    int lcolor= 0, color= 0;
    for (i=0;i<Strlen(msg);i++) {
      if (msg[i] == '\03' && msg[i+1]) {
	lcolor= color;
	
	if (isdigit(msg[i+1]) && !isdigit(msg[i+2])) {
	  color= msg[++i]-'0';
	} else if (isdigit(msg[i+1]) && isdigit(msg[i+2])) {
	  color= (msg[++i]-'0')*10 + msg[++i]-'0';
	}
	color= color%16;
	if (color) {
	  wattron(w->w, COLOR_PAIR(color));
	}else {
	  wattroff(w->w, COLOR_PAIR(lcolor));
	}
      }else {
	wprintw(w->w, "%c", msg[i]);
      }
    }
    wattroff(w->w, COLOR_PAIR(color));
  }else {
    wprintw(w->w, "%s", msg);
  }

  wrefresh(w->w);
  return TRUE;
}

static int
Curses_window_new(IRCSession *IRCs, char *name) {
  CInterface *ci= (CInterface *)IRCs->interface;
  CIWindow *w;
  int nwin;

  nwin= LList_length(ci->Wlist);
  {
    int vtype, maxwin;
    maxwin= (int)PPopt_getvalue(IRCs->optlist, "max_windows", &vtype);
    if (vtype!=OPTTYPE_LONG) maxwin= 0;
    if (nwin >= maxwin) {
      Client_msgf(IRCs, 0, "[ERROR]", "Too many windows\n", FALSE);
      return 0;
    }
  }

  if (nwin > MAX_WINDOWS) return FALSE;

  w= (CIWindow*)Calloc(1, sizeof(CIWindow));
  w->name= (char*)Strdup(name);

  if (nwin==0) {
    w->w= newwin(LINES-2, COLS, 0, 0);
  }else {
    w->w= newwin(1, 1, 0, 0);
  }

  w->channels= (LLItem*)LList_new();
  w->aheight = 0;

  scrollok(w->w, TRUE);

  LList_add(ci->Wlist, name, w);
  Curses_window_draw(IRCs, Curses_window_getbyname(IRCs, name));
  Curses_update_screen(IRCs);
  return TRUE;
}

int
Curses_window_destroy(IRCSession *IRCs, char *name) {
  CInterface *ci= (CInterface *)IRCs->interface;
  CIWindow *w;

  if (!Strcasecmp(name, "main")) {
    Client_msgf(IRCs, 0, "[ERROR]", "Can't destroy main-window\n", FALSE);    
    return FALSE;
  }

  w= Curses_window_getbyname(IRCs, name);
  if (w && LList_remove(ci->Wlist, name)) {
    if (w->channels) {
      LLItem *li= w->channels;
      CIWindow *mainw;
      
      mainw= Curses_window_getbyname(IRCs, "main");
      
      while (li) {
	if (li->name) LList_add(mainw->channels, li->name, NULL);
	li= (LLItem*) li->next;
      }
    }
    
    Free(w->name);
    Free(w->curchan);
    LList_free(w->channels);
    Free(w);
  }
  return TRUE;
}

static int
Curses_window_channel_remove(IRCSession *IRCs, char *cname) {
  CIWindow *w;

  w= Curses_window_getbychannel(IRCs, cname);
  if (w) {
    LList_remove(w->channels, cname);
    Curses_window_draw(IRCs, w);
  }
}

static int
Curses_window_channel_add(IRCSession *IRCs, char *name, char *cname) {
  CIWindow *w;
  CInterface *ci= (CInterface *)IRCs->interface;
  int nchannels;

  w= Curses_window_getbyname(IRCs, name);
  if (!w) return FALSE;

  nchannels= LList_length(w->channels);
  {
    int maxchan, vtype;
    maxchan= (int)PPopt_getvalue(IRCs->optlist, "max_channelsperwin", &vtype);
    if (vtype!=OPTTYPE_LONG) maxchan= 0;
    if (nchannels>=maxchan) {
      Client_msgf(IRCs, 0, "[ERROR]", 
		  "Too many channels!\n", FALSE); 
      return 0;
    }
  }

  if (w->curchan) Free(w->curchan);
  w->curchan= Strdup(cname);

  if (LList_add(w->channels, cname, NULL)) {
    Curses_window_draw(IRCs, w);
    return 1;
  } else {
    return 0;
  }
  return 0;
}

static int
Curses_cmdhist_add(IRCSession *IRCs, char *cmd) {
  CInterface *ci= (CInterface *)IRCs->interface;
  int hist_maxlen, vtype;
  char mbuf[512], *s;
    
  hist_maxlen= (int)PPopt_getvalue(IRCs->optlist, "cmdhistory_maxlength", 
				   &vtype);
  if (vtype!=OPTTYPE_LONG) err_exit("optlist sucks", 0);
  
  if (LList_length(ci->cmdhist) > hist_maxlen+1) {
    if (ci->cmdhist->next) {
      LLItem *ri= (LLItem*) ci->cmdhist->next;
      if (ri->data) Free(ri->data);
      LList_remove(ci->cmdhist, ri->name);
    }
  }
  s= Strdup(cmd);
  snprintf(mbuf, 510, "%d\n", s); 
  LList_add(ci->cmdhist, mbuf, s);
  return TRUE;
}

static int
Curses_input_handle(IRCSession *IRCs) {
  char buf[513];
  int cmddone= FALSE;
  CInterface *ci= (CInterface *)IRCs->interface;

  Curses_cmdhist_add(IRCs, ci->cmd);

  if (*ci->cmd == '/') {
    char **sv;
    int n;

    sv= split(ci->cmd, " ", &n);

    if (!Strcasecmp("/CLEAR", sv[0])) {
      CIWindow *ci;
      ci= Curses_window_getbychannel(IRCs, IRCs->curchan);
      if (ci) {
	Curses_window_draw(IRCs, ci);
	wclear(ci->w);
	Curses_window_draw(IRCs, ci);
      }else {
	ci= Curses_window_getbyname(IRCs, "main");
	if (ci) wclear(ci->w);
	Curses_window_draw(IRCs, ci);
      }
      cmddone=TRUE;
    }

    
    if (!Strcasecmp("/WINDOW", sv[0])) {

      if (n==1) {/*List Windows*/
	LLItem *li= ci->Wlist;
	while (li) { 	  
	  if (li->data) {
	    CIWindow  *cw= (CIWindow*) li->data;
	    
	    Client_msgf(IRCs, 0, "+", 
			strdup_printf("%s %dx%d [%d channels]\n", 
				      cw->name,
				      ci->width,
				      cw->height,
				      LList_length(cw->channels)
				      ),
				      TRUE);
	    { /*List Channels*/
	      LLItem *lc= cw->channels;
	      
	      while (lc) {
		if (lc->name)Client_msgf(IRCs, 0, "  ->", 
					 strdup_printf("%s\n", lc->name),
					 TRUE);
		
		lc= (LLItem *) lc->next;
	      }
	    }
	  }
	  li= (LLItem*) li->next;	  
	}
      }



      if (n>1) {

	if (!Strcasecmp(sv[1], "GROW")) {
	    Client_msgf(IRCs, 0, "[ERROR]", "Not implemented yet.\n",
			FALSE);

	  if (n<3) {
	    Client_msgf(IRCs, 0, "[ERROR]", "/WINDOW GROW [n] win1 win2\n",
			FALSE);
	  } else {
	    int nrows= atoi(sv[2]);

	    if (n==3) {
	      CIWindow *w= Curses_window_getbyname(IRCs, "main");
	    }
	    if (n==4) {
	      CIWindow *w= Curses_window_getbyname(IRCs, sv[3]);
	    }
	    if (n==5) {
	      CIWindow *w, *w2;
	      w=  Curses_window_getbyname(IRCs, sv[3]);
	      w2= Curses_window_getbyname(IRCs, sv[4]);
	    }
	  }
	}

	if (!Strcasecmp(sv[1], "NEW")) {
	  if (n!=3) {
	    Client_msgf(IRCs, 0, "[ERROR]", "/WINDOW NEW [name]\n", FALSE);
	  } else {
	    Curses_window_new(IRCs, sv[2]);
	  }
	}

	if (!Strcasecmp(sv[1], "CLOSE")) {
	  if (n!=3) {
	    Client_msgf(IRCs, 0, "[ERROR]", "/WINDOW CLOSE [name]\n", FALSE);
	  } else {
	    Curses_window_destroy(IRCs, sv[2]);
	  }
	}

	if (!Strcasecmp(sv[1], "VOMIT")){
	  if (n!=3) {
	    Client_msgf(IRCs, 0, "[ERROR]", 
			"/WINDOW VOMIT [channel]\n", FALSE);
	  }else {
	    Curses_window_channel_remove(IRCs, sv[2]);
	  }
	}

	if (!Strcasecmp(sv[1], "SWALLOW")){
	  if (n!=4) {
	    Client_msgf(IRCs, 0, "[ERROR]", 
			"/WINDOW SWALLOW [win] [channel]\n", FALSE);
	  } else {
	    CIWindow *src, *dest;
	    
	    src= Curses_window_getbychannel(IRCs, sv[3]);
	    dest=Curses_window_getbyname   (IRCs, sv[2]);
	    if (dest) {
	      if (src) {
		Curses_window_channel_remove(IRCs, sv[3]);
	      }
	      Curses_window_channel_add(IRCs, sv[2], sv[3]);
	    }else {
	      if (!dest) {
		Client_msgf(IRCs, 0, "[ERROR]", 
			    strdup_printf("can't find window %s\n", sv[2]), 
			    TRUE);
	      }
	    }
	  }      
	}
      }
      cmddone=TRUE;
    }
  
    split_free(sv, n);
  }

  memset(buf, 0, 513);
  snprintf(buf, 512, "%s\n", ci->cmd);
  if (!cmddone) IRC_input_handle(IRCs, ci->cmd);

  return TRUE;
}

static void
Getsocket(IRCSession *IRCs) {
  for (;;) {
    Socket_readln(IRCs);

    Pthread_mutex_lock(&lock);
    if (!IRCs->connected) usleep(10000); /*Replace this!*/
    else {
      Debug_msgf(IRCs, IRCs->sockbuf, 0);
      Debug_msgf(IRCs, "\n", 0);
      IRC_handle(IRCs, IRCs->sockbuf);
    }
    Pthread_mutex_unlock(&lock);
  }
}

static void
Curses_getinput(IRCSession *IRCs) {
  CInterface *ci= (CInterface *)IRCs->interface;

  memset(ci->cmd, 0, 512);

  for(;;) {
    int ch;

    ch= wgetch(ci->inputW);

    if (ch==13) ch= KEY_ENTER;
    if (ch== 8) ch= KEY_BACKSPACE;

    Pthread_mutex_lock(&lock);

    if (ch != KEY_UP && ch!= KEY_DOWN) ci->histpos=-1;

    switch(ch) {
    case KEY_UP:{ 
      if (ci->histpos == -1) {
	Curses_cmdhist_add(IRCs, ci->cmd);
	ci->histpos++;
      }
      ci->histpos++;
    }
      break;
    case KEY_DOWN:{
      ci->histpos--;
      if (ci->histpos<-1) ci->histpos=-1;
    }
    case KEY_LEFT:
      if (ci->cmdp>0) ci->cmdp--;
      break;
    case KEY_RIGHT:
      if (ci->cmd[ci->cmdp]) ci->cmdp++;
      break;
    case KEY_ENTER:
      Curses_input_handle(IRCs);
      wclear(ci->inputW);
      ci->cmdp=0;
      memset(ci->cmd, 0, 512);
      break;
    case KEY_BACKSPACE:
      if (ci->cmdp>0) ci->cmd[--ci->cmdp]=0;
      break;
    }

    if (ch == KEY_UP || ch == KEY_DOWN) {
      LLItem *li= ci->cmdhist;
      
      if (ci->histpos>-1){
	int n, len, i;
	len= LList_length(ci->cmdhist);
	n= len-ci->histpos;
	if (n<1) {
	  n=1;
	  ci->histpos--;
	}
	for (i=0;i<n;i++) {
	  li= (LLItem*)li->next;
	}
	if (li && li->data) {
	  memset(ci->cmd, 0, 512);
	  memcpy(ci->cmd, li->data, Strlen((char*)li->data));
	  ci->cmdp= Strlen((char*)li->data);
	}
      } 
      
    }

    if (ch < 256) {
      ci->cmd[ci->cmdp++]=ch;
      if (ci->cmdp>510) ci->cmdp=510;
    }
    Curses_inputw_draw(IRCs);
    Curses_update_screen(IRCs);

    Pthread_mutex_unlock(&lock);
  }
}



/*mm*/

int
Client_msgf(IRCSession *IRCs, int oper, char *to, char *s, int freeit) {
  if (!to || !s) return FALSE;
  if (oper>2) {
    if (freeit) Free(s);
    return FALSE;
  }

  {
    CIWindow *cw;
    
    cw= Curses_window_getbychannel(IRCs, to);
    if (!cw) {
      cw= Curses_window_getbyname(IRCs, "main");
    }
    if (cw) {
      char *msg;
      
      msg= strdup_printf("%s %s", to, s);
      Curses_window_message(IRCs, cw, msg);
      Free(msg);
    }
  }
  return TRUE;
}

int
Client_drawp(IRCSession *IRCs) {
  Curses_update_screen(IRCs);
  return TRUE;
}

int
Debug_msgf(IRCSession *IRCs, char *msg, int freeit) {
  if (freeit) Free(msg);

#ifdef DEBUG
#endif
  return FALSE;
}

void
err_exit(char *msg, int do_perror) {
#ifdef PP_USECURSES
  endwin();
#endif

  if (msg) {
    if (do_perror) perror(msg);
    else fprintf(stderr, "%s\n", msg);
  }
  exit(0);
}

int
IRC_server(IRCSession *IRCs,char *s) {
  char *ihost, *ports;
  int port= DEFAULTPORT;

 if (!s) return FALSE;
  ihost= getbc(s, ':');
  if (!ihost) ihost= Strdup(s);

  ports= Strchr(s, ':');
  if (ports && isdigit(*(ports+1))) port= atoi(ports+1);
  
  if (ihost) {
    if (IRCs->connected) {
      Socket_outf(IRCs, "QUIT\n", 0);
      close (IRCs->sockfd);
      Client_msgf(IRCs, 0, "[server]", "disconnected.\n", 0);
      IRCs->connected= 0;
    }
    Free(IRCs->ircserver);
    IRCs->ircserver= Strdup(ihost);
    IRCs->ircport= port;
    Client_connect(IRCs, IRCs->ircserver, IRCs->ircport);
    return TRUE;
  }
  return FALSE;
}

int
main(int argc, char **argv) {
  IRCSession *IRCs;
  struct passwd *pw;
  int n;

  IRCs= (IRCSession*) Calloc(1, sizeof(IRCSession));
  Curses_init(IRCs);

  pw= getpwuid(getuid());
  if (!pw) err_exit("getpwuid\n", 0);

  for (n=1;n<argc;n++) {
    if (*argv[n] == '-' && n < argc){
      switch (*(argv[n]+1)) {
      case 's':
	{
	  char *s=NULL;
	  IRCs->ircserver= getbc(argv[n+1], ':');
	  if (!IRCs->ircserver) IRCs->ircserver= Strdup(argv[n+1]);

	  s= Strchr(argv[n+1], ':');
	  if (s && isdigit(*(s+1))) IRCs->ircport= atoi(s+1);
	  if (IRCs->ircport==0) IRCs->ircport= DEFAULTPORT;
	  break;
	}
      case 'n':
	IRCs->ircnick= Strdup(argv[n+1]);
	break;
      case 'r':
	IRCs->ircrealname= Strdup(argv[n+1]);
	break;
      }
    }
  }
    
  if (!IRCs->ircnick) IRCs->ircnick= Strdup(pw->pw_name);
  if (!IRCs->ircrealname) IRCs->ircrealname= Strdup(pw->pw_gecos);
  
  IRCs->chanlist=  LList_new();
  IRCs->optlist=   LList_new();
  IRCs->pluginlist=LList_new();

  PPopt_initdefaults(IRCs, IRCs->optlist);
  PPopt_add(IRCs->optlist, "msg2", OPTTYPE_MSG, FALSE, 
	    "- Curses-interface ------------\n");

  PPopt_add(IRCs->optlist, "max_windows", OPTTYPE_LONG, 10, NULL);
 
  PPopt_add(IRCs->optlist, "max_channelsperwin", OPTTYPE_LONG, 50, NULL);
  PPopt_add(IRCs->optlist, "cmdhistory_maxlength", OPTTYPE_LONG, 25, NULL);


  Curses_window_new(IRCs, "main");


  //  Curses_getinput(IRCs);
  {
    pthread_t th1, th2;
    Pthread_create(&th1, NULL, (void *(*)(void*))Curses_getinput, IRCs);
    Pthread_create(&th2, NULL, (void *(*)(void*))Getsocket, IRCs);

    Pthread_join(th1, NULL);
    Pthread_join(th2, NULL);
  }

  Curses_close();
  return TRUE;
}
