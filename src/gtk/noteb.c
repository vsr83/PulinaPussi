/*
 * noteb.c / PulinaPussi 0.12
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

#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include "../pp.h"
#include "gtk.h"

#define DEFAULTFONT "-*-lucidatypewriter-medium-*-*-*-10-*-*-*-*-*-*-*"
#define TEXT_MAX_LENGTH 50000

GdkColor text_colors[]= {
  {0x0000, 0xcfcf, 0xcfcf, 0xcfcf},
  {0x0000, 0x0000, 0x0000, 0x0000},
  {0x0000, 0x0000, 0x0000, 0xcccc},
  {0x0000, 0x0000, 0xcccc, 0x0000},

  {0x0000, 0xdddd, 0x0000, 0x0000},
  {0x0000, 0xaaaa, 0x0000, 0x0000},
  {0x0000, 0xbbbb, 0x0000, 0xbbbb},
  {0x0000, 0xffff, 0xaaaa, 0x0000},

  {0x0000, 0xeeee, 0xdddd, 0x2222},
  {0x0000, 0x3333, 0xdede, 0x5555},
  {0x0000, 0x0000, 0xcccc, 0xcccc},
  {0x0000, 0x3333, 0xdddd, 0xeeee}, 

  {0x0000, 0x0000, 0x0000, 0xffff},
  {0x0000, 0xeeee, 0x2222, 0xeeee},
  {0x0000, 0x7777, 0x7777, 0x7777},
  {0x0000, 0x9999, 0x9999, 0x9999}
};

/*label-styles*/
static GtkStyle *lsnormal, *lschanged, *lsforme;

GdkColor lschangedcolor= {0xffff, 0xffff, 0x0000, 0x0000};
GdkColor lsformecolor  = {0xffff, 0x0000, 0x0000, 0xffff};

gint
notebook_pageswitch(GtkWidget *nb, gpointer data, gint n){
  notebook_setchanged(nb, n, NULL, 0);
  return TRUE;
}

GtkWidget *
notebook_new(IRCSession *IRCs) {
  GtkWidget *nb;
  GdkFont *font;
  int i;

  lsnormal = gtk_widget_get_default_style();
  lschanged= gtk_style_copy(lsnormal);
  lsforme  = gtk_style_copy(lsnormal);
  
  for (i=0;i<7;i++){
    lschanged->fg[i]= lschangedcolor;
    lsforme->fg[i]=   lsformecolor;
  }
  
  font= gdk_font_load(DEFAULTFONT);

  nb= gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(nb), GTK_POS_BOTTOM);
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(nb), TRUE);

  gtk_object_set_data(GTK_OBJECT(nb), "font", font);
  if (IRCs) gtk_object_set_data(GTK_OBJECT(nb), "IRCs", IRCs);

  gtk_signal_connect(GTK_OBJECT(nb), "switch_page", 
		     (GtkSignalFunc)notebook_pageswitch, NULL);

  return nb;
}

gint
notebook_getnthbyname(GtkWidget *nb, char *s) {
  int i, length;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (!g_strcasecmp(s, GTK_LABEL(curpage->tab_label)->label)) return i;
  }
  return -1;
}

gint
notebook_setchanged(GtkWidget *nb, int n, char *s, int isc) {
  GtkWidget *label=NULL;
  int i, length;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);

    if (curpage && s && (!g_strcasecmp(s, GTK_LABEL(curpage->tab_label)->label))) {
      label= (GtkWidget *)GTK_LABEL(curpage->tab_label);
    }
    if (curpage && !s && i==n) {
      label= (GtkWidget *)GTK_LABEL(curpage->tab_label);
    }
  }

  if (!label) return 0;

  if (isc) {
    gtk_widget_set_style(label, lschanged);
  }else {
    gtk_widget_set_style(label, lsnormal);
  }
  return 1;
}


gint
notebook_setfont(GtkWidget *nb, char *name) {
  GdkFont *font= NULL;

  font= (GdkFont*)gtk_object_get_data(GTK_OBJECT(nb), "font");
  if (font) gdk_font_unref(font);

  font= gdk_font_load(name);
  if (!font) return FALSE;
  return TRUE;
}

GtkWidget *
notebook_get_widget(GtkWidget *nb, char *s) {
  gint length, i;

  if (!s || !nb) return NULL;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && !g_strcasecmp(s, GTK_LABEL(curpage->tab_label)->label)) {
      return curpage->child;
    }
  }
  return NULL;
}

gint
notebook_page_exists(GtkWidget *nb, char *s) {
  gint length, i;

  if (!s || !nb) return 0;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && !g_strcasecmp(s, GTK_LABEL(curpage->tab_label)->label)) {
      return 1;
    }
  }
  return 0;
}

gint
notebook_add(GtkWidget *nb, char *s, int has_userlist) {
  /*
   * main_vbox + hbox3--+ topic
   *           + hbox --+ text
   *           |        + vscroll
   *           |        + userlist
   *           |
   *           + hbox2 -+ nicklabel
   *                    + entry
   * main_vbox, label -> notebook
   */

  GtkWidget *main_vbox, *label, *text, *vscroll, *hbox, *hbox2, *hbox3,
    *userlist, *entry, *nicklabel, *topic;
  IRCSession *IRCs;

  if (!s || !nb) return 0;

  if (notebook_page_exists(nb, s)) return 0;

  label= gtk_label_new(s);

  main_vbox= gtk_vbox_new(FALSE, 0);
  hbox3= gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), hbox3, FALSE, FALSE, 0);

  if (has_userlist) {
    topic= gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(topic), FALSE);
    //  gtk_label_set_justify(GTK_LABEL(topic), GTK_JUSTIFY_RIGHT);
    gtk_box_pack_start(GTK_BOX(hbox3), topic, TRUE, TRUE, 0);
    gtk_object_set_data(GTK_OBJECT(main_vbox), "topic", topic);
  }

  hbox= gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), hbox, TRUE, TRUE, 0);

  hbox2= gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), hbox2, FALSE, FALSE, 0);

  nicklabel= gtk_label_new("nickname");
  gtk_box_pack_start(GTK_BOX(hbox2), nicklabel, FALSE, FALSE, 5);

  entry= gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox2), entry, TRUE, TRUE, 0);
  //  gtk_signal_connect(GTK_OBJECT(entry), "key_press_event", gtk_main_quit, NULL);

  text= gtk_text_new(NULL, NULL);

  vscroll= gtk_vscrollbar_new(GTK_TEXT(text)->vadj);
  gtk_box_pack_start(GTK_BOX(hbox), text, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vscroll, FALSE, FALSE, 0);


  IRCs= (IRCSession*)gtk_object_get_data(GTK_OBJECT(nb), "IRCs");
  if (has_userlist && IRCs){
    userlist= userlist_new(IRCs, s);
    gtk_widget_set_usize(userlist, 90, 100);
    gtk_box_pack_start(GTK_BOX(hbox), userlist, FALSE, TRUE, 0);
    gtk_object_set_data(GTK_OBJECT(main_vbox), "userlist", userlist);
  }

  gtk_object_set_data(GTK_OBJECT(main_vbox), "text", text);
  gtk_object_set_data(GTK_OBJECT(main_vbox), "entry", entry);
  gtk_object_set_data(GTK_OBJECT(main_vbox), "nicklabel", nicklabel);

  gtk_notebook_append_page(GTK_NOTEBOOK(nb), main_vbox, label);
  gtk_widget_show_all(main_vbox);

  /*hmm?*/
  notebook_set_style(nb, &text_colors[0], &text_colors[1]);

  return 1;
}

gint
notebook_remove(GtkWidget *nb, char *s) {
  gint length, i;
  if (!s || !nb) return 0;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && !g_strcasecmp(s, GTK_LABEL(curpage->tab_label)->label)) {
      gtk_notebook_remove_page(GTK_NOTEBOOK(nb), i);
      return 1;
    }
  }
  return 0;
}

gint
notebook_msg(GtkWidget *nb, char *s, char *msg) {
  GtkWidget *nw, *text;
  GdkFont *font;
  IRCSession *IRCs;
  int bufmaxlen;

  int i, has_color=FALSE, curpage;

  if (!nb || !s || !msg) return 0;

  IRCs= (IRCSession*)gtk_object_get_data(GTK_OBJECT(nb), "IRCs");
  
  curpage= gtk_notebook_get_current_page(GTK_NOTEBOOK(nb));
  if (curpage != notebook_getnthbyname(nb,  s)) notebook_setchanged(nb, -1, s, 1);

  nw= notebook_get_widget(nb, s);
  if (!nw) return 0;

  text= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(nw), "text");
  if (!text) return FALSE;

  bufmaxlen= TEXT_MAX_LENGTH;
  if (IRCs) {
    int vtype;
    void *rval;

    rval= PPopt_getvalue(IRCs->optlist, "buffer_maxlength", &vtype);
    if (vtype==OPTTYPE_LONG && rval) bufmaxlen= (long)rval;
  }
  if (bufmaxlen<1000) bufmaxlen=1000;

  if (gtk_text_get_length(GTK_TEXT(text)) > bufmaxlen) {
    int len= gtk_text_get_length(GTK_TEXT(text));

    g_print("length %d\n", len);

    gtk_text_freeze(GTK_TEXT(text));
    gtk_text_set_point(GTK_TEXT(text), 0);
    gtk_text_forward_delete(GTK_TEXT(text), len-bufmaxlen);
    gtk_text_set_point(GTK_TEXT(text), gtk_text_get_length(GTK_TEXT(text)));
    gtk_text_thaw(GTK_TEXT(text));
  }

  font= (GdkFont*)gtk_object_get_data(GTK_OBJECT(nb), "font");

  /*\04 is character for mIRC-bgcolor?*/
  for (i=0;i<strlen(msg);i++) {
    if (msg[i]=='\03') has_color=TRUE;
  }

  if (IRCs && !PPopt_getvalue(IRCs->optlist, "show_colors", NULL)) {
    has_color= FALSE;
  }

  /*mIRC-colors :(*/
  /*Colored shit is drawn very very slowly.*/
  if (has_color) {
    GdkColor *foreground= NULL, *background= NULL;
    int i, msglen;

    msglen= Strlen(msg);

    for (i=0;i<msglen;i++) {
      if (msg[i]=='\03' && msg[i+1]){
	int color=0;
	/*atoi could be nice*/
	if (isdigit(msg[i+1]) && !isdigit(msg[i+2])){ 
	  color= msg[++i]-'0';
	}
	else if (isdigit(msg[i+1]) && isdigit(msg[i+2])) {
	  color= (msg[++i]-'0')*10 + msg[++i]-'0';
	}

	color= color%16;

	foreground= (GdkColor*)NULL;
	if (color>0 && color<16) {
	  foreground= &text_colors[color];
	}
      }else {
	gtk_text_insert(GTK_TEXT(text), font, foreground, background, msg+i, 1);
      }
    }
  }
  else {/*NOCOLOR*/
    gtk_text_insert(GTK_TEXT(text), font, (GdkColor*)NULL, (GdkColor*)NULL, 
		    msg, strlen(msg));
  }

  return TRUE;
}

//notebook_msg(GtkWidget *nb, char *s, char *msg) {
gint
notebook_msgall(GtkWidget *nb, char *msg) {
  gint length, i;
  if (!nb || !msg) return FALSE;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && GTK_LABEL(curpage->tab_label)->label) {
      notebook_msg(nb, GTK_LABEL(curpage->tab_label)->label, msg);
    }
  }

  return TRUE;
}

gint
notebook_setnicklabel(GtkWidget *nb, char *nick) {
  gint length, i;
  if (!nb || !nick) return FALSE;

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    if (curpage && GTK_LABEL(curpage->tab_label)->label) {
      GtkWidget *hb=(GtkWidget*)NULL, *label=(GtkWidget*)NULL;
      hb= notebook_get_widget(nb, GTK_LABEL(curpage->tab_label)->label);
      if (hb) label= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(hb), "nicklabel");
      if (label) gtk_label_set_text(GTK_LABEL(label), nick); 
    }
  }

  return TRUE;
}

GtkWidget *
notebook_userlist(GtkWidget *nb, char *s) {
  GtkWidget *hb, *ul;

  if ((hb= (GtkWidget*)notebook_get_widget(nb, s))==NULL) 
    return (GtkWidget*)NULL;
  if ((ul= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(hb), "userlist"))==NULL) 
    return (GtkWidget*)NULL;

  return ul;
}

GtkWidget *
notebook_entry(GtkWidget *nb, char *s) {
  GtkWidget *hb, *ul;

  if ((hb= (GtkWidget*)notebook_get_widget(nb, s))==NULL) 
    return (GtkWidget*)NULL;
  if ((ul= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(hb), "entry"))==NULL) 
    return (GtkWidget*)NULL;

  return ul;
}

GtkWidget *
notebook_topic(GtkWidget *nb, char *s) {
  GtkWidget *hb, *ul;

  if ((hb= notebook_get_widget(nb, s))==NULL) return (GtkWidget*)NULL;
  if ((ul= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(hb), "topic"))==NULL) 
    return (GtkWidget*)NULL;

  return ul;
}

gint
notebook_settopic(GtkWidget *nb, char *tabs, char *s) {
  GtkWidget *topic;

  if (!nb || !tabs || !s) return FALSE;

  topic= notebook_topic(nb, tabs);
  if (topic) {
    gtk_entry_set_text(GTK_ENTRY(topic), s);
    return TRUE;
  }
  return FALSE;
}

gint
notebook_clear(GtkWidget *nb, char *s) {
  GtkWidget *hb, *text;

  if ((hb=(GtkWidget*)notebook_get_widget(nb, s))==NULL) return FALSE;
  if ((text=(GtkWidget*)gtk_object_get_data(GTK_OBJECT(hb), "text"))==NULL) 
    return FALSE;

  gtk_text_backward_delete(GTK_TEXT(text), gtk_text_get_length(GTK_TEXT(text)));
  return TRUE;
}

gint
notebook_set_style(GtkWidget *nb, GdkColor *foreground, GdkColor *background) {
  GtkStyle *current, *newstyle;
  int length, i;

  if (!nb) return 0;

  current= gtk_widget_get_default_style();
  newstyle= gtk_style_copy(current);

  if (background) {
    newstyle->base[GTK_STATE_NORMAL]= *background;
  }
  if (foreground) {
    newstyle->text[GTK_STATE_NORMAL]= *foreground;
  }

  length=  g_list_length(GTK_NOTEBOOK(nb)->children);

  for (i=0;i<length;i++) {
    GtkNotebookPage *curpage= (GtkNotebookPage*)g_list_nth_data(GTK_NOTEBOOK(nb)->children, i);
    GtkWidget *child;

    child= curpage->child;

    if (child) {
      GtkWidget *text= (GtkWidget*)gtk_object_get_data(GTK_OBJECT(child), "text");
      if (text) gtk_widget_set_style(text, newstyle);
    }
  }
  gtk_style_unref(newstyle);

  return TRUE;
}

#ifdef TEST

vammacb(GtkWidget *nb) {
  static int s=0;
  g_print("%d\n", s++);

  notebook_remove(nb, "apina2");
  notebook_add(nb, "ApInA2", 1);
  notebook_msg(nb, "apina2", "outoa, eikö totta?\n");
  return 1;
}

int
main(int argc, char **argv) {
  GtkWidget *w, *nb;

  gtk_init(&argc, &argv);

  w= gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(w), 550, 250);
  nb= notebook_new();
  gtk_container_add(GTK_CONTAINER(w), nb);

  notebook_add(nb, "[main]", FALSE);
  notebook_add(nb, "#pippeli", TRUE);

  userlist_add(notebook_userlist(nb, "#pippeli"), "spuuki", 0); 
  notebook_msg(nb, "[main]", "PulinaPussi 0.11 (c) Ville Räisänen 2001, 2002\n");
  notebook_msg(nb, "[main]", "http://www.sourceforge.net/projects/pulinapussi\n\n");

  notebook_msg(nb, "#pippeli", "\03\03raivolla\03 testitesti");

  gtk_widget_show_all(w);

  //  for (;;)
  {
    GdkColor black= {0,0,0,0x0000};
    GdkColor white= {0, 0xcfcf, 0xcfcf, 0xcfcf};
    notebook_set_style(nb, &white, &black);
  }

  gtk_timeout_add(10, vammacb, nb);
  gtk_main();
}

#endif
