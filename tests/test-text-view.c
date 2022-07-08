/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <gspell/gspell.h>

#define TEST_TYPE_SPELL (test_spell_get_type ())
G_DECLARE_FINAL_TYPE (TestSpell, test_spell,
		      TEST, SPELL,
		      GtkGrid)

struct _TestSpell
{
	GtkGrid parent;

	GtkTextView *view;
};

G_DEFINE_TYPE (TestSpell, test_spell, GTK_TYPE_GRID)

static GspellChecker *
get_spell_checker (TestSpell *spell)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;

	gtk_buffer = gtk_text_view_get_buffer (spell->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);

	return gspell_text_buffer_get_spell_checker (gspell_buffer);
}

static void
test_spell_class_init (TestSpellClass *klass)
{
}

static void
checker_button_gesture_click_pressed_on ( GtkGestureClick* self,
					  gint n_press,
					  gdouble x,
					  gdouble y,
					  TestSpell *spell)
{
  GtkWidget * window;
  GtkWidget * checker_dialog;
  GspellNavigator * navigator;
  GtkWindow *parent = NULL;

  window = GTK_WIDGET (gtk_widget_get_root(GTK_WIDGET(spell)));
  if (GTK_IS_WINDOW(window))
  {
    parent = GTK_WINDOW (window);
  }

  navigator = gspell_navigator_text_view_new (spell->view);
  checker_dialog = gspell_checker_dialog_new (GTK_WINDOW (window), navigator);

   if (parent != NULL) {
	gtk_window_set_modal (GTK_WINDOW (checker_dialog),
			      gtk_window_get_modal (parent));
  }

  gtk_window_present (GTK_WINDOW ( checker_dialog ));
}

static void
change_buffer_button_clicked_cb (GtkGestureClick* self,
				gint n_press,
				gdouble x,
				gdouble y,
				TestSpell *spell)
{
	GtkTextBuffer *old_gtk_buffer;
	GtkTextBuffer *new_gtk_buffer;
	GspellTextBuffer *old_gspell_buffer;
	GspellTextBuffer *new_gspell_buffer;
	GspellChecker *checker;

	old_gtk_buffer = gtk_text_view_get_buffer (spell->view);
	old_gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (old_gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker (old_gspell_buffer);

	new_gtk_buffer = gtk_text_buffer_new (NULL);
	new_gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (new_gtk_buffer);
	gspell_text_buffer_set_spell_checker (new_gspell_buffer, checker);

	gtk_text_view_set_buffer (spell->view, new_gtk_buffer);
	g_object_unref (new_gtk_buffer);
}

static GtkWidget *
get_sidebar (TestSpell *spell)
{
	GtkWidget *sidebar;
	GtkWidget *checker_button;
	GtkWidget *language_button;
	GtkWidget *highlight_checkbutton;
	GtkWidget *change_buffer_button;
	GspellChecker *checker;
	const GspellLanguage *language;
	GspellTextView *gspell_view;
	GtkGesture * checker_button_gesture_click;
	GtkGesture * change_buffer_gesture_click;

	sidebar = gtk_grid_new();

	gtk_orientable_set_orientation (GTK_ORIENTABLE (sidebar),
					GTK_ORIENTATION_VERTICAL);

	gtk_grid_set_row_spacing (GTK_GRID (sidebar), 6);

	/* Button to launch a spell checker dialog */
	checker_button = gtk_button_new_with_mnemonic ("_Check Spelling…");

	checker_button_gesture_click = gtk_gesture_click_new ();

	g_signal_connect (G_OBJECT(checker_button_gesture_click), "pressed", G_CALLBACK(checker_button_gesture_click_pressed_on), spell);

	gtk_event_controller_set_name (GTK_EVENT_CONTROLLER (checker_button_gesture_click), "checker_button_gesture_click");

	gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (checker_button_gesture_click), GTK_PHASE_CAPTURE);

	gtk_widget_add_controller (checker_button, GTK_EVENT_CONTROLLER (checker_button_gesture_click));

	gtk_grid_attach (GTK_GRID (sidebar), checker_button, 0, 0, 1, 1);

	/* Button to launch a language dialog */
	checker = get_spell_checker (spell);
	language = gspell_checker_get_language (checker);
	language_button = gspell_language_chooser_button_new (language);
	gtk_grid_attach (GTK_GRID (sidebar), language_button, 0, 1, 1, 1);

	g_object_bind_property (language_button, "language",
				checker, "language",
				G_BINDING_BIDIRECTIONAL);

	/* Checkbutton to activate the inline spell checker */
	highlight_checkbutton = gtk_check_button_new_with_mnemonic ("_Highlight Misspelled Words");
	gtk_grid_attach (GTK_GRID (sidebar), highlight_checkbutton, 0, 2, 1, 1);

	gspell_view = gspell_text_view_get_from_gtk_text_view (spell->view);
	gspell_text_view_set_enable_language_menu (gspell_view, TRUE);

	g_object_bind_property (highlight_checkbutton, "active",
				gspell_view, "inline-spell-checking",
				G_BINDING_DEFAULT);

	gtk_check_button_set_active (GTK_CHECK_BUTTON (highlight_checkbutton), TRUE);

	/* Button to change the GtkTextBuffer */
	change_buffer_button = gtk_button_new_with_mnemonic ("Change _Buffer!");
	gtk_grid_attach (GTK_GRID (sidebar), change_buffer_button, 0, 3, 1, 1);

	change_buffer_gesture_click = gtk_gesture_click_new ();

	g_signal_connect (G_OBJECT(change_buffer_gesture_click), "pressed", G_CALLBACK(change_buffer_button_clicked_cb), spell);

	gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (change_buffer_gesture_click), GTK_PHASE_CAPTURE);

	gtk_widget_add_controller (change_buffer_button, GTK_EVENT_CONTROLLER (change_buffer_gesture_click));

	return sidebar;
}

static void
test_spell_init (TestSpell *spell)
{
	GtkWidget * scrolled_window;
	GtkTextBuffer * gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker * checker;

	spell->view = GTK_TEXT_VIEW (gtk_text_view_new ());
	gtk_buffer = gtk_text_view_get_buffer (spell->view);

	checker = gspell_enchant_checker_new (NULL);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (spell), GTK_ORIENTATION_HORIZONTAL);

	gtk_grid_set_row_spacing (GTK_GRID(spell), 6);
	gtk_grid_set_column_spacing (GTK_GRID(spell), 6);

	gtk_grid_attach (GTK_GRID(spell), get_sidebar (spell), 0, 0, 1, 1);

	scrolled_window = gtk_scrolled_window_new ();

	g_object_set (scrolled_window,
		      "vexpand", TRUE,
		      "hexpand", TRUE,
		      NULL);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET(spell->view));

	gtk_grid_attach (GTK_GRID(spell), scrolled_window, 1, 0, 1, 1);
}

static TestSpell *
test_spell_new (void)
{
	return g_object_new (TEST_TYPE_SPELL, NULL);
}

static void
print_available_language_codes (void)
{
	const GList *available_languages;
	const GList *l;

	g_print ("Available language codes: ");

	available_languages = gspell_language_get_available ();

	if (available_languages == NULL)
	{
		g_print ("none\n");
		return;
	}

	for (l = available_languages; l != NULL; l = l->next)
	{
		const GspellLanguage *language = l->data;
		g_print ("%s", gspell_language_get_code (language));

		if (l->next != NULL)
		{
			g_print (", ");
		}
	}

	g_print ("\n");
}

static void
activate(GtkApplication* app,
         gpointer user_data)
{
	  GtkWidget *window;
	  GtkWidget * titlebar;
	  GtkWidget * scrolled_window;
	  GtkTextView *gtk_view;
	  GspellTextView *gspell_view;
	  TestSpell *spell;

	  window = gtk_application_window_new(app);
	  titlebar = gtk_header_bar_new();

	  gtk_header_bar_set_title_widget (GTK_HEADER_BAR (titlebar), gtk_label_new("Gspell Text View"));

	  gtk_window_set_titlebar (GTK_WINDOW(window), titlebar);

	  spell = test_spell_new ();
	  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(spell));

	  gtk_widget_show(window);
}

gint
main (gint    argc,
      gchar **argv)
{
	GtkApplication *app;
  	int status;

	print_available_language_codes ();

	app = gtk_application_new("org.gnome.gspell.test-text-view", G_APPLICATION_FLAGS_NONE);

	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}

/* ex:set ts=8 noet: */


