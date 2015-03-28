#ifndef _PP_FIN_H_
#define _PP_FIN_H_

#define IHPARAMS (IRCSession *, char *from, char *to, char *cnt)
void htopic     IHPARAMS, hprivmsg   IHPARAMS, hpart      IHPARAMS;
void hnotice    IHPARAMS, hmode      IHPARAMS, hkick      IHPARAMS;
void hjoin      IHPARAMS, hinvite    IHPARAMS, hquit      IHPARAMS;
void hnick      IHPARAMS;

int handle_numeric (IRCSession *, char *, char *, char *, char *);
int IRC_handle      (IRCSession *, char *);

#endif
