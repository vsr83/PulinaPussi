/*
 * fin.c / PulinaPussi 0.12
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

/*
 sockbuf -> IRC_handle() +--> handle_numeric()
                         +--> h**()
*/

#include "../pp.h"

struct handler{
  char *cmd;
  void (*cb) (IRCSession*, char *, char *, char *);
} handlers[] = {
  "TOPIC",    htopic,
  "INVITE",   hinvite,
  "NOTICE",   hnotice,
  "PART",     hpart,
  "PRIVMSG",  hprivmsg,
  "JOIN",     hjoin,
  "KICK",     hkick,
  "MODE",     hmode,
  "QUIT",     hquit,
  "NICK",     hnick
};

int handlerc= sizeof(handlers) / sizeof(struct handler);

/**********************************************************
 * hquit(from, reason, reason2)
 * 
 * char *from;    [COMPLETE] of the user who is quitting.
 * char *reason;  ':' + First word of the reason.
 * char *reason2; The rest.
 **********************************************************/
void
hquit(IRCSession *IRCs, char *from, char *reason, char *reason2) {
  char *rnick =NULL; /*[NICK]*/
  char *nicks =NULL; /*[NICK]*/

  if (!from) return;

  rnick= getbc(from, '!');
  if (rnick) nicks= Strdup(rnick); 
  else nicks= Strdup(from);

  if (!reason) reason= nicks;

  if (!reason2) {
    Client_msgf(IRCs, IRCO_MESSAGE_USER_ALL, nicks, strdup_printf("%s has quit (%s)\n", nicks, 
					  reason+DELD(reason)), TRUE);
  }else {
    Client_msgf(IRCs, IRCO_MESSAGE_USER_ALL, nicks, strdup_printf("%s has quit (%s %s)\n", nicks, 
					  reason+DELD(reason), reason2), TRUE);
  }

  Client_msgf(IRCs, IRCO_USERLISTS_REMOVE, "[", nicks, FALSE);
  Free(rnick);
  Free(nicks);
}

/***************************************************************
 * hinvite(from, to, channel) 
 *
 * char *from;    [COMPLETE] of the user who is inviting you.
 * char *to;
 * char *channel; [CHANNEL] you are being invited to.
 ***************************************************************/
void
hinvite(IRCSession *IRCs, char *from, char *to, char *channel) {
  if (!from || !channel) return;
  Client_msgf(IRCs, 0, "[server]", strdup_printf("%s invites you to %s\n", 
						 from, channel), TRUE);
}

/****************************************************************
 * hkick(from, channel, cnt)
 * 
 * char *from;    The operator [COMPLETE]
 * char *channel; [CHANNEL] the user is being kicked out from.
 * char *cnt;     [NICK]+'!'+REASON
 ****************************************************************/

void
hkick(IRCSession *IRCs, char *from, char *channel, char *cnt) {
  char *coperator=NULL; /*The operator who is kicking somebody out. [NICK]*/
  char *idiot   =NULL; /*The idiot who is being kicked out. [NICK]*/

  if (!channel || !from || !cnt) return;

  idiot= (char *)getbc(cnt, ':');
  
  if (idiot){
    coperator= (char *)getbc(from, '!');

    if (Strlen(idiot)>1) {
      idiot[Strlen(idiot)-1]= 0;
    }

    /*It's you who is kicked ???*/    
    if (idiot && IRCs->ircnick && Strcmp(idiot, IRCs->ircnick)==0) {
      if (IRCs->curchan && Strcmp(IRCs->curchan, channel)==0) {
	Free(IRCs->curchan);
      }
      LList_remove(IRCs->chanlist, channel);
      Client_drawp(IRCs);
    }

    if (coperator){
      Client_msgf(IRCs, 0, channel, strdup_printf("%s kicked from %s by %s\n", 
      				      idiot, channel, coperator), TRUE);
      Client_msgf(IRCs, IRCO_USERLIST_REMOVE, channel, idiot, FALSE);
      Free(coperator);
    }
    Free(idiot);
  }
}

/************************************************************
 * hnick(from, newnick, cnt)
 *
 * char *from;    [COMPLETE] of the user who is chaning
 *                  his/her/whatever nickname.
 * char *newnick; :[NICK]
 * char *cnt;
 ************************************************************/

void
hnick (IRCSession *IRCs, char *from, char *newnick, char *cnt) {
  char *oldnick= NULL; /*Old nickname.*/
  if (!newnick || !from) return;
  if (Strlen(newnick)<2) return;

  oldnick= getbc(from, '!');

  if (oldnick && IRCs->ircnick) {
    if (Strcmp(oldnick, IRCs->ircnick)==0){
      Free(IRCs->ircnick);
      IRCs->ircnick= Strdup(newnick+DELD(newnick));
      Client_drawp(IRCs);
      Client_msgf(IRCs, IRCO_MESSAGE_ALL, "[server]", 
		  strdup_printf("you are now known as %s\n", 
				newnick+DELD(newnick)), TRUE);
      Client_msgf(IRCs, IRCO_NICKNAME_UPDATE, "[server]", "raivo", 0);
    }else {
      Client_msgf(IRCs, IRCO_MESSAGE_USER_ALL, oldnick, 
		  strdup_printf("%s is now known as %s\n", oldnick, 
				newnick+DELD(newnick)), TRUE);
    }
    
    Client_msgf(IRCs, IRCO_USERLISTS_NICK, oldnick, newnick+DELD(newnick), 
		FALSE);
  }
  Free(oldnick);
}

/******************************************************
 * hjoin(from, _channel, cnt) 
 * 
 * char *from;    [COMPLETE] of the user who is joining.
 * char *channel; :[CHANNEL] the user is joining to.
 * char *cnt;
 ******************************************************/

void
hjoin(IRCSession *IRCs, char *from, char *_channel, char *cnt) {
  char *rnick   =NULL; /*[NICK]*/
  char *channel =NULL; /*[CHANNEL]*/

  if (!_channel || !from) return;
  if (Strlen(_channel)<(1+DELD(_channel))) return;

  channel  = Strdup(_channel   + DELD(_channel));
  rnick= (char *)getbc(from, '!');
 
  if (rnick && IRCs->ircnick && Strcmp(rnick, IRCs->ircnick)==0){
    int vtype, maxchan;

    Free(IRCs->curchan);
    IRCs->curchan= Strdup(channel);

    maxchan= (int)PPopt_getvalue(IRCs->optlist, "max_channels", &vtype);
    if (vtype!=OPTTYPE_LONG) maxchan= 0;

    LList_add(IRCs->chanlist, channel, NULL);

    /*Palvelin voisi esimerkiksi "huijata" ohjelman varaamaan tilaa miljoonalle
      400-merkkiselle kananvan nimelle.*/
    if (LList_length(IRCs->chanlist) >= maxchan){
      Client_msgf(IRCs, 0, "[ERROR]", "Too many channels\n", FALSE);
      IRC_part(IRCs, channel);
    }

    Client_drawp(IRCs);
  }

  if (rnick){
    Client_msgf(IRCs, 0, channel, strdup_printf("%s (%s) JOINED to %s\n", 
						rnick, from, channel), TRUE);

    Client_msgf(IRCs, IRCO_USERLIST_ADD, channel, rnick, FALSE);
    Free(rnick);
  }

  Free(channel);
}

void
hmode (IRCSession *IRCs, char *from, char *to, char *cnt) {
  if (!cnt || !to || !from) return;
  if (Strlen(cnt)<2) return;

  if (to[0]=='#' || to[0]=='&'){
    char *rnick=NULL, *mode=NULL, *frnick=NULL;

    frnick= (char *)getbc(from, '!');
    mode  = (char *)getbc(cnt,  32);

    if (mode && Strlen(cnt)<=3 && frnick){
      Client_msgf(IRCs, 0, to, strdup_printf("%s sets %s %s\n", frnick, mode, 
					     to), TRUE);
    }
    else if (mode && Strlen(cnt)>3 && cnt[2]==' ' && cnt[3]){
      //      rnick= Strdup(Strchr(cnt, 32)+1);    
      rnick= Strdup(cnt + 3);

      if (!frnick) {
	Client_msgf(IRCs, 0, to ,strdup_printf("%s sets %s %s %s\n",
					  from, to, mode, rnick), TRUE);
      } else {
	Client_msgf(IRCs, 0, to , strdup_printf("%s sets %s %s %s\n",
					  frnick, to, mode, rnick), TRUE);

	Client_msgf(IRCs, IRCO_USERLIST_SETMODE, to, 
		    strdup_printf("%s:%s", mode, rnick), TRUE);
      }
    }
    Free(frnick);
    Free(rnick);
    Free(mode);
  }
  
  if (Strcmp(IRCs->ircnick, to)==0){
    if (!Strcmp(IRCs->ircnick, from) && Strlen(cnt)>1){
      Client_msgf(IRCs, 0, "[server]", 
		  strdup_printf("%s sets %s %s\n", IRCs->ircserver,
				cnt+DELD(cnt), IRCs->ircnick), TRUE);
    } 
    /*S[] F[spuuki] C[MODE] T[spuuki] C[:+i]*/
  }
}

/*****************************************************************
 * hnotice(from, to, cnt)
 *
 * char *from;     [COMPLETE] of the user who is sending the notice.
 * char *to;       [CHANNEL]/[USER] the notice is being sent to.
 * char *_message; ':' + [MESSAGE]
 ******************************************************************/

void
hnotice (IRCSession *IRCs, char *from, char *to, char *_message) {
  char *message=NULL; /*[MESSAGE]*/

  if (!from || !to || !_message || !*_message) return;
  if (DELD(_message) && Strlen(_message)<2) return;

  if (_message) message= Strdup(_message+DELD(_message));

  if (message[0]!='\01'){
    char *rnick=NULL;
    
    rnick= (char *)getbc(from, '!');

    if (rnick){
      if (to[0]=='#' || to[0]=='&'){
	Client_msgf(IRCs, 0, to, strdup_printf("-%s- %s\n", rnick, message), 
		    TRUE);
      } else {
	char *s= strdup_printf("-%s", rnick);
	Client_msgf(IRCs, 0, s, strdup_printf("-%s- %s\n", rnick,message), 
		    TRUE);
	Free(s);
      }
      Free(rnick);
    }
  } else {
    char *rnick=NULL;

    rnick= (char *)getbc(from, '!');

    if (rnick){
      Client_msgf(IRCs, 0, "[CTCP]", 
		  strdup_printf("-%s- %s\n", rnick, message), TRUE);
      Free(rnick);
    }else{
      Client_msgf(IRCs, 0, "[server]", 
		  strdup_printf("-%s- %s\n", from, message), TRUE); 
    }
  }
  Free(message);
}

/*******************************************************
 * hpart(from, channel, nick)
 *
 * char *from;     [COMPLETE] of the user who is leaving.
 * char *channel;  [CHANNEL] the user is leaving from.
 * char *reason;    ':'+[REASON].
 *******************************************************/
void
hpart (IRCSession *IRCs, char *from, char *channel, char *reason) {
  char *nick; /*[NICK]*/
  if (!channel || !from || !reason) return;

  nick= (char *)getbc(from, '!');
  if (!nick) return;

  if (IRCs->ircnick && Strcmp(nick, IRCs->ircnick)==0){
    if (IRCs->curchan && !Strcmp(IRCs->curchan, channel)) Free(IRCs->curchan);

    if (!LList_remove(IRCs->chanlist, channel)){
      Client_msgf(IRCs, 0, "ERROR", "chanlist SUCKS\n", FALSE);
    }
    Client_drawp(IRCs);
  }

  if (channel[0]=='#' || channel[0]=='&') {
    Client_msgf(IRCs, 0, channel, strdup_printf("%s (%s) left from %s (%s)\n", nick, from, channel, reason), TRUE);
    Client_msgf(IRCs, IRCO_USERLIST_REMOVE, channel, nick, FALSE);
  }
  Free(nick);
}


/*****************************************************************
 * hprivmsg(from, to, cnt)
 *
 * char *from;  [COMPLETE] of the user who is sending the message.
 * char *to;    [USER]/[CHANNEL] the message is being sent to.
 * char *cnt;   ':' + the message.
 *****************************************************************/

void
hprivmsg (IRCSession *IRCs, char *from, char *to, char *cnt) {
  char *rcnt=NULL;

  if (!from || !to || !cnt) return;

  if (cnt) rcnt= Strdup(cnt);
  if (!rcnt) return;

  /*CTCP, DCC..*/
  if (rcnt[0]=='\01'){

    char *rnick=NULL;
    rnick= (char *)getbc(from, '!');

    if (rnick){
      if (strncmp(rcnt, "\01ACTION\01", 7)==0) {
	char *acs=NULL;
	
	if (Strchr(rcnt, ' ')!=NULL){
	  acs= Strdup(Strchr(rcnt, ' ')); /* +1 here would be "ugly" :O */
	                                 /* strdup(NULL) crashes the program*/
	  if (Strlen(acs)>1){
	    Free(acs);
	    acs= Strdup(Strchr(rcnt, ' ')+1);
	  }
	  
	  if (acs){
	    if (to && (to[0]=='#' || to[0]=='&')){
	      Client_msgf(IRCs, 0, to, strdup_printf("*%s %s\n", rnick, acs),
			  TRUE);
	    }else {
	      char *s= strdup_printf("-%s", rnick);
	      Client_msgf(IRCs, 0, s, strdup_printf("*%s %s\n", rnick, acs),
	      		  TRUE);
	      Free(s);
	    }
	    Free(acs);
	  }
	} 
      }
      else {/*CTCP*/
#ifndef PP_DISABLE_CTCP
	if (PPopt_getvalue(IRCs->optlist, "CTCP", NULL)) {
	  CTCP_handle(IRCs, rnick, rcnt);
	}
#endif
      }

      Free(rnick);
    }  
  }
  
  if (rcnt[0]!='\01'){
    char *rnick;
    
    rnick= (char *)getbc(from, '!');

    if (rnick){
      if (to[0]=='#' || to[0]=='&'){	
	Client_msgf(IRCs, 0, to, strdup_printf("<%s> %s\n", rnick, rcnt), 
		    TRUE);
      } else {
	char *s= strdup_printf("-%s", rnick);

	Client_msgf(IRCs, 0, s, strdup_printf("<%s> %s\n", rnick, rcnt),TRUE);
	Free(s);
      }
      Free(rnick);
    }
  }
  Free(rcnt);
}

/*******************************************************
 * htopic(from, channel, cnt)
 * 
 * char *from;     [COMPLETE] of the user who is changing
 *                 the topic.
 * char *channel;
 * char *topic;
 *******************************************************/

void
htopic (IRCSession *IRCs, char *from, char *channel, char *topic) {
  char *rnick=NULL; /*[NICK]*/
  if (!from || !channel) return;

  rnick= (char *)getbc(from, '!');
  if (rnick){
    Client_msgf(IRCs, 0, channel, 
		strdup_printf("%s has changed the topic to :%s\n", 
			      rnick, topic), TRUE);

    if (topic) {
      Client_msgf(IRCs, IRCO_SETTOPIC, channel, topic, FALSE);
    } else {
      Client_msgf(IRCs, IRCO_SETTOPIC, channel, " ", FALSE);
    }
    Free(rnick);
  }
}

int
handle_numeric (IRCSession *IRCs, char *from, char *cmd, char *to, char *cnt) {

  if (!cmd) return FALSE;

  /*If given nickname differs from the nickname given by the server
    at startup, correct it automatically.*/
  if (atoi(cmd)==1 && to && IRCs->ircnick) {
    if (Strcmp(IRCs->ircnick, to)!=0) {
      Free(IRCs->ircnick);
      IRCs->ircnick= Strdup(to);
      Client_drawp(IRCs);
      Client_msgf(IRCs, IRCO_NICKNAME_UPDATE, "[server]", "raivo", 0);
    }
  }

  if (atoi(cmd)>=400) {
    Client_msgf(IRCs, 0, "[ERROR]", strdup_printf("%s\n", cnt), TRUE);
  }else {
    Client_msgf(IRCs, 0, "[SERVER]", strdup_printf("%s\n", cnt), TRUE);
  }

/*
:irc1.inet.fi 353 spuuki2 = #chatmaailma :spuuki2 @Varaani Apri` RadLiz 
S[] F[irc1.inet.fi] C[353] T[spuuki2] C[= #chatmaailma :spuuki2 @Varaani Apri` RadLiz ]
*/
  if (atoi(cmd)==RPL_TOPIC && cnt && Strchr(cnt, ':') && Strchr(cnt, ' ')) {
    char *topic=NULL, *channel=NULL, *s=NULL;
    channel= getbc(cnt, ' ');
    s= Strchr(cnt, ':');
    if (s && *s && *(s+1)) topic= Strdup(s+1);

    if (channel && topic) Client_msgf(IRCs, IRCO_SETTOPIC, channel, topic, FALSE);
    Free(channel);
    Free(topic);
  }

  /*USERLIST(353)*/
  if (atoi(cmd)==RPL_NAMREPLY && cnt && Strchr(cnt, ':')) { 
    char *users;
    char *channel;

    channel= getbc(Strchr(cnt, '#'), ' ');
    if (!channel) channel= getbc(Strchr(cnt, '&'), ' ');

    Client_msgf(IRCs, 0, "[", channel, 0);
    
    if (channel) {

      users= Strchr(cnt, ':')+1;

      if (users) {
	char **userlist;
	int nusers;
	userlist= split(users, " ", &nusers);
	
	if (userlist){
	  int i;

	  for (i=0;i<nusers;i++){
	    Client_msgf(IRCs, IRCO_USERLIST_ADD, channel, userlist[i], FALSE);
	  }
	  split_free(userlist, nusers);
	}
      }
      Free(channel);
    }
  }
  /*EUSERLIST*/

  return TRUE;
}

/********************************************************
 * IRC_handle(_s)
 *
 * Handle raw input readen from the socket.
 * 
 * char _s;  String readen from the socket.
 *******************************************************/
int
IRC_handle(IRCSession *IRCs, char *_s) {
  char *srv=NULL, *from=NULL, *cmd=NULL, *to=NULL, *cnt=NULL;
  int p[MAXLEN], np;
  int i, a;

  char *s= (char*)Strdup(_s);
  for (i=0;i<Strlen(s);i++)if (s[i]=='\n')s[i]=0;

  if (Strlen(s)<2) {Free(s);return FALSE;}
  if (Strlen(s)>=SMAXLEN) {Free(s);return FALSE;}

  for (a=0;a<Strlen(s) && s[a]!=':';a++);
  if (a>=Strlen(s)-1){Free(s);return FALSE;}

  np= 0;

  for (i=a;i<Strlen(s);i++) {
    if (s[i]==' '){
      p[np++]=i;
    }
  }

  p[np]=Strlen(s);
  /*PARSE FROM*/
  from= (char*)Calloc(1, p[0]-a+2);
  memcpy(from, s+a+1, p[0]-a-1);
  from[p[0]-a]= 0;

  /*PARSE CMD*/
  if (np>0){
    cmd= (char*)Calloc(1, p[1]-p[0]+2);
    memcpy(cmd, s+p[0]+1, p[1]-p[0]-1);
    cmd[p[1]-p[0]]= 0;
  }
  /*PARSE TO*/
  if (np>1){
    to= (char*)Calloc(1, p[2]-p[1]+2);
    memcpy(to, s+p[1]+1, p[2]-p[1]-1);
    to[p[2]-p[1]]= 0;
  }
  /*PARSE CNT*/
  if (np>2){
    cnt= (char*)Calloc(1, Strlen(s)-p[2]+3);
    memcpy(cnt, s+p[2]+1, Strlen(s)-p[2]-1);
    cnt[Strlen(s)-p[2]]= 0;
  }

  if (a>0) {
    srv= (char*)Calloc(1, a+2);
    memcpy(srv, s, a);
  }
  //printf("s%d a%d np%d\n", Strlen(s), a, np);

  //  Debug_msgf(IRCs, strdup_printf("S[%s] F[%s] C[%s] T[%s] C[%s]\n",
  //			   srv, from, cmd, to, cnt), 1);
  
  {/*Handlers*/
    int f=0;
    for (i=0;i<handlerc;i++) {
      if (cmd && Strcmp(cmd+DELD(cmd), handlers[i].cmd)==0){
	handlers[i].cb(IRCs, from, to, cnt+DELD(cnt));
	f=1;
      }
    }

    /*plugin PPHOOKTYPE_FIN -- This is scary!*/
    {
      LLItem *pi= IRCs->pluginlist;
      
      while (pi) {
	if (pi->data) {
	  PPpluginfile *pfile;
	  LLItem *li;
	  
	  pfile= (PPpluginfile*)pi->data;
	  li= (LLItem*)LList_get(pfile->funclist, cmd);

	  //      if (!li)Debug_msgf(IRCs, strdup_printf("%s not found\n", cmd), TRUE);
	  if (li){
	    PPpluginhook *phook;
	    phook= (PPpluginhook*) li->data;
	    
	    //	    fprintf(stderr, "%s %d %d\n", phook->funcname, phook->hooktype, PPHOOKTYPE_FIN);


	    if (phook->hooktype == PPHOOKTYPE_FIN && phook->cb) {
	      ((void(*)(IRCSession*,char*,char*,char*))phook->cb)(IRCs, from, to, cnt);
	      f=1;
	    }
	  }
	}         
	
	pi= (LLItem*) pi->next; 
      }
    }
    /*eplugin*/
    
    if (f==0 && !srv && cmd){
      if (isdigit(cmd[0])){
	handle_numeric(IRCs, from, cmd, to, cnt+DELD(cnt));
      }
    }

  }

  if (srv){
    if (strncmp(srv,"PING",4)==0){
      char *buf;
      buf= (char*)strdup_printf("PONG %s\n", from);
      Socket_writeln(IRCs, buf);
      Free(buf);
    }
    if (strncmp(srv,"ERROR",5)==0) {
      Client_msgf(IRCs, 0, "SERVER_ERROR", s, FALSE);
      err_exit("SERVER ERROR", 0);
    }
  }

  Free(srv);
  Free(from);
  Free(cmd);
  Free(to);
  Free(cnt);
  Free(s);

  return TRUE;
}
