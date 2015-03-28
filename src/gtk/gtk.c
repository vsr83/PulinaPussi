/*
 * gtk.c / PulinaPussi 0.12
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

/*
0x080530c3 in userlist_sortfunc (cl=0x807db18, row1=0x80b1308, row2=0x80b12d8)
    at gtk/userlist.c:49
49          if (*usermode1 == '@' && *usermode2 != '@') return -1;

*/

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pwd.h>
#include "../pp.h"
#include "gtk.h"

gint       aboutdlg_show(GtkWidget *, PPgtk *);
GtkWidget *menuitem_new (GtkWidget *, char *, char *);
void       menu_create  (IRCSession *);
void       mainw_create (IRCSession *);
void       ppgtk_new    (IRCSession *);

gint       PPsocket_read(IRCSession *, gint, GdkInputCondition);
gint       PPoutcb      (GtkWidget *, IRCSession*);
gint       Client_msgf  (IRCSession *, int, char *, char *, int);
int        Client_drawp (IRCSession *);
int        Debug_msgf   (IRCSession *, char *, int);
int        IRC_server   (IRCSession *, char *);

/*************************************************************
 * gint PPsocket_read(source, cond)
 * Function which is called when there is data incoming to a socket.
 *
 * gint source=IRCs->sockfd;
 *************************************************************/
gint
PPsocket_read(IRCSession *IRCs, gint source, GdkInputCondition cond) {
  Socket_readln(IRCs);

  Debug_msgf(IRCs, IRCs->sockbuf, 0);
  Debug_msgf(IRCs, "\n", 0);

  /*  for (i=0;i<Strlen(IRCs->sockbuf);i++) {
    Debug_msgf(IRCs, strdup_printf("%c[%d]", IRCs->sockbuf[i], IRCs->sockbuf[i]),1);
  }
  Debug_msgf(IRCs, "\n", 0);
  */
  IRC_handle(IRCs, IRCs->sockbuf);
  return TRUE;
}

gint
aboutdlg_show(GtkWidget *w, PPgtk *ppgtk) {
  gtk_widget_show_all(ppgtk->about_window);
  return TRUE;
}

/*Wrapper used to make menuitems*/
GtkWidget *
menuitem_new(GtkWidget *cmenu, char *text, char *key) {
  GtkWidget *menu_item, *hbox, *label;
  
  menu_item= gtk_menu_item_new();
  hbox= gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(menu_item), hbox);

  label= gtk_label_new(text);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  if (key) {
    label= gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
    label= gtk_label_new(key);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  }
  gtk_menu_append(GTK_MENU(cmenu), menu_item);
  return menu_item;
} 

/*Callback used to toggle the visibility of certain parts of the UI.*/
gint
ppgtk_sethidden(GtkWidget *menuitem, IRCSession *IRCs) {
  gint length, i, viewable, nitem;
  GtkWidget *nb;
  PPgtk *ppgtk= (PPgtk*) IRCs->interface;

  if (!IRCs || !menuitem) return FALSE;
  

  nb= ppgtk->main_notebook;
  nitem= GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(menuitem), "nitem"));

  if (!nb) return FALSE;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);
  viewable=  GTK_CHECK_MENU_ITEM(menuitem)->active;
  //  g_print("viewable %d %d\n", nitem, viewable);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && GTK_LABEL(curpage->tab_label)->label) {
      GtkWidget *menushit=NULL;

      switch(nitem) {
      case 1:
	menushit= notebook_topic(nb, GTK_LABEL(curpage->tab_label)->label);
	break;
      case 2:
	menushit= notebook_userlist(nb, GTK_LABEL(curpage->tab_label)->label);
	break;
      }
      if (menushit){
	if (viewable) gtk_widget_show_all(menushit);
	else gtk_widget_hide(menushit);
      }
    }
  }

  return TRUE;
}

/*Create menu*/
void
menu_create(IRCSession *IRCs) {
  GtkWidget *file_menu, *help_menu, *menu_item, *file_item, *help_item,
    *view_menu, *view_item;
  GtkAccelGroup *accel_group;

  PPgtk *ppgtk;
  if (!IRCs) return;
  ppgtk= (PPgtk*)IRCs->interface;

  accel_group= gtk_accel_group_new();

  file_menu= gtk_menu_new();
  help_menu= gtk_menu_new();
  view_menu= gtk_menu_new();

  gtk_menu_set_accel_group(GTK_MENU(file_menu), accel_group);
  gtk_window_add_accel_group(GTK_WINDOW(ppgtk->main_window), accel_group);
  ppgtk->main_menubar= gtk_menu_bar_new();

  file_item= gtk_menu_item_new_with_label("File");
  gtk_menu_bar_append(GTK_MENU_BAR(ppgtk->main_menubar), file_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

  view_item= gtk_menu_item_new_with_label("View");
  gtk_menu_bar_append(GTK_MENU_BAR(ppgtk->main_menubar), view_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), view_menu);

  help_item= gtk_menu_item_new_with_label("Help");
  gtk_menu_bar_append(GTK_MENU_BAR(ppgtk->main_menubar), help_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
  gtk_menu_item_right_justify(GTK_MENU_ITEM(help_item));


  view_item= gtk_check_menu_item_new_with_label("Topic");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_item), TRUE);
  gtk_menu_append(GTK_MENU(view_menu), view_item);
  gtk_object_set_data(GTK_OBJECT(view_item), "nitem", GINT_TO_POINTER(1));
  gtk_signal_connect(GTK_OBJECT(view_item), "activate", (GtkSignalFunc)ppgtk_sethidden, IRCs);

  view_item= gtk_check_menu_item_new_with_label("Userlist");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(view_item), TRUE);
  gtk_menu_append(GTK_MENU(view_menu), view_item);
  gtk_object_set_data(GTK_OBJECT(view_item), "nitem", GINT_TO_POINTER(2));
  gtk_signal_connect(GTK_OBJECT(view_item), "activate", (GtkSignalFunc)ppgtk_sethidden, IRCs);


  menu_item= menuitem_new(help_menu, "About", NULL);
  gtk_signal_connect(GTK_OBJECT(menu_item), "activate", (GtkSignalFunc)aboutdlg_show, ppgtk);

  menu_item= menuitem_new(file_menu, "Quit", "Ctl-q");
  gtk_signal_connect(GTK_OBJECT(menu_item), "activate", gtk_main_quit, NULL);
  gtk_widget_add_accelerator(menu_item, "activate", accel_group, 'q', 
			     GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
}

/*Create the IRCs->main_window*/
void
mainw_create(IRCSession *IRCs) {
  GtkWidget *main_vbox;
  PPgtk *ppgtk;

  if (!IRCs) return;
  ppgtk= (PPgtk*) IRCs->interface;

  ppgtk->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(ppgtk->main_window), "PulinaPussi-GTK 0.11");

  gtk_window_set_default_size(GTK_WINDOW(ppgtk->main_window), 550, 300);

  ppgtk->main_notebook= notebook_new(IRCs);

  main_vbox= gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(ppgtk->main_window), main_vbox);

  menu_create(IRCs);
  gtk_box_pack_start(GTK_BOX(main_vbox), ppgtk->main_menubar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), ppgtk->main_notebook, TRUE, TRUE, 0);
}


/*Command history*/
gint
entry_keypress(GtkWidget *entry, GdkEvent *event, IRCSession *IRCs) {
  GdkEventKey *key= (GdkEventKey*)event;

  //  g_print("%d\n", key->keyval);
  if (key->keyval== GDK_Up) {
    gtk_signal_emit_stop_by_name(GTK_OBJECT(entry), "key_press_event");
  }
  return TRUE;
}


/***************************************************
 * gint PPoutcb(entry) (CALLBACK)
 * Handles input written by the user.
 **************************************************/
gint 
PPoutcb(GtkWidget *entry, IRCSession *IRCs) {
  gchar *vammaosio, *s;

  PPgtk *ppgtk= (PPgtk*) IRCs->interface;

  s= gtk_entry_get_text(GTK_ENTRY(entry));
  if (s == NULL || !*s) return FALSE;

  vammaosio= (gchar*)gtk_object_get_data(GTK_OBJECT(entry), "kanavanimi");
  if (vammaosio == NULL) return FALSE;

  gtk_entry_set_text(GTK_ENTRY(entry), "\0");
  //g_print("|%s|%s|\n", vammaosio, s);

  if (!g_strncasecmp(s, "/CLOSE", 6)){
    if (*vammaosio== '#' || *vammaosio=='&' || IS_NICKC(*vammaosio)) {
      notebook_remove(ppgtk->main_notebook, vammaosio);
    }
    return TRUE;
  }
  if (!g_strncasecmp(s, "/CLEAR", 6)) {
    notebook_clear(ppgtk->main_notebook, vammaosio);
    return TRUE;
  }
  if (!g_strncasecmp(s, "/SERVER", 7)) {
    IRC_server(IRCs, s);
    return TRUE;
  }

  Free(IRCs->curchan);

  if (*vammaosio=='#' || *vammaosio=='&' || *vammaosio=='-') {
    IRCs->curchan= Strdup(vammaosio);
  }
  IRC_input_handle(IRCs, s);
  return TRUE;
}

/*UI-wrappers****************************************************************/
int
Client_msgf(IRCSession *IRCs, int oper, char *to, char *s, int freeit) {
  gchar *pagename;

  PPgtk *ppgtk=NULL;
  if (IRCs) ppgtk= (PPgtk*) IRCs->interface;

  if (!to || !s || !IRCs || !ppgtk || !ppgtk->main_notebook) {
    if (s && freeit) Free(s);
    return FALSE;
  }



  if (*to == '[') {
    pagename= Strdup("[server]");
  } 
  else {
    pagename= Strdup(to);
  }

  /*Add a new tab to the notebook.*/
  if (!notebook_page_exists(ppgtk->main_notebook, pagename) && !oper) {
    GtkWidget *entry=NULL;
    char *ename;
    int ischan=0, ntabs=0, maxtabs=0, vtype=0;

    if (*pagename == '#' || *pagename =='&') ischan=1;
    ntabs= g_list_length(GTK_NOTEBOOK(ppgtk->main_notebook)->children);
    maxtabs= (int)PPopt_getvalue(IRCs->optlist, "max_tabs", &vtype);
    if (vtype!=OPTTYPE_LONG) maxtabs=0;

    g_print("%d/%d TABS\n", ntabs, maxtabs);

    if (ntabs<maxtabs) {
      notebook_add(ppgtk->main_notebook, pagename, ischan);
      entry= notebook_entry(ppgtk->main_notebook, pagename);
#ifdef __cplusplus
      gtk_signal_connect(GTK_OBJECT(entry), "key_press_event", 
			 (void (*)(...))entry_keypress,
			 IRCs);
#else
      gtk_signal_connect(GTK_OBJECT(entry), "key_press_event", 
			 entry_keypress,
			 IRCs);
#endif
      if (entry) {
	ename= Strdup(pagename);
	gtk_object_set_data(GTK_OBJECT(entry), "kanavanimi", ename);
	gtk_signal_connect(GTK_OBJECT(entry), "activate", (GtkSignalFunc)PPoutcb,
			   IRCs);
      }
      notebook_setnicklabel(ppgtk->main_notebook, IRCs->ircnick);
    } else {
      Client_msgf(IRCs, IRCO_MESSAGE,"[server]", 
		  g_strdup_printf("can't add tab %s\n", pagename), TRUE);

      if (s && freeit) Free(s);
      Free(pagename);

      return FALSE;
    }
  }

  switch (oper) { /*UGLY*/
    /*Send message to a window.*/
  case IRCO_MESSAGE:
    notebook_msg(ppgtk->main_notebook, pagename, s);
    break;
    /*Send message to all windows.*/
  case IRCO_MESSAGE_ALL:
    notebook_msgall(ppgtk->main_notebook, s);
    break;
    /*Send message to all channels where [to] is present.*/
  case IRCO_MESSAGE_USER_ALL:
    {
      LLItem *ci= IRCs->chanlist;
     
      while (ci!=NULL) {
	if (ci->name) {
	  if (userlist_exists(notebook_userlist(ppgtk->main_notebook, 
						ci->name), to)) {
	    notebook_msg(ppgtk->main_notebook, ci->name, s);
	  }
	}
	ci= (LLItem*) ci->next;
      } 
    }
    break;
    /*Add user to an userlist.*/
  case IRCO_USERLIST_ADD:
    {
      char *username;
      int mode;
      
      if (*s=='@' && isalpha(s[1])) {
	mode= USERMODE_OPERATOR;
	username= Strdup(s+1);
      }else if (*s=='+' && isalpha(s[1])) {
	mode= USERMODE_VOICE;
	username= Strdup(s+1);
      }
      else{
	mode= 0;
	username= Strdup(s);
      }
      
      if (userlist_exists(notebook_userlist(ppgtk->main_notebook, 
					    pagename), username)) {
	userlist_remove(notebook_userlist(ppgtk->main_notebook, pagename),
			username);
      }
      userlist_add(notebook_userlist(ppgtk->main_notebook, pagename), 
		   username, mode);       
      Free(username);
    }
    break;
    /*Remove user from userlist.*/
  case IRCO_USERLIST_REMOVE:
    userlist_remove(notebook_userlist(ppgtk->main_notebook, pagename), s);
    break;
  case IRCO_USERLIST_SETMODE:
    {/*Not finished.... +ov -> +v*/
      char *modes, *nickname, *nicks;
      int mode;

      if (!Strchr(s, ':')) break;

      nicks= Strchr(s, ':')+1;
      modes= getbc(s, ':');

      /*Servers adds ' ' to MODE-string.*/
      nickname= Strrm(nicks, ' ');

      if (!modes || !nickname || !*nickname) break;

      //      if (!notebook_userlist(ppgtk->main_notebook, pagename)) g_print("PERKELESAATANA\n");
      mode= userlist_getmode(notebook_userlist(ppgtk->main_notebook, 
					       pagename), nickname);
 
      //      g_print("|%d|%s|\n", mode, modes);
      if (!Strcmp(modes, "+o") && !(mode & USERMODE_OPERATOR)){
	mode+= USERMODE_OPERATOR;
      }
      if (!Strcmp(modes, "-o") && mode & USERMODE_OPERATOR) {
	mode-= USERMODE_OPERATOR;
      }
      if (!Strcmp(modes, "+v") && !(mode & USERMODE_VOICE)) {
	mode+= USERMODE_VOICE;
      } 
      if (!Strcmp(modes, "-v") && mode & USERMODE_VOICE) {
	mode-= USERMODE_VOICE;
      }
      
      // g_print("MODE |%d|%s|%s|\n", mode, pagename, nickname);
      userlist_setmode(notebook_userlist(ppgtk->main_notebook, pagename), 
		       nickname, mode);
      Free(modes);
      Free(nickname);
    }
    break;
  case IRCO_USERLISTS_REMOVE:
    {
      LLItem *ci= IRCs->chanlist;
      
      while (ci!=NULL) {
	if (ci->name) {
	  userlist_remove(notebook_userlist(ppgtk->main_notebook, ci->name), 
			  s);
	}
	ci= (LLItem*) ci->next;
      }
    }
    break;
  case IRCO_USERLISTS_NICK:
    {
      LLItem *ci= IRCs->chanlist;
      char *username;
      int mode;

      username= Strdup(to);
      
      while (ci!=NULL){
	if (ci->name) {
	  GtkWidget *ul= notebook_userlist(ppgtk->main_notebook, ci->name);
	  if (userlist_exists(ul, username)) {
	    mode= userlist_getmode(ul, username);
	    userlist_remove(ul, username);
	    userlist_add(ul, s, mode);
	  }
	}
	ci= (LLItem*) ci->next;
	if (ci==NULL) break;
      }
      Free(username);
    }      
    break;
  case IRCO_SETTOPIC:
    {
      char *ts;
      ts= g_strdup_printf("%s - %s", pagename, s);
      notebook_settopic(ppgtk->main_notebook, pagename, ts);
      Free(ts);
      break;
    }
  case IRCO_NICKNAME_UPDATE:
    notebook_setnicklabel(ppgtk->main_notebook, IRCs->ircnick);
    break;
  }

  if (freeit) Free(s);
  Free(pagename);
  return TRUE;
}

void
ppgtk_new(IRCSession *IRCs) {
  PPgtk *ppgtk;
  
  IRCs->interface= (PPgtk*)calloc(1, sizeof(PPgtk));
  if (!IRCs->interface) {
    g_error("calloc");
    gtk_main_quit();
    exit(0);
  }

  ppgtk= (PPgtk*) IRCs->interface;

  ppgtk->about_window= aboutdlg_new();
  mainw_create(IRCs);
}


int
Client_drawp(IRCSession *IRCs) {
  return TRUE;
}

int
Debug_msgf(IRCSession *IRCs, char *msg, int dofreeshit) {
  Client_msgf(IRCs, 0, "*RAW*", msg, 0);
  if (dofreeshit) Free(msg);
  return TRUE;
}
/****************************************************************************/

void
err_exit(char *msg, int do_perror) {
  gdk_threads_leave();
  if (msg) {
    g_print("ERROR: %s\n", msg);
  }
  gtk_main_quit();
  exit(0);
}

int
IRC_server(IRCSession *IRCs, char *_s) {
  char *ihost, *ports, *s;
  int port=DEFAULTPORT;

  PPgtk *ppgtk= (PPgtk*) IRCs->interface;

  if (!_s || !Strchr(_s, ' ')) return FALSE;
  s= Strchr(_s, ' ')+1;
  if (!*s) return FALSE;
  
  ihost= getbc(s, ':');
  if (!ihost) ihost= Strdup(s);

  ports= Strchr(s, ':');
  if (ports && isdigit(*(ports+1))) port= atoi(ports+1);

  if (ihost) {
    if (IRCs->connected) {
      Socket_outf(IRCs, "QUIT\n", 0);
      Client_msgf(IRCs, 0, "[server]", "disconnected.\n", 0);
      IRCs->connected= 0;
      gdk_input_remove(ppgtk->inputt);
      close(IRCs->sockfd);
    }
    Free (IRCs->ircserver);
    IRCs->ircserver= Strdup(ihost);
    IRCs->ircport= port;

    Client_connect(IRCs, IRCs->ircserver, IRCs->ircport);
    ppgtk->inputt= gdk_input_add(IRCs->sockfd, GDK_INPUT_READ, 
				       (GdkInputFunction)PPsocket_read,
				       IRCs);

    return TRUE;
  }
  return FALSE;
}

gint
segf_find(IRCSession *IRCs) {
  {
    static int n=0;
    int ofd;

    //    srand(time(NULL));

    //ofd= open("/dev/null", O_WRONLY);
    //IRCs->sockfd= ofd;

    {
      char s[1024], s2[1024], s3[1024], s4[1024];int i;
      memset(s, 0, 1023);
      memset(s2, 0, 1023);
      memset(s3, 0, 1023);
      memset(s4, 0, 1023);

      for (i=0;i<256;i++){
	s[i]=rand()%256;
	s2[i]=rand()%256;
	s3[i]=rand()%256;
	s4[i]=rand()%256;
      }

      if (rand()%10>8) *s=0;
      if (rand()%10>8) *s2=0;
      if (rand()%10>8) *s3=0;
      if (rand()%10>8) *s4=0;

      i=rand()%600;
              
      handle_numeric(IRCs, s, "353", s2, s3);
      handle_numeric(IRCs, s, s4, s2, s3);
      
      IRC_handle(IRCs,s);
      
      // htopic(IRCs,s, s2, s3);
      //hnotice(IRCs,s, s2, s3);
      //hjoin(IRCs,s, s2, s3);
      hprivmsg(IRCs,s, s2, s3);
      hprivmsg(IRCs,s, s2, "\01FINGER");
      hprivmsg(IRCs,s, s2, "\01VERSION");
      hprivmsg(IRCs,s, s2, "\01PING");
      hprivmsg(IRCs,s, s2, "\01TIME");
      hprivmsg(IRCs,s, s2, "\01ACTION\01APINA");
      /*
      
      hmode(IRCs,s, s2, s3);
      hinvite(IRCs,s, s2, s3);
      hpart(IRCs,s, s2, s3);
      hkick(IRCs,s, s2, s3);
      hnick(IRCs,"spuuki!apina", s2, s3);
      IRC_input_handle(IRCs,s);
      
      s[0]='/';
      IRC_op(IRCs,s);
      IRC_deop(IRCs,s);
      IRC_voice(IRCs,s);
      IRC_devoice (IRCs,s);
      IRC_kick(IRCs,s);
         
      IRC_action(IRCs,s);
      IRC_finger(IRCs,s);
      IRC_ping(IRCs,s);
      IRC_time(IRCs,s);
      IRC_version(IRCs,s);
      
      IRC_channel(IRCs,s);
      IRC_msg(IRCs,s);
      IRC_part(IRCs,s);

      IRC_showlist(IRCs,s);
      
      sprintf(s, "%d\n\r", n++);
      write(0, s, Strlen(s));
      sprintf(s, "%d", rand()%100);
      LList_add(IRCs->chanlist, s, NULL);
      sprintf(s, "%d", rand()%100);
      LList_remove(IRCs->chanlist, s);
      
      //      Client_msgf(IRCs, 0, "[", s3, 0);
      memcpy(s2, "TIME ", 5);
      CTCP_handle(IRCs, s, s2);
      memcpy(s2, "PING ", 5);
      CTCP_handle(IRCs, s, s2);
      memcpy(s2, "FINGER ", 7);
      CTCP_handle(IRCs, s, s2);
      memcpy(s2, "VERSION ", 8);
      CTCP_handle(IRCs, s, s2);

      memcpy(s2, "DCC SEND ", 9);
      CTCP_handle(IRCs, s, s2);
      memcpy(s2, "DCC CHAT ", 9);
      CTCP_handle(IRCs, s, s2);*/
    }
    //    close(ofd);
  }
  return TRUE;
}

int
main(int argc, char **argv) {
  IRCSession *IRCs;

  gtk_init(&argc, &argv);
  IRCs= (IRCSession*)Calloc(1, sizeof(IRCSession));
  {
    struct passwd *pw;
    pw= getpwuid(getuid());
    if (!pw) err_exit("getpwuid\n", 0);
    
    IRCs->ircnick=     Strdup(pw->pw_name);
    IRCs->ircrealname= Strdup(pw->pw_gecos);
    IRCs->chanlist=    LList_new();
    IRCs->pluginlist=  LList_new();
    IRCs->optlist=     LList_new();
  }

  /*  IRCs->logfilename= "vammalog";
  IRCs->logfile= fopen(IRCs->logfilename, "a");
  */  
  IRCs->connected= 0;
  
  PPopt_initdefaults(IRCs, IRCs->optlist);
  PPopt_add(IRCs->optlist, "msg2", OPTTYPE_MSG, FALSE, 
	    "- GTK+-interface -------------\n");
  PPopt_add(IRCs->optlist, "buffer_maxlength", OPTTYPE_LONG,
	    25000, NULL);
  PPopt_add(IRCs->optlist, "max_tabs", OPTTYPE_LONG,
	    25, NULL);
  //PPopt_add(IRCs->optlist, "cmdhistory_maxlength", OPTTYPE_LONG,
  //	    25, NULL);

  ppgtk_new(IRCs);
  {
    PPgtk *ppgtk= (PPgtk*) IRCs->interface;
    gtk_widget_show_all(ppgtk->main_window);
    
    Client_msgf(IRCs, 0, "[server]", WMSG, 0);
    Client_msgf(IRCs, 0, "[sErVeR]", "\03""3ok.\n", 0);
  }
  {
    struct passwd *pw;
    pw= getpwuid(getuid());
    if (!Strcasecmp(pw->pw_name, "root")) {
      Client_msgf(IRCs, 0, "[server]",
		  "PLEASE DON'T RUN PULINAPUSSI AS ROOT!!\n", FALSE);
    }
  }

#ifdef SEGF_FIND
    gtk_idle_add(segf_find, IRCs);
    //gtk_timeout_add(100, segf_find, IRCs);
#endif

  //  PPLoadPlugin(IRCs, "./libtesti.so.0.0");
  //PPClosePlugin(IRCs, "./libtesti.so.0.0");

  gtk_main();
  return TRUE;
}
