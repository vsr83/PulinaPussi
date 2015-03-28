#include "../src/pp.h"

#include <sys/utsname.h>

void *
vammacb(IRCSession *IRCs, char *s) {
    Client_msgf(IRCs, 0, "[server]",strdup_printf("%s vihaa teitä\n",s), TRUE);
}

void *
testicb(IRCSession *IRCs, char *s) {
  FILE *in;
  struct utsname uts;

  if (uname(&uts)>=0) {
    Client_msgf(IRCs, 0, "[server]", 
		"-------------------------------------------------\n", FALSE);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("System name: %s\n", 
						   uts.sysname), TRUE);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("Release    : %s\n", 
						   uts.release), TRUE);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("Version    : %s\n", 
						   uts.version), TRUE);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("Hardware   : %s\n", 
						   uts.machine), TRUE);
    Client_msgf(IRCs, 0, "[server]", strdup_printf("Node name  : %s\n", 
						   uts.nodename), TRUE);
    Client_msgf(IRCs, 0, "[server]", 
		"-------------------------------------------------\n", FALSE);
  }


  Client_msgf(IRCs, 0, "[sErVeR]", 
	      "\03""2this\03""4 is \03""5testicb()\03""6 talking :).\n", 0);
  //  fprintf(stderr, "%d raivoa\n", raivo);
}

int
intesti(IRCSession *IRCs, char *from, char *to, char *cnt){
  //  Client_msgf(IRCs, 0, "[server]", strdup_printf("F|%s| T|%s| C|%s|",
  //						 from,to,cnt), TRUE);
  char *s;

  if (!from || !to) return FALSE;

  s= getbc(from, '!');
  if (s) {
    Socket_outf(IRCs, strdup_printf("PRIVMSG %s :toki %s\n", to, s), TRUE);
    Free(s);
  }
}

PPpluginhook PPpluginhooks[] = {
  intesti, "intesti", "privmsg", PPHOOKTYPE_FIN, FALSE, 
  testicb, "testicb", "help",    PPHOOKTYPE_FOUT,FALSE,
  vammacb, "vammacb", "uloste",  PPHOOKTYPE_FOUT,FALSE,
  NULL,    NULL,      NULL,    0,              0
};

int
PPplugin_init(IRCSession *IRCs){
  Client_msgf(IRCs, 0, "[sErVeR]", "libtesti loaded\n", 0);
  return TRUE;
}
