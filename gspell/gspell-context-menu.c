/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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

#include "gspell-context-menu.h"
#include <glib/gi18n-lib.h>

#define LANGUAGE_DATA_KEY "gspell-language-data-key"
#define SUGGESTION_DATA_KEY "gspell-suggestion-data-key"

typedef struct _LanguageData	LanguageData;
typedef struct _SuggestionData	SuggestionData;

struct _LanguageData
{
	const GspellLanguage *lang;
	GspellLanguageActivatedCallback callback;
	gpointer user_data;
};

struct _SuggestionData
{
	GspellChecker *checker;
	gchar *misspelled_word;

	gchar *suggested_word;
	GspellSuggestionActivatedCallback callback;
	gpointer user_data;
};

static void
suggestion_data_free (gpointer data)
{
	SuggestionData *suggestion_data = data;

	if (suggestion_data != NULL)
	{
		g_clear_object (&suggestion_data->checker);
		g_free (suggestion_data->misspelled_word);
		g_free (suggestion_data->suggested_word);
		g_free (suggestion_data);
	}
}

static void
activate_language_cb (GtkWidget *menu_item)
{
	LanguageData *data;

	data = g_object_get_data (G_OBJECT (menu_item), LANGUAGE_DATA_KEY);
	g_return_if_fail (data != NULL);

	if (data->callback != NULL)
	{
		data->callback (data->lang, data->user_data);
	}
}

static GMenu *
get_language_menu (const GspellLanguage            *current_language,
		   GspellLanguageActivatedCallback  callback,
		   gpointer                         user_data)
{
	GMenu * menu;
	const GList *languages;
	const GList *l;
	menu = g_menu_new ();

	languages = gspell_language_get_available ();
	for (l = languages; l != NULL; l = l->next)
	{
		const GspellLanguage *lang = l->data;
		const gchar *lang_name;
		const gchar *code;
		GMenuItem *menu_item;

		code = gspell_language_get_code (lang);
		menu_item = g_menu_item_new (lang_name, NULL);
		g_menu_item_set_action_and_target (menu_item, "spelling.language", "s", code);
		g_menu_append_item (menu, menu_item);
	}

	return menu;
}

GMenuItem *
_gspell_context_menu_get_language_menu_item (const GspellLanguage            *current_language,
					     GspellLanguageActivatedCallback  callback,
					     gpointer                         user_data)
{
	GMenu *lang_menu;
	GMenuItem *menu_item;

	lang_menu = get_language_menu (current_language, callback, user_data);

	menu_item = g_menu_item_new_submenu (_("_Language"), G_MENU_MODEL (lang_menu));

	return menu_item;
}

static void
activate_suggestion_cb (GtkWidget *menu_item)
{

	printf("===\n%s\n===", "activate_suggestion_cb");
				     return;

	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	if (data->callback != NULL)
	{
		data->callback (data->suggested_word, data->user_data);
	}
}

static void
ignore_all_cb (GtkWidget *menu_item)
{
	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	gspell_checker_add_word_to_session (data->checker,
					    data->misspelled_word,
					    -1);
}

static void
add_to_dictionary_cb (GtkWidget *menu_item)
{
	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	gspell_checker_add_word_to_personal (data->checker,
					     data->misspelled_word,
					     -1);
}

static GMenu *
get_suggestion_menu (GspellChecker                     *checker,
		     const gchar                       *misspelled_word,
		     GspellSuggestionActivatedCallback  callback,
		     gpointer                           user_data)
{
	/* GtkWidget *top_menu; */
	GMenuItem *menu_item;
	GSList *suggestions = NULL;
	SuggestionData *data;

	/* top_menu = gtk_menu_new (); */

	GMenu * menu = g_menu_new ();

	suggestions = gspell_checker_get_suggestions (checker, misspelled_word, -1);

	if (suggestions == NULL)
	{
		/* No suggestions. Put something in the menu anyway... */
		menu_item = g_menu_item_new(_("(no suggested words)"), NULL);
		g_menu_append_item (menu, menu_item);
	}
	else {

		GSList *l;
		for (l = suggestions; l != NULL; l = l->next)
		{
			gchar *suggested_word = l->data;
			menu_item = g_menu_item_new (suggested_word, NULL);
			g_menu_item_set_action_and_target (menu_item, "spelling.correct", "s", suggested_word);

			g_menu_append_item (menu, menu_item);

			data = g_new0 (SuggestionData, 1);
			data->suggested_word = g_strdup (suggested_word);
			data->callback = callback;
			data->user_data = user_data;

			/* g_object_set_data_full (G_OBJECT (menu_item), */
			/* 			SUGGESTION_DATA_KEY, */
			/* 			data, */
			/* 			suggestion_data_free); */

			/* g_signal_connect (menu_item, */
			/* 		  "activate", */
			/* 		  G_CALLBACK (activate_suggestion_cb), */
			/* 		  NULL); */

		}
	}

	GMenu * add_ignore_menu = g_menu_new ();

	menu_item = g_menu_item_new (_("_Ignore All"), NULL);
	g_menu_item_set_action_and_target (menu_item, "spelling.ignore-all", "s", misspelled_word);

	g_menu_append_item (add_ignore_menu, menu_item);

	menu_item = g_menu_item_new (_("_Add"), NULL);
	g_menu_item_set_action_and_target (menu_item, "spelling.add", "s", misspelled_word);

	g_menu_append_item (add_ignore_menu, menu_item);

	GMenuItem * add_ignore_section = g_menu_item_new_section (NULL, G_MENU_MODEL(add_ignore_menu));

	g_menu_append_item (menu, add_ignore_section);

	return menu;

	/* if (suggestions == NULL) */
	/* { */
		/* No suggestions. Put something in the menu anyway... */
		/* menu_item = gtk_menu_item_new_with_label (_("(no suggested words)")); */
		/* gtk_widget_set_sensitive (menu_item, FALSE); */
		/* gtk_menu_shell_prepend (GTK_MENU_SHELL (top_menu), menu_item); */
	/* } */
	/* else */
	/* { */
	/* 	GtkWidget *menu = top_menu; */
	/* 	gint count = 0; */
	/* 	GSList *l; */

		/* Build a set of menus with suggestions. */
	/* 	for (l = suggestions; l != NULL; l = l->next) */
	/* 	{ */
	/* 		gchar *suggested_word = l->data; */
	/* 		GtkWidget *label; */
	/* 		gchar *label_text; */

	/* 		if (count == 10) */
	/* 		{ */
				/* Separator */
	/* 			menu_item = gtk_separator_menu_item_new (); */
	/* 			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */

	/* 			menu_item = gtk_menu_item_new_with_mnemonic (_("_More…")); */
	/* 			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */

	/* 			menu = gtk_menu_new (); */
	/* 			gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu); */
	/* 			count = 0; */
	/* 		} */

	/* 		label_text = g_strdup_printf ("<b>%s</b>", suggested_word); */

	/* 		label = gtk_label_new (label_text); */
	/* 		gtk_label_set_use_markup (GTK_LABEL (label), TRUE); */
	/* 		gtk_widget_set_halign (label, GTK_ALIGN_START); */

	/* 		menu_item = gtk_menu_item_new (); */
	/* 		gtk_container_add (GTK_CONTAINER (menu_item), label); */
	/* 		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */

	/* 		data = g_new0 (SuggestionData, 1); */
	/* 		data->suggested_word = g_strdup (suggested_word); */
	/* 		data->callback = callback; */
	/* 		data->user_data = user_data; */

	/* 		g_object_set_data_full (G_OBJECT (menu_item), */
	/* 					SUGGESTION_DATA_KEY, */
	/* 					data, */
	/* 					suggestion_data_free); */

	/* 		g_signal_connect (menu_item, */
	/* 				  "activate", */
	/* 				  G_CALLBACK (activate_suggestion_cb), */
	/* 				  NULL); */

	/* 		g_free (label_text); */
	/* 		count++; */
	/* 	} */
	/* } */

	/* g_slist_free_full (suggestions, g_free); */

	/* Separator */
	/* menu_item = gtk_separator_menu_item_new (); */
	/* gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item); */

	/* Ignore all */
	/* menu_item = gtk_menu_item_new_with_mnemonic (_("_Ignore All")); */
	/* gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item); */

	/* data = g_new0 (SuggestionData, 1); */
	/* data->checker = g_object_ref (checker); */
	/* data->misspelled_word = g_strdup (misspelled_word); */

	/* g_object_set_data_full (G_OBJECT (menu_item), */
	/* 			SUGGESTION_DATA_KEY, */
	/* 			data, */
	/* 			suggestion_data_free); */

	/* g_signal_connect (menu_item, */
	/* 		  "activate", */
	/* 		  G_CALLBACK (ignore_all_cb), */
	/* 		  NULL); */

	/* Add to Dictionary */
	/* menu_item = gtk_menu_item_new_with_mnemonic (_("_Add")); */
	/* gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item); */

	/* data = g_new0 (SuggestionData, 1); */
	/* data->checker = g_object_ref (checker); */
	/* data->misspelled_word = g_strdup (misspelled_word); */

	/* g_object_set_data_full (G_OBJECT (menu_item), */
	/* 			SUGGESTION_DATA_KEY, */
	/* 			data, */
	/* 			suggestion_data_free); */

	/* g_signal_connect (menu_item, */
	/* 		  "activate", */
	/* 		  G_CALLBACK (add_to_dictionary_cb), */
	/* 		  NULL); */

	/* return top_menu; */
}

GMenuItem *
_gspell_context_menu_get_suggestions_menu_item (GspellChecker                     *checker,
						const gchar                       *misspelled_word,
						GspellSuggestionActivatedCallback  callback,
						gpointer                           user_data)
{
	/* GtkWidget *suggestion_menu; */
	/* GtkMenuItem *menu_item; */

	/* g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL); */
	/* g_return_val_if_fail (misspelled_word != NULL, NULL); */

	/* suggestion_menu = get_suggestion_menu (checker, */
	/* 				       misspelled_word, */
	/* 				       callback, */
	/* 				       user_data); */

	/* menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_mnemonic (_("_Spelling Suggestions…"))); */
	/* gtk_menu_item_set_submenu (menu_item, suggestion_menu); */
	/* gtk_widget_show_all (GTK_WIDGET (menu_item)); */

	/* return menu_item; */

	GMenu *suggestion_menu;
	GMenuItem *menu_item;

	suggestion_menu = get_suggestion_menu (checker, misspelled_word, callback, user_data);

	menu_item = g_menu_item_new_submenu (_("_Spelling Suggestions…"), G_MENU_MODEL (suggestion_menu));

	return menu_item;
}

/* ex:set ts=8 noet: */

