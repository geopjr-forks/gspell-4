/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
 * Copyright 2022 - otrocodigo
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gspell-checker-dialog.h"
#include <glib/gi18n-lib.h>
#include "gspell-checker.h"

/**
 * SECTION:checker-dialog
 * @Short_description: Spell checker dialog
 * @Title: GspellCheckerDialog
 * @See_also: #GspellNavigator
 *
 * #GspellCheckerDialog is a #GtkDialog to spell check a document one word
 * at a time. It uses a #GspellNavigator.
 */

typedef struct _GspellCheckerDialogPrivate GspellCheckerDialogPrivate;

struct _GspellCheckerDialogPrivate
{
	GspellNavigator *navigator;
	GspellChecker *checker;

	gchar *misspelled_word;

	GtkLabel *misspelled_word_label;
	GtkEntry *word_entry;
	GtkWidget *check_word_button;
	GtkWidget *ignore_button;
	GtkWidget *ignore_all_button;
	GtkWidget *change_button;
	GtkWidget *change_all_button;
	GtkWidget *add_word_button;
	GtkTreeView *suggestions_view;

	GtkLabel * header_bar_subtitle;

	guint initialized : 1;
};

enum
{
	PROP_0,
	PROP_SPELL_NAVIGATOR,
};

enum
{
	COLUMN_SUGGESTION,
	N_COLUMNS
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellCheckerDialog, gspell_checker_dialog, GTK_TYPE_DIALOG)

static void
set_spell_checker (GspellCheckerDialog *dialog,
		   GspellChecker       *checker)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	if (g_set_object (&priv->checker, checker))
	{
		const GspellLanguage *lang;

		lang = gspell_checker_get_language (checker);
		gtk_label_set_label (priv->header_bar_subtitle, gspell_language_get_name (lang));
		gtk_widget_set_visible (GTK_WIDGET(priv->header_bar_subtitle), TRUE);
	}
}

static void
set_navigator (GspellCheckerDialog *dialog,
	       GspellNavigator     *navigator)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	g_return_if_fail (priv->navigator == NULL);
	priv->navigator = g_object_ref_sink (navigator);

	g_object_notify (G_OBJECT (dialog), "spell-navigator");
}

static void
clear_suggestions (GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	GtkListStore *store;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->suggestions_view));
	gtk_list_store_clear (store);

	gtk_tree_view_columns_autosize (priv->suggestions_view);
}

static void
set_suggestions (GspellCheckerDialog *dialog,
		 GSList              *suggestions)
{
	GspellCheckerDialogPrivate *priv;
	GtkListStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	const gchar *first_suggestion;
	GSList *l;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	clear_suggestions (dialog);

	store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->suggestions_view));

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);

	if (suggestions == NULL)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    /* Translators: Displayed in the "Check Spelling"
				     * dialog if there are no suggestions for the current
				     * misspelled word.
				     */
				    COLUMN_SUGGESTION, _("(no suggested words)"),
				    -1);



		gtk_entry_buffer_set_text (entry_buffer, "", -1);

		gtk_widget_set_sensitive (GTK_WIDGET (priv->suggestions_view), FALSE);
		return;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (priv->suggestions_view), TRUE);

	first_suggestion = suggestions->data;
	gtk_entry_buffer_set_text (entry_buffer, first_suggestion, -1);

	for (l = suggestions; l != NULL; l = l->next)
	{
		const gchar *suggestion = l->data;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_SUGGESTION, suggestion,
				    -1);
	}

	selection = gtk_tree_view_get_selection (priv->suggestions_view);
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
	gtk_tree_selection_select_iter (selection, &iter);
}

static void
set_misspelled_word (GspellCheckerDialog *dialog,
		     const gchar         *word)
{
	GspellCheckerDialogPrivate *priv;
	gchar *label;
	GSList *suggestions;

	g_assert (word != NULL);

	priv = gspell_checker_dialog_get_instance_private (dialog);

	g_return_if_fail (!gspell_checker_check_word (priv->checker, word, -1, NULL));

	g_free (priv->misspelled_word);
	priv->misspelled_word = g_strdup (word);

	label = g_strdup_printf("<b>%s</b>", word);
	gtk_label_set_markup (priv->misspelled_word_label, label);
	g_free (label);

	suggestions = gspell_checker_get_suggestions (priv->checker, priv->misspelled_word, -1);

	set_suggestions (dialog, suggestions);

	g_slist_free_full (suggestions, g_free);
}

static void
set_completed (GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);



	clear_suggestions (dialog);
	gtk_entry_buffer_set_text (entry_buffer, "", -1);

	gtk_widget_set_sensitive (GTK_WIDGET (priv->word_entry), FALSE);
	gtk_widget_set_sensitive (priv->check_word_button, FALSE);
	gtk_widget_set_sensitive (priv->ignore_button, FALSE);
	gtk_widget_set_sensitive (priv->ignore_all_button, FALSE);
	gtk_widget_set_sensitive (priv->change_button, FALSE);
	gtk_widget_set_sensitive (priv->change_all_button, FALSE);
	gtk_widget_set_sensitive (priv->add_word_button, FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (priv->suggestions_view), FALSE);
}

static void
show_error (GspellCheckerDialog *dialog,
	    GError              *error)
{
	GspellCheckerDialogPrivate *priv;
	gchar *label;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	label = g_strdup_printf ("<b>%s</b> %s", _("Error:"), error->message);
	gtk_label_set_markup (priv->misspelled_word_label, label);
	g_free (label);

	set_completed (dialog);
}

static void
goto_next (GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	gchar *word = NULL;
	GspellChecker *checker = NULL;
	GError *error = NULL;
	gboolean found;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	found = gspell_navigator_goto_next (priv->navigator, &word, &checker, &error);

	if (error != NULL)
	{
		show_error (dialog, error);
		g_clear_error (&error);
	}
	else if (found)
	{
		set_spell_checker (dialog, checker);
		set_misspelled_word (dialog, word);
	}
	else
	{
		gchar *label;

		if (priv->initialized)
		{
			label = g_strdup_printf ("<b>%s</b>", _("Completed spell checking"));
		}
		else
		{
			label = g_strdup_printf ("<b>%s</b>", _("No misspelled words"));
		}

		gtk_label_set_markup (priv->misspelled_word_label, label);
		g_free (label);

		set_completed (dialog);
	}

	priv->initialized = TRUE;

	g_free (word);
	g_clear_object (&checker);
}

static void
gspell_checker_dialog_get_property (GObject    *object,
				    guint       prop_id,
				    GValue     *value,
				    GParamSpec *pspec)
{
	GspellCheckerDialog *dialog = GSPELL_CHECKER_DIALOG (object);

	switch (prop_id)
	{
		case PROP_SPELL_NAVIGATOR:
			g_value_set_object (value, gspell_checker_dialog_get_spell_navigator (dialog));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_dialog_set_property (GObject      *object,
				    guint         prop_id,
				    const GValue *value,
				    GParamSpec   *pspec)
{
	GspellCheckerDialog *dialog = GSPELL_CHECKER_DIALOG (object);

	switch (prop_id)
	{
		case PROP_SPELL_NAVIGATOR:
			set_navigator (dialog, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_dialog_dispose (GObject *object)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (GSPELL_CHECKER_DIALOG (object));

	g_clear_object (&priv->navigator);
	g_clear_object (&priv->checker);

	G_OBJECT_CLASS (gspell_checker_dialog_parent_class)->dispose (object);
}

static void
gspell_checker_dialog_finalize (GObject *object)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (GSPELL_CHECKER_DIALOG (object));

	g_free (priv->misspelled_word);

	G_OBJECT_CLASS (gspell_checker_dialog_parent_class)->finalize (object);
}

static void
gspell_checker_dialog_show (GtkWidget *widget)
{
	GspellCheckerDialog *dialog = GSPELL_CHECKER_DIALOG (widget);
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	/* Chain-up */
	if (GTK_WIDGET_CLASS (gspell_checker_dialog_parent_class)->show != NULL)
	{
		GTK_WIDGET_CLASS (gspell_checker_dialog_parent_class)->show (widget);
	}

	/* A typical implementation of a SpellNavigator is to select the
	 * misspelled word when goto_next() is called. Showing the dialog makes
	 * a focus change, which can unselect the buffer selection (e.g. in a
	 * GtkTextBuffer). So that's why goto_next() is called after the
	 * chain-up.
	 */
	if (!priv->initialized)
	{
		goto_next (dialog);
	}
}

static void
gspell_checker_dialog_class_init (GspellCheckerDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gspell_checker_dialog_get_property;
	object_class->set_property = gspell_checker_dialog_set_property;
	object_class->dispose = gspell_checker_dialog_dispose;
	object_class->finalize = gspell_checker_dialog_finalize;

	widget_class->show = gspell_checker_dialog_show;

	/**
	 * GspellCheckerDialog:spell-navigator:
	 *
	 * The #GspellNavigator to use.
	 */
	g_object_class_install_property (object_class,
					 PROP_SPELL_NAVIGATOR,
					 g_param_spec_object ("spell-navigator",
							      "Spell Navigator",
							      "",
							      GSPELL_TYPE_NAVIGATOR,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/gspell/checker-dialog.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, misspelled_word_label);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, word_entry);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, check_word_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, ignore_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, ignore_all_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, change_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, change_all_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, add_word_button);
	gtk_widget_class_bind_template_child_private (widget_class, GspellCheckerDialog, suggestions_view);
}

static void
word_entry_changed_handler (GtkEntry            *word_entry,
			    GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	gboolean sensitive;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	sensitive = gtk_entry_get_text_length (word_entry) > 0;

	gtk_widget_set_sensitive (priv->check_word_button, sensitive);
	gtk_widget_set_sensitive (priv->change_button, sensitive);
	gtk_widget_set_sensitive (priv->change_all_button, sensitive);
}

static void
suggestions_selection_changed_handler (GtkTreeSelection    *selection,
				       GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *text;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COLUMN_SUGGESTION, &text,
			    -1);

	gtk_entry_buffer_set_text (entry_buffer, text, -1);

	g_free (text);
}

static void
check_word_button_clicked_handler (GtkButton           *button,
				   GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	const gchar *word;
	gboolean correctly_spelled;
	GError *error = NULL;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);

	g_return_if_fail (gtk_entry_get_text_length (priv->word_entry) > 0);

	word = gtk_entry_buffer_get_text (entry_buffer);

	correctly_spelled = gspell_checker_check_word (priv->checker, word, -1, &error);

	if (error != NULL)
	{
		show_error (dialog, error);
		g_error_free (error);
		return;
	}

	if (correctly_spelled)
	{
		GtkListStore *store;
		GtkTreeIter iter;

		clear_suggestions (dialog);

		store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->suggestions_view));

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
		                    /* Translators: Displayed in the "Check
				     * Spelling" dialog if the current word
				     * isn't misspelled.
				     */
				    COLUMN_SUGGESTION, _("(correct spelling)"),
				    -1);

		gtk_widget_set_sensitive (GTK_WIDGET (priv->suggestions_view), FALSE);
	}
	else
	{
		GSList *suggestions;

		suggestions = gspell_checker_get_suggestions (priv->checker, word, -1);

		set_suggestions (dialog, suggestions);

		g_slist_free_full (suggestions, g_free);
	}
}

static void
add_word_button_clicked_handler (GtkButton           *button,
				 GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	g_return_if_fail (priv->misspelled_word != NULL);

	gspell_checker_add_word_to_personal (priv->checker, priv->misspelled_word, -1);

	goto_next (dialog);
}

static void
ignore_button_clicked_handler (GtkButton           *button,
			       GspellCheckerDialog *dialog)
{
	goto_next (dialog);
}

static void
ignore_all_button_clicked_handler (GtkButton           *button,
				   GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	g_return_if_fail (priv->misspelled_word != NULL);

	gspell_checker_add_word_to_session (priv->checker, priv->misspelled_word, -1);

	goto_next (dialog);
}

static void
change_button_clicked_handler (GtkButton           *button,
			       GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	const gchar *entry_text;
	gchar *change_to;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);

	g_return_if_fail (priv->misspelled_word != NULL);

	entry_text = gtk_entry_buffer_get_text (entry_buffer);
	g_return_if_fail (entry_text != NULL);
	g_return_if_fail (entry_text[0] != '\0');

	change_to = g_strdup (entry_text);
	gspell_checker_set_correction (priv->checker,
				       priv->misspelled_word, -1,
				       change_to, -1);

	gspell_navigator_change (priv->navigator, priv->misspelled_word, change_to);
	g_free (change_to);

	goto_next (dialog);
}

/* double click on one of the suggestions is like clicking on "change" */
static void
suggestions_row_activated_handler (GtkTreeView         *view,
				   GtkTreePath         *path,
				   GtkTreeViewColumn   *column,
				   GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	change_button_clicked_handler (GTK_BUTTON (priv->change_button), dialog);
}

static void
change_all_button_clicked_handler (GtkButton           *button,
				   GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	const gchar *entry_text;
	gchar *change_to;
	GtkEntryBuffer * entry_buffer;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	entry_buffer = gtk_entry_get_buffer (priv->word_entry);

	g_return_if_fail (priv->misspelled_word != NULL);

	entry_text = gtk_entry_buffer_get_text (entry_buffer);
	g_return_if_fail (entry_text != NULL);
	g_return_if_fail (entry_text[0] != '\0');

	change_to = g_strdup (entry_text);
	gspell_checker_set_correction (priv->checker,
				       priv->misspelled_word, -1,
				       change_to, -1);

	gspell_navigator_change_all (priv->navigator, priv->misspelled_word, change_to);
	g_free (change_to);

	goto_next (dialog);
}

static void
gspell_checker_dialog_init (GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkTreeSelection *selection;

	priv = gspell_checker_dialog_get_instance_private (dialog);

	gtk_widget_init_template (GTK_WIDGET (dialog));

	/* Suggestion list */
	store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model (priv->suggestions_view, GTK_TREE_MODEL (store));
	g_object_unref (store);

	/* Add the suggestions column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Suggestions"), cell,
							   "text", COLUMN_SUGGESTION,
							   NULL);

	gtk_tree_view_append_column (priv->suggestions_view, column);

	gtk_tree_view_set_search_column (priv->suggestions_view, COLUMN_SUGGESTION);

	selection = gtk_tree_view_get_selection (priv->suggestions_view);

	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	GtkHeaderBar *header_bar;
	GtkWidget * title;

	header_bar = GTK_HEADER_BAR (gtk_dialog_get_header_bar (GTK_DIALOG (dialog)));

	title = gtk_label_new(_("Check Spelling"));
	gtk_label_set_single_line_mode (GTK_LABEL(title), true);
	gtk_label_set_ellipsize (GTK_LABEL(title), PANGO_ELLIPSIZE_END);
	gtk_label_set_width_chars (GTK_LABEL(title), 5);
	gtk_widget_add_css_class (title, "title");

	priv->header_bar_subtitle = GTK_LABEL (gtk_label_new(NULL));
	gtk_label_set_single_line_mode (priv->header_bar_subtitle, true);
	gtk_label_set_ellipsize (priv->header_bar_subtitle, PANGO_ELLIPSIZE_END);
	gtk_label_set_width_chars (priv->header_bar_subtitle, 5);
	gtk_widget_add_css_class (GTK_WIDGET (priv->header_bar_subtitle), "subtitle");
	gtk_widget_set_visible (GTK_WIDGET (priv->header_bar_subtitle), FALSE);

	GtkWidget * box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
	gtk_box_append(GTK_BOX(box), title);
       	gtk_box_append(GTK_BOX(box), GTK_WIDGET (priv->header_bar_subtitle));

	gtk_header_bar_set_title_widget (header_bar, box);

	/* Connect signals */
	g_signal_connect (priv->word_entry,
			  "changed",
			  G_CALLBACK (word_entry_changed_handler),
			  dialog);

	g_signal_connect_object (selection,
				 "changed",
				 G_CALLBACK (suggestions_selection_changed_handler),
				 dialog,
				 0);

	g_signal_connect (priv->check_word_button,
			  "clicked",
			  G_CALLBACK (check_word_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->add_word_button,
			  "clicked",
			  G_CALLBACK (add_word_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->ignore_button,
			  "clicked",
			  G_CALLBACK (ignore_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->ignore_all_button,
			  "clicked",
			  G_CALLBACK (ignore_all_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->change_button,
			  "clicked",
			  G_CALLBACK (change_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->change_all_button,
			  "clicked",
			  G_CALLBACK (change_all_button_clicked_handler),
			  dialog);

	g_signal_connect (priv->suggestions_view,
			  "row-activated",
			  G_CALLBACK (suggestions_row_activated_handler),
			  dialog);

	gtk_window_set_default_widget(GTK_WINDOW(dialog), priv->change_button);
}

/**
 * gspell_checker_dialog_new:
 * @parent: transient parent of the dialog.
 * @navigator: the #GspellNavigator to use.
 *
 * Returns: a new #GspellCheckerDialog widget.
 */
GtkWidget *
gspell_checker_dialog_new (GtkWindow       *parent,
			   GspellNavigator *navigator)
{
	g_return_val_if_fail (GTK_IS_WINDOW (parent), NULL);
	g_return_val_if_fail (GSPELL_IS_NAVIGATOR (navigator), NULL);

	return g_object_new (GSPELL_TYPE_CHECKER_DIALOG,
			     "transient-for", parent,
			     "use-header-bar", TRUE,
			     "spell-navigator", navigator,
			     NULL);
}

/**
 * gspell_checker_dialog_get_spell_navigator:
 * @dialog: a #GspellCheckerDialog.
 *
 * Returns: (transfer none): the #GspellNavigator used.
 */
GspellNavigator *
gspell_checker_dialog_get_spell_navigator (GspellCheckerDialog *dialog)
{
	GspellCheckerDialogPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_CHECKER_DIALOG (dialog), NULL);

	priv = gspell_checker_dialog_get_instance_private (dialog);
	return priv->navigator;
}

/* ex:set ts=8 noet: */

