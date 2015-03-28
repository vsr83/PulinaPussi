/*
 * socket.c / PulinaPussi 0.12
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

/**********************************************
 * Socket_readln()  
 * Read string(IRCs->sockbuf) from a socket.
 **********************************************/
int
Socket_readln(IRCSession *IRCs){
  int valid= 1, i= 0;
  unsigned char c;
  
  memset(IRCs->sockbuf, 0, 512);

  while (valid){
    if (recv(IRCs->sockfd, &c, 1, 0)<1) return 0;

    /*This is only paranoid security feature which should be disabled
      by default. It should make exploiting security holes more difficult
      by removing unnecessary characters from the incoming data. :)*/

    if (PPopt_getvalue(IRCs->optlist, "show_hilightown", NULL)){
      if (c>128 && c!=228 && c!=196 && c!=246 && c!=214) c='?';
    }

    if (i<511 && c !='\n' && c != '\r'){
      IRCs->sockbuf[i++]=c;
    }else {
      valid=0;
    }
  }
  IRCs->sockbuf[i]='\0';

  if (IRCs->logfile) {
    fprintf(IRCs->logfile, "<%s\n", IRCs->sockbuf);
  }

  return TRUE;
}

/**********************************************************
 * int Socket_writeln(char *buf) 
 * Write string to a socket.
 * 
 * char *buf;   The string;
 **********************************************************/
int
Socket_writeln(IRCSession *IRCs, char *buf) {
#ifndef SEGF_FIND
  if (!IRCs->connected) {
    Client_msgf(IRCs, 0, "[server]", "not connected\n", 0);
    return FALSE;
  }

  if (send(IRCs->sockfd, buf, Strlen(buf), 0) < 0){
    Client_msgf(IRCs, 0, "[ERROR]", "disconnect\n", 0);
  }

  Debug_msgf(IRCs, strdup_printf("> %s", buf),  TRUE);
  if (IRCs->logfile) {
    fprintf(IRCs->logfile, ">%s\n", buf);
  }
#endif
  return TRUE;
}

/**********************************************************
 * int Socket_writeln(char *buf) 
 * Write formatted string to a socket.
 * 
 * char *buf;   The string;
 **********************************************************/
int
Socket_outf(IRCSession *IRCs, char *s, int freeit){
  if (!IRCs || !s) return FALSE;

  Socket_writeln(IRCs, s);
  if (freeit) Free(s);
  return TRUE;
}

/**********************************************************
 * int Socket_create(char *host, int port) 
 * Create socket and connection to the server.
 *
 * char *host;   Hostname;
 * int   port;   Port;
 *
 * Returns descriptor referencing the socket.
 **********************************************************/
static int
Socket_create(char *host, int port) {
  struct sockaddr_in sa;
  struct hostent    *he;
  int s;


  he= gethostbyname(host);
  if (he==NULL){
    //win32
    //errno= ECONNREFUSED;
    return -1;
  }

  bzero(&sa, sizeof(sa));
  bcopy(he->h_addr, (char*)&sa.sin_addr, he->h_length);
  sa.sin_family= he->h_addrtype;
  sa.sin_port  = htons((short)port);

  s= socket(he->h_addrtype, SOCK_STREAM, 0);
  if (s<0) return -1;

  if (connect(s, (struct sockaddr *) &sa, sizeof(sa))<0){
    close(s);
    return -1;
  }

  return s;
}

/**********************************************************
 * int Client_connect(char *host, int port) 
 * Login to a IRC-server.
 *
 * char *host;   Hostname;
 * int   port;   Port;
 **********************************************************/
int
Client_connect(IRCSession *IRCs, char *host, int port) {
  char localhost[64];
  gethostname(localhost, 64);

  if (!IRCs->ircnick) {
    Client_msgf(IRCs, 0, "[server]", "no nickname", 0);
    return FALSE;
  }
  if (!IRCs->ircrealname) {
    IRCs->ircrealname= Strdup("abc");
  }

  Client_msgf(IRCs, 0, "[socket]", 
	      strdup_printf("*** trying port %d of %s\n", port, host), TRUE);

  IRCs->sockfd= Socket_create(host, port);
  if (IRCs->sockfd == -1) {
    Client_msgf(IRCs, 0, "[socket]", 
		strdup_printf("*** can't connect to %s:%d\n", host, port), 
		TRUE);
    //    err_exit("can't connect to server\n", 0);
    return FALSE;
  }
  IRCs->connected= TRUE;

  Socket_outf(IRCs, strdup_printf("NICK %s\n", IRCs->ircnick), TRUE);
  Socket_outf(IRCs, strdup_printf("USER %s %s %s :%s\n", IRCs->ircnick, 
				  localhost, host, IRCs->ircrealname), TRUE);

  return TRUE;
}
