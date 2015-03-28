#ifndef __USERLIST_H__
#define __USERLIST_H__

#include "../pp.h"

#define USERMODE_VOICE    1
#define USERMODE_OPERATOR 2

GtkWidget *userlist_new(IRCSession *, char *);
gint userlist_add(GtkWidget *ul, char *nick, int usermode);

gint userlist_exists (GtkWidget *ul, char *nick);
gint userlist_getmode(GtkWidget *ul, char *nick);
gint userlist_getrow (GtkWidget *ul, char *nick);
gint userlist_remove (GtkWidget *ul, char *nick);
gint userlist_setmode(GtkWidget *ul, char *nick, int mode);

#endif
