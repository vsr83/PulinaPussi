#ifndef __PP_PLUGIN_H__
#define __PP_PLUGIN_H__

/*plugin.c************************************************************/
/*FIN_RAW && FOUT_RAW not yet implemented*/
enum {
  PPHOOKTYPE_NONE,
  PPHOOKTYPE_FIN,
  PPHOOKTYPE_FIN_RAW,
  PPHOOKTYPE_FOUT,
  PPHOOKTYPE_FOUT_RAW
};

typedef struct {
  char *filename;
  LLItem *funclist;

  void *dlhandle;
} PPpluginfile;

typedef struct _PPpluginhook{
  void *cb;
  char *funcname, *name;

  int hooktype;
  int disableold;
} PPpluginhook;

int PPListPlugins(IRCSession *IRCs);
int PPLoadPlugin (IRCSession *IRCs, char *filename);
int PPClosePlugin(IRCSession *IRCs, char *filename);

#endif
