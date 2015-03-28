/*
 * main.c / PulinaPussi 0.11
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

#include <curses.h>
#include <signal.h>
#include <pthread.h>

#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../pp.h"

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cht = PTHREAD_COND_INITIALIZER;

IRCSession *IRCs;

int
IRC_server(IRCSession *IRCs, char *s) {
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

static void
input_handle() {
  if (!Strcasecmp(IRCs->pcmd, "/clear")) {
    Curses_clearall();
    return;
  }

  IRC_input_handle(IRCs, IRCs->pcmd);
}

/*Thread used to read input from the user.*/
static void*
getinput(void *idp) {
  memset(IRCs->pcmd, 0, 512);

  IRCs->pcmdp=0;
  for (;;){
    int ch;

    ch= Inputf_getch();
    if (ch==13) ch= KEY_ENTER;
    if (ch== 8) ch= KEY_BACKSPACE;
   
    Pthread_mutex_lock(&lock);

    switch(ch) {
    case KEY_LEFT:
      if (IRCs->pcmdp>0) IRCs->pcmdp--;
      break;
    case KEY_RIGHT:
      if (IRCs->pcmd[IRCs->pcmdp]) IRCs->pcmdp++;
      break;
    case KEY_ENTER:
      IRCs->pcmdp=0;
      Inputf_clear();
      input_handle();
      memset(IRCs->pcmd, 0, 512);
      break;
    case KEY_BACKSPACE:
      if (IRCs->pcmdp>0)IRCs->pcmd[--IRCs->pcmdp]=0;
      break;
    }

    if (ch<256) {
      IRCs->pcmd[IRCs->pcmdp++]= ch;
      if(IRCs->pcmdp>510)IRCs->pcmdp=510;
    }

    Client_drawp(IRCs);
    Pthread_mutex_unlock(&lock);
    
  }    

  return NULL;
}

/*Thread used to read input from the socket.*/
static void*
getsocket(void *idp) {
  for (;;){
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

  return NULL;
}

static void
show_usage(char *argv0) {
  printf("Usage: %s -s [server:port] -n [nick] -r [realname] -l [logfile]\n", 
	 argv0);
}

int
main(int argc, char *argv[]) {
  struct passwd *pw;
  int n;

  if (argc==2 && (!Strcmp(argv[1], "-help") || !Strcmp(argv[1], "--help"))) {
    show_usage(argv[0]);
    exit(0);
  }

  IRCs= (IRCSession*)Calloc(1, sizeof(IRCSession));

  pw= getpwuid(getuid());
  if (!pw) err_exit("getpwuid\n", 0);

  for (n=1;n<argc;n++) {
    if (*argv[n]=='-' && n<argc){
      switch(*(argv[n]+1)) {
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
      case 'l':
	//	IRCs->logfilename= Strdup(argv[n+1]);
	break;
      }
    }
  }
  //  if (!IRCs->ircserver || !IRCs->ircport) err_exit("invalid server/port\n", 0);
  if (!IRCs->ircnick) IRCs->ircnick= Strdup(pw->pw_name);
  if (!IRCs->ircrealname) IRCs->ircrealname= Strdup(pw->pw_gecos);

  IRCs->chanlist=   LList_new();
  IRCs->optlist=    LList_new();
  IRCs->pluginlist= LList_new();

  signal(SIGINT, finish);
  
  Curses_init();

  /*  if (IRCs->logfilename) {
    IRCs->logfile= fopen(IRCs->logfilename, "a");
    if (!IRCs->logfile) {
      err_exit("Can't open logfile\n", 0);    
    }
    }*/

  /*A stupid way of looking for memory leaks and buffer overflows.*/

 if (!PPLoadPlugin(IRCs, "../plugins/libtesti.so.0.0")) exit(0);

#ifdef SEGF_FIND
  {
    static int n=0;
    int ofd;

    srand(time(NULL));

    ofd= open("/dev/null", O_WRONLY);
    IRCs->sockfd= ofd;

    for (;;){
      char s[1024], s2[1024], s3[1024], s4[1024];int i;
      memset(s, 0, 1023);
      memset(s2, 0, 1023);
      memset(s3, 0, 1023);
      memset(s4, 0, 1023);

      for (i=0;i<256;i++){
	s[i]=rand()%256;
	s2[i]=rand()%256;
	s3[i]=rand()%256;
	s4[i]=rand()%256;
      }

      if (rand()%10>8) *s=0;
      if (rand()%10>8) *s2=0;
      if (rand()%10>8) *s3=0;
      if (rand()%10>8) *s4=0;

      i=rand()%600;
      /*           
      handle_numeric(IRCs, s, s, s, s);

      IRC_handle(IRCs,s);
      
      htopic(IRCs,s, s2, s3);
      hnotice(IRCs,s, s2, s3);
      hjoin(IRCs,s, s2, s3);
      hprivmsg(IRCs,s, s2, s3);
      hprivmsg(IRCs,s, s2, "\01FINGER");
      hprivmsg(IRCs,s, s2, "\01VERSION");
      hprivmsg(IRCs,s, s2, "\01PING");
      hprivmsg(IRCs,s, s2, "\01TIME");
      hprivmsg(IRCs,s, s2, "\01ACTION\01APINA");
      */


      /* TÄÄLLÄ JOSSAIN
      hmode(IRCs,s, s2, s3);
      hinvite(IRCs,s, s2, s3);
      hpart(IRCs,s, s2, s3);
      hkick(IRCs,s, s2, s3);
      hnick(IRCs,"spuuki!apina", s2, s3);
      hnick(IRCs,s, s2, s3);
      
      */
      IRC_input_handle(IRCs,s);
      
      s3[0]= '\01';
      hprivmsg(IRCs,s, s2, s3);
      
      IRC_op(IRCs,s);
      IRC_deop(IRCs,s);
      IRC_voice(IRCs,s);
      IRC_devoice (IRCs,s);
      IRC_kick(IRCs,s);
      
      IRC_action(IRCs,s);
      IRC_finger(IRCs,s);
      IRC_ping(IRCs,s);
      IRC_time(IRCs,s);
      IRC_version(IRCs,s);
      


      IRC_channel(IRCs,s);
      IRC_msg(IRCs,s);
      IRC_part(IRCs,s);

      IRC_showlist(IRCs,s);
      
      sprintf(s, "%d\n\r", n++);
      write(0, s, Strlen(s));
      sprintf(s, "%d", rand()%100);
      LList_add(IRCs->chanlist, s, NULL);
      sprintf(s, "%d", rand()%100);
      LList_remove(IRCs->chanlist, s);
    }
  }
#endif

  Client_msgf(IRCs, 0, "\r", WMSG, 0);

  PPopt_initdefaults(IRCs, IRCs->optlist);
    
  Client_drawp(IRCs);

  if (IRCs->ircserver) Client_connect(IRCs, IRCs->ircserver, IRCs->ircport);
  Client_drawp(IRCs);

  {
    pthread_t th1, th2;

    Pthread_create(&th1, NULL, getinput, (void*)&th1);
    Pthread_create(&th2, NULL, getsocket, (void*)&th2);

    Pthread_join(th1, NULL);
    Pthread_join(th2, NULL);
  }  

  finish(0);
  return TRUE;
}

void 
finish(int sig) {
  if (IRCs->sockfd) close(IRCs->sockfd);
  endwin(); 

  if (IRCs->logfile) {
    fclose(IRCs->logfile);
  }
}
