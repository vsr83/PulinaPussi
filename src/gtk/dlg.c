/*
 * dlg.c / PulinaPussi 0.12
 * written by Ville Räisänen <raivil@geek.com> 2001- 2003
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
#include "pp.xpm"

#define COPYRIGHTMSG "PulinaPussi 0.12 (c) Ville Räisänen 2001-2003"
#define PPHOMEURL    "http://www.sourceforge.net/projects/pulinapussi"

gint
aboutdlg_hide(GtkWidget *b, GtkWidget *about_window) {
  gtk_widget_hide(about_window);
  return TRUE;
}

GtkWidget *
aboutdlg_new() {
  GtkWidget *about_window, *main_vbox, *urllabel, *hsep, *closeb, *ctext;
  GtkWidget *pixmapw, *pixmap_frame;
  GtkStyle *style;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  about_window= gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect(GTK_OBJECT(about_window), "delete_event", 
                     (GtkSignalFunc)gtk_widget_hide, NULL);
  gtk_window_set_policy(GTK_WINDOW(about_window), FALSE, TRUE, FALSE);
  gtk_window_set_title(GTK_WINDOW(about_window), "About");
  gtk_widget_realize(about_window);

  main_vbox= gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(about_window), main_vbox);

  ctext= gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(ctext), COPYRIGHTMSG);
  gtk_entry_set_editable(GTK_ENTRY(ctext), FALSE);
  gtk_box_pack_start(GTK_BOX(main_vbox), ctext, FALSE, FALSE, 0);

  style= gtk_widget_get_style(about_window);
  pixmap= gdk_pixmap_create_from_xpm_d(about_window->window,
				       &mask, 
				       &style->bg[GTK_STATE_NORMAL],
				       (gchar **)pp_xpm);
  pixmapw= gtk_pixmap_new(pixmap, mask);
  pixmap_frame= gtk_frame_new(" ");
  gtk_container_add(GTK_CONTAINER(pixmap_frame), pixmapw);
  gtk_box_pack_start(GTK_BOX(main_vbox), pixmap_frame, FALSE, FALSE, 5);

  hsep= gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(main_vbox), hsep, FALSE, FALSE, 2);
  closeb= gtk_button_new_with_label("Close");
  gtk_box_pack_start(GTK_BOX(main_vbox), closeb, FALSE, FALSE, 0);
  urllabel= gtk_label_new(PPHOMEURL);
  gtk_box_pack_start(GTK_BOX(main_vbox), urllabel, FALSE, FALSE, 0);

  gtk_signal_connect(GTK_OBJECT(closeb), "clicked", 
		     (GtkSignalFunc)aboutdlg_hide, about_window);

  return about_window;
}

/*
int
main(int argc, char **argv) {
  GtkWidget *ad;

  gtk_init(&argc, &argv);

  ad= aboutdlg_new();
  gtk_widget_show_all(ad);

  gtk_main();
}
*/
