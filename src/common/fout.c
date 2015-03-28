/*
 * fout.c / PulinaPussi 0.12
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

struct ohandler{
  char *cmd;
  int (*cb) (IRCSession *, char *args);
} ohandlers[] = {
  "CHANLIST", IRC_showlist, 

  "FINGER",   IRC_finger,
  "PING",     IRC_ping,
  "TIME",     IRC_time,
  "VERSION",  IRC_version,

  "OP",       IRC_op,
  "DEOP",     IRC_deop,
  "VOICE",    IRC_voice,
  "DEVOICE",  IRC_devoice,
  "KICK",     IRC_kick,

  "CHANNEL",  IRC_channel,
  "ME",       IRC_action,
  "MSG",      IRC_msg,
  "LEAVE",    IRC_part,
  "PART",     IRC_part,
  "QUIT",     IRC_quit,
  "SERVER",   IRC_server,
  "NICK",     IRC_nick,
  "SET",      IRC_set,

  "PLUGIN",   IRC_plugin,

  "LICENSE",  IRC_license,
  "HELP",     IRC_help
};

int ohandlerc= sizeof(ohandlers) / sizeof(struct ohandler);

int
IRC_op(IRCSession *IRCs, char *nick) {
  char *outs=NULL;
  if (!nick || !IRCs->curchan) return FALSE;

  outs= strdup_printf("MODE %s +o :%s\n", IRCs->curchan, nick);
  Socket_writeln(IRCs, outs);
  Free(outs);

  return TRUE;
}

int
IRC_deop(IRCSession *IRCs, char *nick) {
  char *outs=NULL;
  if (!nick || !IRCs->curchan) return FALSE;

  outs= strdup_printf("MODE %s -o :%s\n", IRCs->curchan, nick);
  Socket_writeln(IRCs, outs);
  Free(outs);

  return TRUE;
}

int
IRC_voice(IRCSession *IRCs, char *nick) {
  char *outs=NULL;
  if (!nick || !IRCs->curchan) return FALSE;

  outs= strdup_printf("MODE %s +v :%s\n", IRCs->curchan, nick);
  Socket_writeln(IRCs, outs);
  Free(outs);

  return TRUE;
}

int
IRC_devoice(IRCSession *IRCs, char *nick) {
  char *outs=NULL;
  if (!nick || !IRCs->curchan) return FALSE;

  outs= strdup_printf("MODE %s -v :%s\n", IRCs->curchan, nick);
  Socket_writeln(IRCs, outs);
  Free(outs);

  return TRUE;
}

int 
IRC_kick(IRCSession *IRCs, char *cnt) {
  char **args=NULL;/*args[0] = channel
		     args[1] = user
		     args[2] = reason*/
  char *outs=NULL;
  int narg;

  if (!cnt || !IRCs->curchan) return FALSE;

  args= split(cnt, " ", &narg);
  if (!args) return FALSE;

  if (narg==1) {
    outs= strdup_printf("KICK %s %s\n", IRCs->curchan, cnt);
    Socket_writeln(IRCs, outs);
  }
  if (narg==2) {
    outs= strdup_printf("KICK %s %s\n", args[0], args[1]);
    Socket_writeln(IRCs, outs);
  }
  if (narg==3) {
    outs= strdup_printf("KICK %s %s :%s\n", args[0], args[1], args[2]);
    Socket_writeln(IRCs, outs);
  }

  split_free(args, narg);
  Free(outs);  

  return TRUE;
}

int
IRC_showlist(IRCSession *IRCs, char *cnt) {
  LLItem *ci= IRCs->chanlist;
  if (!IRCs->chanlist) return FALSE;

  while (ci->next != NULL){
    if (ci->name) Client_msgf(IRCs, 0, "[CHANLIST]", 
				  strdup_printf("%s\n", ci->name), TRUE);
    ci= (LLItem*)ci->next;
  }
  if (ci->name) Client_msgf(IRCs, 0, "[CHANLIST]", 
				  strdup_printf("%s\n", ci->name), TRUE);

  return TRUE;
}


int
IRC_action(IRCSession *IRCs, char *cnt) {
  char *s=NULL;
  
  if (!IRCs->curchan || !cnt) return FALSE;
  s= strdup_printf("PRIVMSG %s :\01ACTION %s\01\n", IRCs->curchan, cnt);
  Socket_writeln(IRCs, s);
  Free(s);

  return TRUE;
}

int
IRC_channel(IRCSession *IRCs, char *cnt) {
  if (!cnt) return FALSE;

  if (LList_exists(IRCs->chanlist, cnt)){
    Free(IRCs->curchan);
    IRCs->curchan= Strdup(cnt);
    Client_drawp(IRCs);    
  }
  if (!LList_exists(IRCs->chanlist, cnt)) {
    Client_msgf(IRCs, 0, "[ERROR]", strdup_printf("You are not on channel %s\n", cnt), 
		TRUE);
  }

  return FALSE;
}

int
IRC_finger(IRCSession *IRCs, char *nick) {
  char *s=NULL;

  if (!nick) return FALSE;
  s= strdup_printf("PRIVMSG %s :\01FINGER\01\n", nick);
  Socket_writeln(IRCs, s);
  Free(s);

  return TRUE;
}

int
IRC_ping(IRCSession *IRCs, char *nick) {
  char *s=NULL;

  if (!nick) return FALSE;
  s= strdup_printf("PRIVMSG %s :\01PING\01\n", nick);
  Socket_writeln(IRCs, s);
  Free(s);

  return TRUE;
}

int
IRC_time(IRCSession *IRCs, char *nick) {
  char *s=NULL;
  
  if (!nick) return FALSE;
  s= strdup_printf("PRIVMSG %s :\01TIME\01\n", nick);
  Socket_writeln(IRCs, s);
  Free(s);

  return TRUE;
}

int
IRC_version(IRCSession *IRCs, char *nick) {
  char *s=NULL;
  
  if (!nick) return FALSE;
  s= strdup_printf("PRIVMSG %s :\01VERSION\01\n", nick);
  Socket_writeln(IRCs, s);
  Free(s);

  return TRUE;
}

int
IRC_msg(IRCSession *IRCs, char *cnt) {
  char *nick= NULL, *msg= NULL;
  if (!cnt) return FALSE;

  if (Strchr(cnt, ' ') && Strlen(Strchr(cnt, ' '))>1){
    char *cmd=NULL;
    msg= (char*)Strdup(Strchr(cnt, ' ')+1);
    nick= (char*)getbc(cnt, ' ');

    cmd= (char*)strdup_printf("PRIVMSG %s :%s\n", nick, msg);
    Socket_writeln(IRCs, cmd);

    Client_msgf(IRCs, 0, nick, strdup_printf("%s\n", msg), TRUE);

    Free(cmd);
  }
  Free(msg);
  Free(nick);

  return TRUE;
}

int
IRC_quit(IRCSession *IRCs, char *reason) {
  if (!reason){
    Socket_writeln(IRCs, "QUIT\n"); 
  }else {
    char *s=NULL;
    s= (char*)strdup_printf("QUIT :%s\n", reason);
    Socket_writeln(IRCs, s);
    Free(s);
  }
  err_exit(NULL, 0);

  return TRUE;
}

int
IRC_part(IRCSession *IRCs, char *channels) {
  char *s=NULL;

  s= (char*)strdup_printf("PART %s\n", channels);
  Socket_writeln(IRCs, s);

  Free(s);

  return TRUE;
}

int
IRC_plugin(IRCSession *IRCs, char *s) {
  char **p;
  int nargs;

  if (!s) {
    PPListPlugins(IRCs);
    return TRUE;
  }
  
  p= split(s, "\" =", &nargs);
  if (!p) return FALSE;

  if (nargs==1) {
    if (!Strcasecmp(p[0], "LIST")) {
      PPListPlugins(IRCs);
    }
  }

  if (nargs==2) {

    if (!Strcasecmp(p[0], "LOAD")) {
      if (!PPLoadPlugin(IRCs, p[1])) {
	Client_msgf(IRCs, 0, "[server]", 
		    strdup_printf("Can't load plugin (%s)\n", p[1]), TRUE);
      }else {
	Client_msgf(IRCs, 0, "[server]", 
		    strdup_printf("Plugin loaded (%s)\n", p[1]), TRUE);
      }
    }

    if (!Strcasecmp(p[0], "CLOSE")) {
      if (PPClosePlugin(IRCs, p[1])) {
	Client_msgf(IRCs, 0, "[server]", 
		    strdup_printf("Plugin closed (%s)\n", p[1]), TRUE);
      }else {
	Client_msgf(IRCs, 0, "[server]", 
		    strdup_printf("Plugin not open (%s)\n", p[1]), TRUE);
      }
    }
  }

  split_free(p, nargs);
}

int
IRC_docmd(IRCSession *IRCs, char *cmd, char *args) {
  int i, isbuiltin=0;

  if (!cmd) return FALSE;

  for (i=0;i<ohandlerc;i++){
    if (Strcasecmp(cmd, ohandlers[i].cmd)==0){
      ohandlers[i].cb(IRCs, args);
      isbuiltin=1;
    }
  }

  /*plugin PPHOOKTYPE_FOUT*/
  {
    LLItem *pi= IRCs->pluginlist;

    while (pi) {
      if (pi->data) {
	PPpluginfile *pfile;
	LLItem *li;

	pfile= (PPpluginfile*)pi->data;

	li= (LLItem*)LList_get(pfile->funclist, cmd);

	if (li){
	  PPpluginhook *phook;
	  phook= (PPpluginhook*) li->data;

	  if (phook->hooktype == PPHOOKTYPE_FOUT && phook->cb) {
	    ((void(*)(IRCSession*, char*))phook->cb)(IRCs, args);
	    isbuiltin=1;
	  }
	}
      }		

      pi= (LLItem*) pi->next;
    }
  }
  /****/


  if (isbuiltin==0 && Strlen(cmd)>1) {
    char *outs=NULL;

    outs= (char*)strdup_printf("%s %s", cmd, args);

    Debug_msgf(IRCs, strdup_printf("[%s]\n", outs), TRUE);

    Socket_writeln(IRCs, outs);
    Socket_writeln(IRCs, "\n");
    Free(outs);
  }

  return TRUE;
}

int
IRC_input_handle(IRCSession *IRCs, char *s) {
  if (!s) return FALSE;
  
  /*ISCMD?*/
  if (s[0]=='/'){
    char *cmd=NULL, *args=NULL;
    if (Strlen(s)<2) return 0;

    /*NOARGS*/
    if (!Strchr(s, ' ')){
      cmd= (char*)Strdup(s+1);

	Debug_msgf(IRCs, strdup_printf("CMD[%s]\n", cmd), 1);
	IRC_docmd(IRCs, cmd, NULL);
	Free(cmd);
    }
    /*ARGS*/
    else {
      cmd=  (char*)getbc (s+1, ' ');
      
      if (Strlen(Strchr(s, ' '))>1){
	args= (char*)Strdup(Strchr(s, ' ')+1);

	Debug_msgf(IRCs, strdup_printf("CMD[%s] ARGS[%s]\n", cmd, args), 1);
	IRC_docmd(IRCs, cmd, args);

	Free(args);
      }
      Free(cmd);
    }
    
    /*EARGS*/
    
  }/*EISCMD*/else {
   
    if (IRCs->curchan) {
      char *os=NULL;

      if (*IRCs->curchan=='-' && *(IRCs->curchan+1)) {
	os= (char*)strdup_printf("PRIVMSG %s :%s\n", IRCs->curchan+1, s);	
      } else {
	os= (char*)strdup_printf("PRIVMSG %s :%s\n", IRCs->curchan, s);
      }      

      if (PPopt_getvalue(IRCs->optlist, "show_hilightown", NULL)){
	Client_msgf(IRCs, 0, IRCs->curchan, strdup_printf("\03""14<%s> %s\n",
							  IRCs->ircnick, s), 
		    TRUE);
      }else {
	Client_msgf(IRCs, 0, IRCs->curchan, strdup_printf(">%s\n", s), TRUE);
      }

      Socket_writeln(IRCs, os);
    }else {/*!curchan*/
      Client_msgf(IRCs, 0, "[ERROR]", "No channel joined\n", FALSE);
    }
  }

  return TRUE;
}

int
IRC_license(IRCSession *IRCs, char *s) {
  Client_msgf(IRCs, 0, "[LICENSE]", LICENSEMSG, 0);

  return TRUE;
}

int
IRC_help(IRCSession *IRCs, char *s) {
  Client_msgf(IRCs, 0, "[HELP]", HELPMSG, 0);
  Client_msgf(IRCs, 0, "[HELP]", "\n", 0);

  return TRUE;
}

/*int
IRC_log(IRCSession *IRCs, char *s) {
  if (!s && IRCs->logfile && IRCs->logfilename) {
    Client_msgf(IRCs, 0, "[LOG]", strdup_printf("ON %s\n", IRCs->logfilename), 1);
}
  return TRUE;
  }*/

int
IRC_nick(IRCSession *IRCs, char *s) {
  if (!s) return FALSE;

  if (IRCs->connected) {
    Socket_outf(IRCs, strdup_printf("NICK :%s\n", s), 1);
  }else {
    Free(IRCs->ircnick);
    IRCs->ircnick= Strrm(s, ' ');
    Client_msgf(IRCs, IRCO_NICKNAME_UPDATE, "[server]", "raivo", 0);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("you are now known as %s\n",
						   IRCs->ircnick), TRUE);
  }
  return TRUE;
}

int
IRC_set(IRCSession *IRCs, char *s) {
  char **p;
  int n;

  if (!s) {
    PPopt_printall(IRCs, IRCs->optlist);
    return TRUE;
  }

  p= split(s, "\" =", &n);
  if (!p) return FALSE;

  if (n>2  && *p[1]!='\"') {
  
  }
  if (n>=2) {
    int vtype;
    void *v;

    v= PPopt_getvalue(IRCs->optlist, p[0], &vtype);
    Debug_msgf(IRCs, strdup_printf("%d %s %s", vtype, p[0], p[1]), TRUE);


    if (vtype == OPTTYPE_BOOL) {
      if (toupper(*p[1])=='T') {
	PPopt_setvalue(IRCs->optlist, p[0], TRUE, NULL);
      }
      if (toupper(*p[1])=='F') {
	PPopt_setvalue(IRCs->optlist, p[0], FALSE, NULL);
      }
      if (!Strcasecmp(p[1], "on")){
	PPopt_setvalue(IRCs->optlist, p[0], TRUE, NULL);
      }
      if (!Strcasecmp(p[1], "off")) {
	PPopt_setvalue(IRCs->optlist, p[0], FALSE, NULL);
      }
    }
    if (vtype == OPTTYPE_STRING) {
      if (n==2){ 
	PPopt_setvalue(IRCs->optlist, p[0], 0, p[1]);
      } 
      if (n>2) {
	
      }
    }
    if (vtype == OPTTYPE_LONG) {
      if (isdigit(*p[1])) PPopt_setvalue(IRCs->optlist, p[0], atoi(p[1]), NULL);
    }
  }
  PPopt_print(IRCs, IRCs->optlist, p[0]);

  split_free(p, n);

  return TRUE;
}
