#ifndef _PP_FOUT_H_
#define _PP_FOUT_H_

int IRC_showlist(IRCSession *, char*);

int IRC_action (IRCSession *, char*);
int IRC_finger (IRCSession *, char*);
int IRC_ping   (IRCSession *, char*);
int IRC_time   (IRCSession *, char*);
int IRC_version(IRCSession *, char*);

int IRC_channel(IRCSession *, char *);
int IRC_msg    (IRCSession *, char*);
int IRC_part   (IRCSession *, char*);

int IRC_server (IRCSession *, char*);
int IRC_nick   (IRCSession *, char*);
int IRC_quit   (IRCSession *, char*);

int IRC_op     (IRCSession *, char*);
int IRC_deop   (IRCSession *, char*);
int IRC_voice  (IRCSession *, char*);
int IRC_devoice(IRCSession *, char*);
int IRC_kick   (IRCSession *, char*);

int IRC_license(IRCSession *, char*);
int IRC_help   (IRCSession *, char*);
//int IRC_log    (IRCSession *, char*);

int IRC_plugin(IRCSession *, char*);
int IRC_set    (IRCSession *, char*);

int IRC_input_handle(IRCSession *, char *);

#endif
