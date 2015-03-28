#ifndef _PP_TYPES_H_
#define _PP_TYPES_H_

typedef struct{
  LLItem *chanlist, *optlist, *pluginlist;

  int sockfd;
  char sockbuf[512];

  FILE *logfile;

  char *curchan, *ircnick;
  char *ircrealname, *ircserver;
  char *logfilename;

  int ircport;
  int connected;

  void *interface;   /*Pointer to the UI-shit*/
}IRCSession;

#endif
