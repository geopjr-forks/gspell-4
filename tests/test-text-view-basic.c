/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <gspell/gspell.h>

static void
activate(GtkApplication* app,
         gpointer user_data)
{
	GtkWidget *window;
	GtkWidget * titlebar;
	GtkWidget * scrolled_window;
	GtkTextView *gtk_view;
	GspellTextView *gspell_view;

	window = gtk_application_window_new(app);
	titlebar = gtk_header_bar_new();

	gtk_header_bar_set_title_widget (GTK_HEADER_BAR (titlebar), gtk_label_new("Gspell Text View Basic"));


	gtk_window_set_titlebar (GTK_WINDOW(window), titlebar);

	gtk_view = GTK_TEXT_VIEW (gtk_text_view_new ());
	gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
	gspell_text_view_basic_setup (gspell_view);

	scrolled_window = gtk_scrolled_window_new ();
	gtk_widget_set_vexpand (scrolled_window, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET (gtk_view));


	gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(scrolled_window));

	gtk_widget_show(window);
}

int
main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	app = gtk_application_new("org.gnome.gspell.test-text-view-basic", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}

/* ex:set ts=8 noet: */
