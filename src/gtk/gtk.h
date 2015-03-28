#ifndef __PP_GTK_H__
#define __PP_GTK_H__

#include <gtk/gtk.h> 
#include "userlist.h"

typedef struct{
  GtkWidget *main_window;
  GtkWidget *main_notebook;
  GtkWidget *main_menubar;

  GtkWidget *about_window;
  //  struct PPGUIopt guiopt;

  gint inputt;
}PPgtk;

GtkWidget *notebook_get_widget (GtkWidget *, char *);
GtkWidget *notebook_userlist   (GtkWidget *, char *);
GtkWidget *notebook_entry      (GtkWidget *, char *);
GtkWidget *notebook_topic      (GtkWidget *, char *);

gint notebook_getnthbyname     (GtkWidget *, char *);
gint notebook_setchanged       (GtkWidget *, int , char *, int);
gint notebook_pageswitch       (GtkWidget *, gpointer, gint);
gint notebook_settopic         (GtkWidget *, char *, char*);
gint notebook_clear            (GtkWidget *, char *);
gint notebook_setfont          (GtkWidget *, char *);
gint notebook_setnicklabel     (GtkWidget *, char *);
gint notebook_page_exists      (GtkWidget *, char *);
gint notebook_add              (GtkWidget *, char *, int);
gint notebook_remove           (GtkWidget *, char *);
gint notebook_msg              (GtkWidget *, char *, char *);
gint notebook_msgall           (GtkWidget *, char *);
gint notebook_set_style        (GtkWidget *, GdkColor *, GdkColor *);

GtkWidget *aboutdlg_new();
GtkWidget *notebook_new(IRCSession *IRCs);



#endif
