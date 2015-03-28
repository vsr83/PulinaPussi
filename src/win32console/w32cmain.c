/*
 * w32cmain.c / PulinaPussi 0.11
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

#define INPUT_MAXSIZE 448

CONSOLE_SCREEN_BUFFER_INFO binfo;
CRITICAL_SECTION cs; /*should work like mutex?*/
HANDLE hstdin, hout; /*I/O-buffers*/
IRCSession *IRCs;
char inputs[512];
int  inputp;

#define CONSOLE_DEFAULTATTR FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

void
socketthread(IRCSession *IRCs) {
  for (;;) {
    Socket_readln(IRCs);
    EnterCriticalSection(&cs);
    Debug_msgf(IRCs, IRCs->sockbuf, 0);
    Debug_msgf(IRCs, "\n", 0);

    IRC_handle(IRCs, IRCs->sockbuf);
    LeaveCriticalSection(&cs);
  }
}

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

    if (IRCs->connected) {
      _beginthread(socketthread, 0, IRCs);
    }
    return TRUE;
  }
  return FALSE;
}

static int 
console_init() {
  hout= CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL,
				  CONSOLE_TEXTMODE_BUFFER, NULL);
  if (hout==INVALID_HANDLE_VALUE)                return 0;
  if (!SetConsoleActiveScreenBuffer(hout))       return 0;
  if (!SetConsoleMode(hout, ENABLE_LINE_INPUT))  return 0;

  hstdin= GetStdHandle(STD_INPUT_HANDLE);
  if (hstdin==INVALID_HANDLE_VALUE)              return 0;
  if (!GetConsoleScreenBufferInfo(hout, &binfo)) return 0;
  return 1;
}

static int
startWSA() {
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  wVersionRequested= MAKEWORD(1, 1);
  err= WSAStartup(wVersionRequested, &wsaData);
  if (err) return 0;
  if (LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1) {
    return 0;
  }
  return 1;
}

static void
wsetinputs(HANDLE outh, char *s) {
  COORD curc;
  long n;

  if (!GetConsoleScreenBufferInfo(outh, &binfo)) exit(0);
  SetConsoleTextAttribute(outh, CONSOLE_DEFAULTATTR | BACKGROUND_BLUE |
			  FOREGROUND_INTENSITY);

  curc.X= 0;
  curc.Y= binfo.dwSize.Y-2;
  SetConsoleCursorPosition(outh, curc);
  if (Strlen(s) > binfo.dwSize.X) {
    int a = Strlen(s) - binfo.dwSize.X;
    WriteConsole(outh, s+a, Strlen(s)-a, &n, NULL);
  } else {
    int i;
    char cs[512];

    memset(cs, ' ', 511);
    
    WriteConsole(outh, s, Strlen(s), &n, NULL);
    WriteConsole(outh, cs, binfo.dwSize.X-Strlen(s), &n, NULL);
  }
  SetConsoleCursorPosition(outh, binfo.dwCursorPosition);
  SetConsoleTextAttribute(outh, CONSOLE_DEFAULTATTR);
}

/*ugly*/
static void
Wouts(HANDLE outh, char *s) {
  long n, i, cc;

  for (i=0;i<Strlen(s);i++) {
    char c= s[i];
    if (c!='\n') {
      WriteConsole(outh, &c, 1, &n, NULL);
    }
    if (!GetConsoleScreenBufferInfo(outh, &binfo)) exit(0);
    if (c=='\n' || binfo.dwCursorPosition.X >= binfo.dwSize.X-2) {
      if (binfo.dwCursorPosition.Y < binfo.dwSize.Y-4) {
	COORD curc;
	
	curc.X= 0;
	curc.Y= binfo.dwCursorPosition.Y+1;
	SetConsoleCursorPosition(outh, curc);
      }else {
	SMALL_RECT scrr, clipr;
	COORD dest, curc;
	CHAR_INFO cfill;

	scrr.Left=  0;
	scrr.Top =  1;
	scrr.Right= binfo.dwSize.X;
	scrr.Bottom=binfo.dwSize.Y-3;
	memcpy(&clipr, &scrr, sizeof(SMALL_RECT));
	clipr.Top=0;

	dest.X=0; dest.Y=0;
	cfill.Char.AsciiChar= ' ';
	cfill.Attributes= 0;

	ScrollConsoleScreenBuffer(outh, &scrr, &clipr, dest, &cfill);
	curc.X= 0;
	curc.Y= binfo.dwSize.Y-4;
	SetConsoleCursorPosition(outh, curc);
      }
    }
  }
}

static int
input_loop() {
  INPUT_RECORD lpbuf;
  
  inputp= 0;
  memset(inputs, 0, 512);

  for (;;) {
    long ievents;

    if (!ReadConsoleInput(hstdin, &lpbuf, 1, &ievents)) exit(0);
    EnterCriticalSection(&cs);
    switch(lpbuf.EventType) {
    case KEY_EVENT:
      {
	unsigned char keyval = lpbuf.Event.KeyEvent.uChar.AsciiChar; /*:O*/
	BOOL          keydown= lpbuf.Event.KeyEvent.bKeyDown;

	if (keydown && keyval) {
	  char buf[20];

	  if (keyval == 8 && inputp>0) {/*BackSpace*/
	    if (inputp>0) {
	      inputs[--inputp]= 0;
	    }
	  } else if (keyval==13) { /*Enter*/
	    inputs[inputp]= 0;
	    
	    if (inputp) IRC_input_handle(IRCs, inputs);
	    inputs[0]=0;
	    inputp=0;
	  } else if (inputp<INPUT_MAXSIZE) {
	    inputp++;
	    inputs[inputp-1]= keyval;
	    inputs[inputp]=0;
	  }
	  wsetinputs(hout, inputs);
	}
	break;
      }
    }
    LeaveCriticalSection(&cs);
  }
}

/******/
int
Client_msgf(IRCSession *IRCs, int oper, char *_to, char *s, int freeit) {
  char *to=NULL;

  if (!_to || !s) return FALSE;
  if (oper>2) {
    if (freeit) Free(s);
    return 1;
  }
  to= Strdup(_to);
  if (!to) return FALSE;

  if (to) {
    int i;
    for (i=0;i<Strlen(to);i++) {
      to[i]= toupper(to[i]);
    }
  }
  SetConsoleTextAttribute(hout, CONSOLE_DEFAULTATTR | FOREGROUND_INTENSITY);
  Wouts(hout, to);
  Wouts(hout, " ");
  SetConsoleTextAttribute(hout, CONSOLE_DEFAULTATTR);
  Wouts(hout, s);
  Free(to);
  if (freeit) Free(s);
  return TRUE;
}

int
Debug_msgf(IRCSession *IRCs, char *s, int freeit) {
#ifdef DEBUG
  return Client_msgf(IRCs, 0, "[DEBUG]", s, freeit);
#else
  if (freeit) Free(s);
  return 1;
#endif
}

int
Client_drawp(IRCSession *IRCs) {
  wsetinputs(hout, inputs);
  return TRUE;
}

/************/
static void
show_usage(char *argv0) {
  printf("Usage: %s -s [server:port] -n [nick] -r [realname] -l [logfile]\n",
	 argv0);
}

int
main(int argc, char **argv) {
  int n;

  IRCs= Calloc(1, sizeof(IRCSession));
  IRCs->chanlist=  LList_new();
  IRCs->pluginlist=LList_new();
  IRCs->optlist=   LList_new();
  PPopt_initdefaults(IRCs, IRCs->optlist);

  if (argc>=2 && (!Strcmp(argv[1], "-help") || !Strcmp(argv[1], "--help"))) {
    show_usage(argv[0]);
    exit(0);
  }

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
	IRCs->logfilename= Strdup(argv[n+1]);
	break;
      }
    }
  }
  if (!IRCs->ircrealname) IRCs->ircrealname= Strdup("abc");

  if (!startWSA())     err_exit("Cannot start WSA\n", 0);
  if (!console_init()) err_exit("Cannot open Console", 0);

  InitializeCriticalSection(&cs);
  SetConsoleTitle("PulinaPussi 0.10");
  {
    char buf[512];
    memset(buf, 0, 512);
    snprintf(buf, 512, "%s\n", WMSG);
    Client_msgf(IRCs, 0, "[server]", buf, 0);
  }
  input_loop(); /*Replace with thread?*/
}
