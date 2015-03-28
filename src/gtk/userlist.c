/*
 * userlist.c / PulinaPussi 0.12
 * written by Ville R‰is‰nen <raivil@geek.com> 2001-2003
 * t‰ll‰tteet kyl kannattais varmaan teh‰ p‰rlil :)
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

/*GTK on kyll‰ sitten niin vitun ‰rsytt‰v‰‰.*/

#include <gtk/gtk.h>
#include "../pp.h"
#include "gtk.h"

GdkColor color_operator= {0x0000, 0xbbbb, 0xffff, 0xbbbb};
GdkColor color_voice   = {0x0000, 0xffff, 0xffff, 0xbbbb};
GdkColor color_normal  = {0x0000, 0xffff, 0xffff, 0xffff};

static gint
userlist_sortfunc(GtkCList *cl, const void *_row1, const void *_row2) {
  gchar *usermode1, *usermode2, *nick1, *nick2;
  GtkCListRow *row1, *row2;

  row1= (GtkCListRow*) _row1;
  row2= (GtkCListRow*) _row2;

  if (!cl || !row1 || !row2 || !row1->cell || !row2->cell) return 0;
  if (!GTK_CELL_TEXT(row1->cell[0]) || !GTK_CELL_TEXT(row2->cell[0])) return 0;
  if (!GTK_CELL_TEXT(row1->cell[1]) || !GTK_CELL_TEXT(row2->cell[1])) return 0;
 
  /*T‰‰ on pelottava*/
  usermode1= GTK_CELL_TEXT(row1->cell[0])->text;
  usermode2= GTK_CELL_TEXT(row2->cell[0])->text;
  nick1    = GTK_CELL_TEXT(row1->cell[1])->text;
  nick2    = GTK_CELL_TEXT(row2->cell[1])->text;

  if (!nick1 || !nick2 || !usermode1 || !usermode2) return 0;
  
  //  g_print("%s %s %s %s\n", usermode1, usermode2, nick1, nick2);

  if (usermode1 && usermode2) {

    if (*usermode1 == '@' && *usermode2 != '@') return -1;
    if (*usermode2 == '@' && *usermode1 != '@') return 1;
    if (*usermode1 == '+' && *usermode2 != '+') return -1;
    if (*usermode2 == '+' && *usermode1 != '+') return 1;
  }
  
  return g_strcasecmp(nick1, nick2);
}

static gint
userlist_title_update(GtkWidget *userlist) {
  char s[32];

  if (!userlist) return FALSE;

  snprintf(s, 30, "%d users", GTK_CLIST(userlist)->rows);
  gtk_clist_set_column_title(GTK_CLIST(userlist), 1, s);
  return TRUE;
}

static gint
userlist_popupcb(GtkWidget *widget, GdkEvent *event, GtkWidget *menu) {
  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton*)event;

    if (bevent->button==3) {
      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 
		     bevent->button, bevent->time);
      return TRUE;
    }
  }
  return FALSE;
}

static gint
userlist_selectrowcb(GtkWidget *clist, gint row, gint columns, GdkEvent *event,
		     gpointer data) {
  GtkWidget *plabel;
  gchar *s, s2[512];
  
  if (!gtk_clist_get_text(GTK_CLIST(clist), row, 1, &s)) return FALSE;

  plabel= (GtkWidget*) gtk_object_get_data(GTK_OBJECT(clist), "pnicklabel");
  if (plabel && s) {
    gtk_object_set_data(GTK_OBJECT(clist), "currow", GINT_TO_POINTER(row+1));
    snprintf(s2, 510, "Whois %s", s);
    gtk_label_set_text(GTK_LABEL(plabel), s2);
    return TRUE;
  }
  return FALSE;
}

static gint
userlist_menuactcb(GtkWidget *menu_item, GtkWidget *clist) {
  IRCSession *IRCs;
  char *channame, *nick;
  gint nitem, currow;

  nitem= GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(menu_item),"nitem"));
  IRCs=    (IRCSession*)(gtk_object_get_data(GTK_OBJECT(clist),    "IRCs"));
  channame=     (gchar*)(gtk_object_get_data(GTK_OBJECT(clist),    "channame"));
  currow=GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(clist),    "currow"));

  g_print("%s %d\n", channame, currow);
  if (!nitem || !IRCs || !channame || !currow) return FALSE;

  currow--;

  if (!gtk_clist_get_text(GTK_CLIST(clist), currow, 1, &nick)) return FALSE;

  switch(nitem) {
  case 1:
    Socket_outf(IRCs, g_strdup_printf("MODE %s +o %s\n", channame, nick), TRUE);
    break;
  case 2:
    Socket_outf(IRCs, g_strdup_printf("MODE %s -o %s\n", channame, nick), TRUE);
    break;
  case 3:
    Socket_outf(IRCs, g_strdup_printf("MODE %s +v %s\n", channame, nick), TRUE);
    break;
  case 4:
    Socket_outf(IRCs, g_strdup_printf("MODE %s -v %s\n", channame, nick), TRUE);
    break;
  case 5:
    Socket_outf(IRCs, g_strdup_printf("KICK %s %s\n", channame, nick), TRUE);
    break;
  case 16:
    Socket_outf(IRCs, g_strdup_printf("WHOIS %s\n", nick), TRUE);
    break;
  }
  return TRUE;
}

static gint
userlist_destroycb(GtkWidget *userlist, gpointer data) {
  char *channame;

  channame= (char*)gtk_object_get_data(GTK_OBJECT(userlist), "channame");
  if (channame) {
    g_free(channame);
  }
}

GtkWidget *
userlist_new(IRCSession *IRCs, char *channame) {
  GtkWidget *swin, *clist;

  swin= gtk_scrolled_window_new((GtkAdjustment*)NULL, (GtkAdjustment*)NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
				 GTK_POLICY_NEVER,
				 GTK_POLICY_AUTOMATIC);
  
  {
    gchar *titles[]= {" ", " "};
    clist= gtk_clist_new_with_titles(2, titles);
  }
  if (IRCs) gtk_object_set_data(GTK_OBJECT(clist), "IRCs", IRCs);
  if (channame) gtk_object_set_data(GTK_OBJECT(clist), "channame",
				    g_strdup(channame));

  gtk_clist_set_column_width(GTK_CLIST(clist), 0, 11);
  gtk_clist_set_column_width(GTK_CLIST(clist), 1, 80);

  gtk_container_add(GTK_CONTAINER(swin), clist);
  gtk_object_set_data(GTK_OBJECT(swin), "clist", clist);

  gtk_clist_set_auto_sort(GTK_CLIST(clist), TRUE);
  gtk_clist_set_sort_column(GTK_CLIST(clist), 1);
  gtk_clist_set_compare_func(GTK_CLIST(clist), userlist_sortfunc);
  //  gtk_clist_sort(GTK_CLIST(clist));

  {
    GtkWidget *menu, *menu_item;

    menu= gtk_menu_new();
    {
      GtkWidget *label;
      menu_item= gtk_menu_item_new();
      label= gtk_label_new("Whois");
      gtk_container_add(GTK_CONTAINER(menu_item), label);
      gtk_object_set_data(GTK_OBJECT(clist), "pnicklabel", label);
      gtk_menu_append(GTK_MENU(menu), menu_item);

      gtk_object_set_data(GTK_OBJECT(menu_item), "nitem",
			  GINT_TO_POINTER(16));
      gtk_signal_connect(GTK_OBJECT(menu_item), "activate", 
			 GTK_SIGNAL_FUNC(userlist_menuactcb), clist);	 
    }
    {
      struct mitem {
	char *s;
	int n;
      } mitems[] = {
	"Give Ops",   1,
	"Take Ops",   2,
	"-",          0,
	"Give Voice", 3,
	"Take Voice", 4,
	"-",          0,
	"Kick",       5,
	NULL,         0};
      int n=0;
      
      while (mitems[n].s) {
	if (*mitems[n].s=='-') {
	  menu_item= gtk_menu_item_new();
	}else {
	  menu_item= gtk_menu_item_new_with_label(mitems[n].s);
	}
	if (mitems[n].n) {
	  gtk_object_set_data(GTK_OBJECT(menu_item), "nitem",
			      GINT_TO_POINTER(mitems[n].n));
	  gtk_signal_connect(GTK_OBJECT(menu_item), "activate", 
			     GTK_SIGNAL_FUNC(userlist_menuactcb), clist);	 
	}
	gtk_menu_append(GTK_MENU(menu), menu_item);
	n++;
      }
    }
    gtk_widget_show_all(menu);
    gtk_signal_connect(GTK_OBJECT(clist), "button_press_event",
		       GTK_SIGNAL_FUNC(userlist_popupcb), menu);
    gtk_signal_connect(GTK_OBJECT(clist), "select-row",
		       GTK_SIGNAL_FUNC(userlist_selectrowcb), NULL);
    gtk_signal_connect(GTK_OBJECT(clist), "destroy", (GtkSignalFunc)userlist_destroycb, NULL);
  }

  return swin;
}

gint
userlist_exists(GtkWidget *ul, char *nick) {
  GtkWidget *userlist;
  gint i, length;

  if (!ul || !nick) return 0;
  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return 0;

  length= g_list_length(GTK_CLIST(userlist)->row_list);
  for (i=0;i<length;i++) {
    char *s;
    gtk_clist_get_text(GTK_CLIST(userlist), i, 1, &s);
    if (!g_strcasecmp(nick, s)) return 1;
  }
  return 0;
}

gint
userlist_getrow(GtkWidget *ul, char *nick) {
  GtkWidget *userlist;
  gint length, i;

  if (!ul || !nick) return -1;
  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return -1;

  length= g_list_length(GTK_CLIST(userlist)->row_list);
  for (i=0;i<length;i++) {
    char *s;
    gtk_clist_get_text(GTK_CLIST(userlist), i, 1, &s);
    if (!g_strcasecmp(nick, s)) return i;
  }
  return -1;
}

gint
userlist_add(GtkWidget *ul, char *nick, int usermode) {
  GtkWidget *userlist;
  if (!ul || !nick) return 0;

  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return 0;

  if (userlist_exists(ul, nick)) return 0;

  {
    gchar *s[2];
    s[0]= " ";
    s[1]= nick;
    gtk_clist_append(GTK_CLIST(userlist), s);
  }
  userlist_setmode(ul, nick, usermode);

  gtk_clist_sort(GTK_CLIST(userlist));

  return 1;
}

gint
userlist_getmode(GtkWidget *ul, char *nick) {
  GtkWidget *userlist;
  gint i;

  if (!ul || !nick) return -1;
  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return -1;

  i= userlist_getrow(ul, nick);
  if (i<0) return -1;

  {
    char *s;
    gtk_clist_get_text(GTK_CLIST(userlist), i, 0, &s);
    if (s){
      if (*s=='@') return USERMODE_OPERATOR;
      if (*s=='+') return USERMODE_VOICE;
    }
  }  
  return 0;
}

gint
userlist_setmode(GtkWidget *ul, char *nick, int mode) {
  GtkWidget *userlist;
  gint i;

  if (!ul || !nick) return 0;
  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return 0;

  i= userlist_getrow(ul, nick);
  if (i<0) return 0;

  g_print("usermode %d\n", mode);

  {
    gchar *s;

    if (mode& USERMODE_OPERATOR) {
      s= "@";
    } else if (mode& USERMODE_VOICE) {
      s= "+";
    }else {
      s= " ";
    }
    gtk_clist_set_text(GTK_CLIST(userlist), i, 0, s);
  }
  {
    int row;

    row= userlist_getrow(ul, nick);
    if (mode&USERMODE_OPERATOR) {
      gtk_clist_set_background(GTK_CLIST(userlist), row, &color_operator);
    } else if (mode&USERMODE_VOICE) {
      gtk_clist_set_background(GTK_CLIST(userlist), row, &color_voice);
    } else {
      gtk_clist_set_background(GTK_CLIST(userlist), row, &color_normal);
    }
  }

  gtk_clist_sort(GTK_CLIST(userlist));
  return TRUE;
}

gint
userlist_remove(GtkWidget *ul, char *nick) {
  GtkWidget *userlist;
  gint i;

  if (!ul || !nick) return 0;
  userlist= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(ul), "clist");
  if (!userlist) return 0;

  i= userlist_getrow(ul, nick);
  if (i<0) return 0;
  gtk_clist_remove(GTK_CLIST(userlist), i);
  return TRUE;
}

#ifdef TEST
gint
testicb(GtkWidget *cl) {
  userlist_remove(cl, "kikuli");
  userlist_add(cl, "kikuli", USERMODE_VOICE);
  return 1;
}

int
main(int argc, char **argv) {
  GtkWidget *w, *cl;

  gtk_init(&argc, &argv);

  w= gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(w), 120, 300);

  cl= userlist_new();
  gtk_container_add(GTK_CONTAINER(w), cl);  
  gtk_widget_show_all(w);

  userlist_add(cl, "lehma",0);
  userlist_add(cl, "kakkakasa",0);
  userlist_add(cl, "mutantti",0);
  userlist_add(cl, "ville",USERMODE_OPERATOR);
  userlist_add(cl, "munamies",0);
  userlist_add(cl, "raivoapina",0);
  userlist_add(cl, "pippeli",0);
  userlist_add(cl, "apina",USERMODE_VOICE);
  userlist_add(cl, "ApInA",USERMODE_VOICE);

  userlist_add(cl, "murhaaja",USERMODE_OPERATOR);

  gtk_timeout_add(10, testicb, cl);

  //  userlist_remove(cl, "apina");
  gtk_main();
}
#endif
