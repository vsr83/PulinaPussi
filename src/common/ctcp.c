/*
 * ctcp.c / PulinaPussi 0.12
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

#ifndef PP_DISABLE_CTCP

#define CTCPPARAMS (IRCSession *, char *, char **, int)

void CTCP_dcc     CTCPPARAMS;
void CTCP_finger  CTCPPARAMS;
void CTCP_ping    CTCPPARAMS;
void CTCP_time    CTCPPARAMS;
void CTCP_version CTCPPARAMS;

struct CTCPhandler{
  char *cmd;
  void (*cb) (IRCSession*, char *rnick, char **sp, int n);
} CTCPhandlers[] = {
  "DCC",    CTCP_dcc,
  "FINGER", CTCP_finger,
  "PING",   CTCP_ping,
  "TIME",   CTCP_time,
  "VERSION",CTCP_version
};
int nCTCPhandlers= sizeof(CTCPhandlers)/sizeof(struct CTCPhandler);

void
CTCP_dcc(IRCSession *IRCs, char *rnick, char **sp, int n) {
#ifndef PP_DISABLE_DCC
  char *buf=NULL, *addrs=NULL;
  unsigned char addrc[4];
  long addri, dport;

  if (!PPopt_getvalue(IRCs->optlist, "DCC", NULL)) return;

  if (!rnick || !sp || n<5) return;
  if (!isdigit(*sp[3]) || !isdigit(*sp[4])) return;

  addri= atoi(sp[3]);
  memcpy(addrc, &addri, sizeof(long));

  addrs= strdup_printf("%d.%d.%d.%d", addrc[3],addrc[2],addrc[1],addrc[0]);
  dport= atoi(sp[4]);

  if (!addrs) return;

  //sp[3]=addr 4=port 5 size

  if (!Strcmp(sp[1], "CHAT")) {
    Client_msgf(IRCs, 0, "[server]", 
		strdup_printf("DCC CHAT from [%s@%s:%d]\n", 
			      rnick, addrs, dport), TRUE);
    
  }else if (!Strcmp(sp[1], "SEND") && n>=6) {
    long fsize;
    fsize= atoi(sp[5]);
    Client_msgf(IRCs, 0, "[server]", 
		strdup_printf("DCC SEND %s(%d bytes) from [%s@%s:%d]\n", 
			      sp[2], fsize, rnick, addrs, dport), TRUE);
  }
  Free(addrs);
#endif
}

void
CTCP_finger(IRCSession *IRCs, char *rnick, char **sp, int n) {
  char *buf=NULL;
  char localhost[64];

  if (!rnick || !sp || !n || !sp[0]) return;

  if (gethostname(localhost, 64)<0) return;
  
  buf=(char*)strdup_printf("NOTICE %s :\01FINGER %s (%s@%s)\01\n", 
			   rnick, IRCs->ircrealname, IRCs->ircnick, localhost);
  Socket_writeln(IRCs, buf);
  Free(buf);
}

void
CTCP_ping(IRCSession *IRCs, char *rnick, char **sp, int n) {
  char *buf=NULL;
  if (!rnick || !sp || !n || !sp[0]) return;

  if (n==2 && sp[1]){
    buf= (char*)strdup_printf("NOTICE %s :\01PING %s\01\n", rnick, sp[1]);
  } else {
    buf= (char*)strdup_printf("NOTICE %s :\01PING\01\n", rnick);
  }	  
  Socket_writeln(IRCs, buf);
  Free(buf);
}

void
CTCP_time(IRCSession *IRCs, char *rnick, char **sp, int n) {
  char *buf=NULL, *s=NULL;
  time_t tt;
  
  if (!rnick || !sp || !n || !sp[0]) return;

  time(&tt);
  
  s= Strdup(ctime(&tt));
  if (Strlen(s)>2 && s[Strlen(s)-1]=='\n') s[Strlen(s)-1]=1;
  buf= (char*)strdup_printf("NOTICE %s :\01TIME %s\n", rnick, s);
  Free(s);
  
  Socket_writeln(IRCs, buf);
  Free(buf);
}

void
CTCP_version(IRCSession *IRCs, char *rnick, char **sp, int n) {
  char *buf=NULL;
  if (!rnick || !sp || !n || !sp[0]) return;

  buf= (char*)strdup_printf("NOTICE %s :\01VERSION %s\01\n", rnick, VERSION);
  Socket_writeln(IRCs, buf);
  Free(buf);
}

int
CTCP_handle(IRCSession *IRCs, char *rnick, char *rcnt) {
  char **sp=NULL;
  int  n= 0, i;
 
  Client_msgf(IRCs, 0, "[server]", 
	      strdup_printf("Received a CTCP %s from %s\n", rcnt, rnick), TRUE);  
  sp= (char**)split(rcnt, ": \01", &n);
  if (!sp || n<1 || !sp[0]) return FALSE;
  
  for (i=0;i<nCTCPhandlers;i++) {
    if (!Strcmp(sp[0],CTCPhandlers[i].cmd)) {
      CTCPhandlers[i].cb(IRCs, rnick, sp, n);
    }
  }

  split_free(sp, n);
}

#endif
