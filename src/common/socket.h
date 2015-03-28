#ifndef _PP_SOCKET_H_
#define _PP_SOCKET_H_

int Client_connect  (IRCSession *, char *, int);
int Socket_readln   (IRCSession *);
int Socket_writeln  (IRCSession *, char*);
int Socket_outf     (IRCSession *, char *, int);

#endif
