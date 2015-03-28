/*
 * mdimain.c / PulinaPussi 0.11
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
#include "resource.h"

/*Tää on ensimmäinen mun kirjoittama wintoosa-paska,
  mikä edes yrittää tehdä jotain, joten bugittomuus
  on hyvin epätodennäköistä!! :)
  
  Toivottovasti nuo threadit ei mene päällekkäin.
  Kuulemma Windowsin grafiikkakutsujen pitäisi olla
  suojattu moiselta? Petzold sanoo jotain semmoista
  vuoden 96 kirjassaan.

  Windowsin MDI-GUI:t on kyllä kivan näköisiä :)

  Tuo listbox-kontrolli pitäisi ilmeisesti korvata
  EDIT-kontrollilla, mutten vielä ole keksinyt
  kuinka siihen lisättäisiin rivejä mukavasti.

  GTK-versionkin voisi kai jotenkin portata
  wintoosalle.
*/

#define PPWINDOW_TITLE "PulinaPussi 0.10 (WIN32)"
#define MAXLINECOUNT   5000

CRITICAL_SECTION cs_ircs;

char g_szclassname[] =      "MDI vamma";
char g_szchildclassname[] = "MDI-childwindow";

char *curwin; /*Tämän tarkoituksena on estää threadien päällekkäinmenoa
		(hjoin ja MDI_ACTIVATE).*/

LLitem   *mdiwlist;
HWND     g_hmainwindow;
HWND     g_hmdiclient;

IRCSession *IRCs;

static void
socketthread(IRCSession *IRCs) {
  for (;;) {
    if (!Socket_readln(IRCs)) break;
    EnterCriticalSection(&cs_ircs);
    Debug_msgf(IRCs, IRCs->sockbuf, 0);
    
    IRC_handle(IRCs, IRCs->sockbuf);
    LeaveCriticalSection(&cs_ircs);
  }
  _endthread();
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
    return FALSE;
  }
  return TRUE;
}

static int
MDImessage(char *wname, char *_msg) {
  LLItem *mi;

  if (!wname || !_msg) return FALSE;

  mi= LList_get(mdiwlist, wname);
  if (mi && mi->data) {
    HWND hedit, mhwnd;
    int i;

    mhwnd= (HWND)mi->data;

    hedit= GetDlgItem(mhwnd, IDC_CHILD_EDIT);
    {
      char **msg;
      int n;

      msg= split(_msg, "\n", &n);
      if (msg) {
	int i;
	for (i=0;i<n;i++) {
	  SendMessage(hedit, LB_ADDSTRING, 0, (LPARAM)msg[i]);
	}
	split_free(msg, n);
      }else {
	SendMessage(hedit, LB_ADDSTRING, 0, (LPARAM)_msg);
      }
      n= SendMessage(hedit, LB_GETCOUNT, 0, (LPARAM)_msg);
      if (n>MAXLINECOUNT) {
	int i;
	for (i=0;i<n-MAXLINECOUNT;i++)
	  SendMessage(hedit, LB_DELETESTRING, 0, (LPARAM)0);
      }
    }
    SendMessage(hedit, WM_VSCROLL, SB_BOTTOM, (LPARAM)0);
  }
}

static int
MDImessageall(char *s) {
  LLItem *mdii= mdiwlist;

  if (!s) return 0;

  while (mdii) {
    mdii= (LLItem*) mdii->next;
    if (mdii && mdii->wname) MDImessage(mdii->wname, s);
  }
  return 1;
}

HWND
CreateNewMDIChild(HWND hMDIClient, char *title) {
  MDICREATESTRUCT mcs;
  HWND hchild;

  if (LList_get(mdiwlist, title)) return 0;
  LList_add(mdiwlist, title, NULL);

  mcs.szTitle = title;
  mcs.szClass = g_szchildclassname;
  mcs.hOwner  = GetModuleHandle(NULL);
  mcs.x       = mcs.cx = CW_USEDEFAULT;
  mcs.y       = mcs.cy = CW_USEDEFAULT;
  mcs.style   = MDIS_ALLCHILDSTYLES;

  hchild= (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LONG)&mcs);
  if (!hchild) {
    MessageBox(hMDIClient, "MDI Child creation failed", ":(", 
	       MB_ICONEXCLAMATION | MB_OK);
  }
  return hchild;
}

LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch(msg) {
  case WM_CREATE:
    {
      CLIENTCREATESTRUCT css;
      HMENU hmenu, hsubmenu;

      css.hWindowMenu  = GetSubMenu(GetMenu(hwnd), 2);
      css.idFirstChild = IDC_MDI_FIRSTCHILD;

      g_hmdiclient = CreateWindowEx(WS_EX_CLIENTEDGE, "mdiclient", NULL,
				    WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL |
				    WS_HSCROLL | WS_VISIBLE,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    hwnd, (HMENU)IDC_MAIN_MDI,
				    GetModuleHandle(NULL),
				    (LPVOID)&css);

      if (g_hmdiclient==NULL) MessageBox(hwnd, "Can't create MDI client!", ":(",
					 MB_OK | MB_ICONERROR);

      hmenu= CreateMenu();

      hsubmenu= CreatePopupMenu();
      AppendMenu(hsubmenu, MF_STRING, ID_FILE_EXIT, "E&xit");
      AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT)hsubmenu, "&File");

      hsubmenu= CreatePopupMenu();
      AppendMenu(hsubmenu, MF_STRING, ID_WINDOW_TILE,    "&Tile");
      AppendMenu(hsubmenu, MF_STRING, ID_WINDOW_CASCADE, "&Cascade");
      AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT) hsubmenu, "&Window");

      hsubmenu= CreatePopupMenu();
      AppendMenu(hsubmenu, MF_STRING, ID_HELP_ABOUT, "About");
      AppendMenu(hmenu, MF_STRING | MF_POPUP | MF_RIGHTJUSTIFY, (UINT) hsubmenu,
		 "&Help");

      SetMenu(hwnd, hmenu);
    }
    break;
  case WM_COMMAND:
    switch (LOWORD(wparam)) {
    case ID_HELP_ABOUT:
      MessageBox(hwnd, WMSG, "About", MB_OK);
      break;
    case ID_FILE_EXIT:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    case ID_WINDOW_TILE:
      SendMessage(g_hmdiclient, WM_MDITILE, 0, 0);
      break;
    case ID_WINDOW_CASCADE:
      SendMessage(g_hmdiclient, WM_MDICASCADE, 0, 0);
      break;
    default:
      {
	if (LOWORD(wparam) >= IDC_MDI_FIRSTCHILD) {
	  DefFrameProc(hwnd, g_hmdiclient, WM_COMMAND, wparam, lparam);
	}else {
	  HWND nchild= (HWND) SendMessage(g_hmdiclient, WM_MDIGETACTIVE, 0, 0);
	  if (nchild) {
	    SendMessage(nchild, WM_COMMAND, wparam, lparam);
	  }
	}
      }
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefFrameProc(hwnd, g_hmdiclient, msg, wparam, lparam);
  }
  return 0;
}

LRESULT CALLBACK
InputWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  int i;
  WNDPROC wndp;

  wndp= (WNDPROC) GetWindowLong(hwnd, GWL_USERDATA);

  switch(msg) {
  case WM_KEYDOWN:
    switch (wparam) {
    case 13:
      {
	char s[512];
	memset(s, 0, 512);
	if (GetWindowText(hwnd, &s, 510)){
	  EnterCriticalSection(&cs_ircs);/*Tähän?*/
	  
	  Free(IRCs->curchan);
	  if (LList_isjoined(IRCs->chanlist, curwin)) {
	    IRCs->curchan= Strdup(curwin);
	  }
	  
	  IRC_input_handle(IRCs, s);
	  LeaveCriticalSection(&cs_ircs);
	  SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)0);
	}
      }
      break;
    }
    break;
  }
  return CallWindowProc(wndp, hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK
MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_CREATE:
    {
      HFONT hfdefault;
      HWND hedit, hinput;
      char cwname[512];
      WNDPROC wndp;

      GetWindowText(hwnd, cwname, 511);
      if (!LList_setdata(mdiwlist, cwname, (void*)hwnd)) {
	MessageBox(hwnd, "MDIlist is evil!", ":(", MB_ICONEXCLAMATION | MB_OK);
      }
      
      hedit= CreateWindowEx(WS_EX_CLIENTEDGE, "listbox", "",
			    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
			    SS_LEFT | LBS_DISABLENOSCROLL | LBS_NOSEL,
			    0, 0, 100, 100, hwnd, (HMENU)IDC_CHILD_EDIT,
			    GetModuleHandle(NULL), NULL);
      if (hedit==NULL) MessageBox(hwnd, "Could not create message box.", ":(",
				  MB_OK | MB_ICONERROR);

      hinput= CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			     WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
			     0, 100, 100, 20, hwnd, (HMENU)IDC_CHILD_INPUT,
			     GetModuleHandle(NULL), NULL);
      if (hinput==NULL) MessageBox(hwnd, "Could not create input box", ":(",
				   MB_OK | MB_ICONERROR);

      wndp= (WNDPROC) SetWindowLong(hinput, GWL_WNDPROC, (long) InputWndProc);
      SetWindowLong(hinput, GWL_USERDATA, (long)wndp);

      hfdefault= GetStockObject(DEFAULT_GUI_FONT);
      SendMessage(hedit,  WM_SETFONT, (WPARAM)hfdefault, MAKELPARAM(FALSE, 0));
      SendMessage(hinput, WM_SETFONT, (WPARAM)hfdefault, MAKELPARAM(FALSE, 0));
    }
    break;
  case WM_SIZE:
    {
      HWND hedit, hinput;
      RECT rcclient;

      GetClientRect(hwnd, &rcclient);
      hedit= GetDlgItem(hwnd, IDC_CHILD_EDIT);
      hinput=GetDlgItem(hwnd, IDC_CHILD_INPUT);

      SetWindowPos(hedit, NULL, 0, 0, rcclient.right, rcclient.bottom-20,
		   SWP_NOZORDER);
      SetWindowPos(hinput, NULL, 0, rcclient.bottom-20, rcclient.right, 20,
		   SWP_NOZORDER);
    }
    return DefMDIChildProc(hwnd, msg, wparam, lparam);
    break;
  case WM_MDIACTIVATE:
    {
      char cwname[512];
      if (!GetWindowText(hwnd, cwname, 511)) {
	MessageBox(hwnd, "GetWindowText failed", ":(", MB_OK | MB_ICONERROR);
      }

      Free(curwin);
      if (cwname && (*cwname=='#' || *cwname=='&')) {
	curwin= Strdup(cwname);
      }
    }
    break;
  default:
    return DefMDIChildProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

BOOL
SetUpMDIChildWindowClass(HINSTANCE hinstance) {
  WNDCLASSEX wc;

  wc.cbSize         = sizeof(WNDCLASSEX);
  wc.style          = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc    = MDIChildWndProc;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = hinstance;
  wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = g_szchildclassname;
  wc.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc)) {
    MessageBox(0, "Could not register child window!", ":(",
	       MB_ICONEXCLAMATION | MB_OK);
    return FALSE;
  }
  return TRUE;
}

/*********/

int
Client_msgf(IRCSession *IRCs, int oper, char *to, char *s, int freeit) {
  char *pagename;

  if (!to || !s) return FALSE;

  if (*to == '[' || isalpha(*to)) {
    pagename= Strdup("[server]");
  } else {
    pagename= Strdup(to);
  }
  CreateNewMDIChild(g_hmdiclient, pagename);

  switch (oper) {
  case IRCO_MESSAGE:
    MDImessage(pagename, s);
    break;
  case IRCO_MESSAGE_ALL:
    MDImessageall(s);
    break;
  case IRCO_MESSAGE_USER_ALL:
    MDImessageall(s);
    break;
  }

  if (freeit) Free(s);
  Free(pagename);
  return TRUE;
}

int
Debug_msgf(IRCSession *IRCs, char *msg, int dofreeshit) {
  if (!msg) return 0;
  return Client_msgf(IRCs, 0, "*RAW*", msg, dofreeshit);
}

int
Client_drawp(IRCSession *IRCs) {
  return TRUE;
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

int WINAPI
WinMain(HINSTANCE hinstance, HINSTANCE hprev, PSTR argv, int argc) {
  WNDCLASSEX wc;
  HWND hwnd;
  MSG msg;

  IRCs= Calloc(1, sizeof(IRCSession));
  IRCs->chanlist=   LList_new();
  IRCs->pluginlist= LList_new();
  IRCs->optlist=    LList_new();
  curwin= NULL;

  PPopt_initdefaults(IRCs, IRCs->optlist);

  if (!startWSA()) {
    MessageBox(NULL, "Can't start WSA!", ":(", MB_ICONERROR | MB_OK);
    exit(0);
  }

  mdiwlist= LList_new();

  InitializeCriticalSection(&cs_ircs);
  InitCommonControls();

  wc.cbSize          = sizeof(WNDCLASSEX);
  wc.style           = 0;
  wc.lpfnWndProc     = WndProc;
  wc.cbClsExtra      = 0;
  wc.cbWndExtra      = 0;
  wc.hInstance       = hinstance;
  wc.hIcon           = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName    = NULL;
  wc.lpszClassName   = g_szclassname;
  wc.hIconSm         = LoadIcon(NULL, IDI_APPLICATION);
  
  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", ":(", MB_ICONERROR | MB_OK);
    return 0;
  }

  if (!SetUpMDIChildWindowClass(hinstance)) return 0;
  
  hwnd= CreateWindowEx(0, g_szclassname, PPWINDOW_TITLE,
		       WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		       CW_USEDEFAULT, CW_USEDEFAULT, 480, 320,
		       NULL, NULL, hinstance, NULL);

  if (hwnd==NULL) {
    MessageBox(NULL, "Window Creation Failed!", ":(", MB_ICONERROR | MB_OK);
    return 0;
  }
  g_hmainwindow= hwnd;

  Client_msgf(NULL, 0, "[server]", WMSG, 0);
  Client_msgf(NULL, 0, "*RAW*", "*RAW*", 0);

  ShowWindow(hwnd, argc);
  UpdateWindow(hwnd);

  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateMDISysAccel(g_hmdiclient, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return msg.wParam;
}
