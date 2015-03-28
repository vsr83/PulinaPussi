/*
 * plugin.c / PulinaPussi 0.12
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

#include "pp.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef WIN32
int
PPLoadPlugin(IRCSession *IRCs, char *filename) {
  Client_msgf(IRCs, 0, "[server]", "not supported.\n", FALSE);
}
int
PPClosePlugin(IRCSession *IRCs, char *filename) {
  Client_msgf(IRCs, 0, "[server]", "not supported.\n", FALSE);
}
#else
int
PPClosePlugin(IRCSession *IRCs, char *filename) {
  LLItem *pf;
  PPpluginfile *pfile;

  pf= LList_get(IRCs->pluginlist, filename);
  if (pf && pf->data) {
    pfile= (PPpluginfile*)pf->data;

    LList_free(pfile->funclist);
    dlclose(pfile->dlhandle);
    LList_remove(IRCs->pluginlist, filename);
  }
}

int
PPLoadPlugin(IRCSession *IRCs, char *filename) {
  PPpluginfile *pf;

  if (LList_exists(IRCs->pluginlist, filename)) return FALSE;

  pf= (PPpluginfile*)Calloc(1, sizeof(PPpluginfile));
  /*RTLD_NOW doesn't work under OpenBSD*/
  pf->dlhandle= dlopen(filename, RTLD_LAZY); 

  if (!pf->dlhandle) {
    fprintf(stderr, "%s\n", dlerror());
    free(pf);
    return FALSE;
  }
  pf->filename= Strdup(filename);
  pf->funclist= LList_new();

  //  printf("ok.\n");

  {
    void (*pinit)(void *);
    pinit= (void (*)(void *))dlsym(pf->dlhandle, "PPplugin_init");
    pinit(IRCs);
  }
  {
    PPpluginhook *hooks;
    int n;

    hooks= (PPpluginhook*) dlsym(pf->dlhandle, "PPpluginhooks");

    for (n=0;hooks[n].cb;n++) {
      Debug_msgf(IRCs, strdup_printf("\nfuncsym    : \"%s\"\n"
				     "funcname   : \"%s\"\n"
				     "hooktype   : %d\n"
				     "replaceold : %s\n",
				     hooks[n].funcname,
				     hooks[n].name,
				     hooks[n].hooktype,
				     hooks[n].disableold? "TRUE":"FALSE"),
		 TRUE);

      if (!dlsym(pf->dlhandle, hooks[n].funcname)) {
	fprintf(stderr, "THIS PLUGIN IS EVIL!!\n");
      }

      LList_add    (pf->funclist, hooks[n].name, &hooks[n]);
    }
    if (IRCs) LList_add(IRCs->pluginlist, filename, pf);
  }
}
#endif

int
PPListPlugins(IRCSession *IRCs) {
  LLItem *li=IRCs->pluginlist;
  int n=0;

  Client_msgf(IRCs, 0, "[server]", 
	      "- Active Plugins -------------\n", FALSE);
  while (li) {
    if (li->data) {

      PPpluginfile *pf= (PPpluginfile *) li->data;
      
      Client_msgf(IRCs, 0, "[server]", 
		  strdup_printf("%d. %s\n", ++n, pf->filename), TRUE);
    }
    
    li= (LLItem*) li->next;
  } 
  
  Client_msgf(IRCs, 0, "[server]", 
	      "------------------------------\n", FALSE);
}

#ifdef TEST

finish(int sig) {};

int 
main(int argc, char **argv) {
  PPLoadPlugin(NULL, "./libtesti.so.0.0");
}
#endif
