/*
 * This file is part of gspell, a spell-checking library.
 *
 * gspell-enchant-checker.c
 *
 * Copyright 2022 otrocodigo <otrocodigodev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include "gspell-enchant-checker.h"
#include <glib/gi18n-lib.h>
#include "gspell-utils.h"

typedef struct _GspellEnchantCheckerPrivate GspellEnchantCheckerPrivate;

struct _GspellEnchantCheckerPrivate
{
	GspellCheckerProvider parent_instance;

	EnchantBroker *broker;
	EnchantDict *dict;
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellEnchantChecker, gspell_enchant_checker, GSPELL_TYPE_CHECKER_PROVIDER)


static void create_new_dictionary (GspellEnchantChecker *checker);

static void
gspell_enchant_checker_add_word_to_personal(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length)
{
	GspellEnchantCheckerPrivate * priv;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_add (priv->dict, word, word_length);

	if (word_length == -1)
	{
		gspell_checker_provider_emit_word_added_to_personal (checker, word);
	} else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		gspell_checker_provider_emit_word_added_to_personal (checker, nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

static gboolean
gspell_enchant_checker_check_word (GspellCheckerProvider  *checker,
				 const gchar    *word,
				 gssize          word_length,
				 GError        **error)
{
	GspellEnchantCheckerPrivate *priv;
	gint enchant_result;
	gboolean correctly_spelled;
	gchar *sanitized_word;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->dict == NULL)
	{
		return TRUE;
	}

	if (_gspell_utils_is_number (word, word_length))
	{
		return TRUE;
	}

	if (_gspell_utils_str_to_ascii_apostrophe (word, word_length, &sanitized_word))
	{
		enchant_result = enchant_dict_check (priv->dict, sanitized_word, -1);
		g_free (sanitized_word);
	}
	else
	{
		enchant_result = enchant_dict_check (priv->dict, word, word_length);
	}

	correctly_spelled = enchant_result == 0;

	if (enchant_result < 0)
	{
		gchar *nul_terminated_word;

		if (word_length == -1)
		{
			word_length = strlen (word);
		}

		nul_terminated_word = g_strndup (word, word_length);

		g_set_error (error,
			     GSPELL_CHECKER_PROVIDER_ERROR,
			     GSPELL_CHECKER_PROVIDER_ERROR_DICTIONARY,
			     _("Error when checking the spelling of word “%s”: %s"),
			     nul_terminated_word,
			     enchant_dict_get_error (priv->dict));

		g_free (nul_terminated_word);
	}

	return correctly_spelled;
}

static GSList *
gspell_enchant_checker_get_suggestions	(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length)
{
	GspellEnchantCheckerPrivate *priv;
	gchar *sanitized_word;
	gchar **suggestions;
	GSList *suggestions_list = NULL;
	gint i;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->dict == NULL)
	{
		return NULL;
	}

	if (_gspell_utils_str_to_ascii_apostrophe (word, word_length, &sanitized_word))
	{
		suggestions = enchant_dict_suggest (priv->dict, sanitized_word, -1, NULL);
		g_free (sanitized_word);
	}
	else
	{
		suggestions = enchant_dict_suggest (priv->dict, word, word_length, NULL);
	}

	if (suggestions == NULL)
	{
		return NULL;
	}

	for (i = 0; suggestions[i] != NULL; i++)
	{
		suggestions_list = g_slist_prepend (suggestions_list, suggestions[i]);
	}

	/* The array/list elements will be freed by the caller. */
	g_free (suggestions);

	return g_slist_reverse (suggestions_list);
}

static void
gspell_enchant_checker_add_word_to_session	(GspellCheckerProvider *checker,
			 			const gchar   *word,
					 	gssize         word_length)
{
	GspellEnchantCheckerPrivate *priv;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_add_to_session (priv->dict, word, word_length);

	if (word_length == -1)
	{
		gspell_checker_provider_emit_word_added_to_session (checker, word);
	}
	else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		gspell_checker_provider_emit_word_added_to_session (checker, nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

static void
gspell_enchant_checker_set_correction	(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length,
					 const gchar   *replacement,
					 gssize         replacement_length)
{
	GspellEnchantCheckerPrivate *priv;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_store_replacement (priv->dict,
					word, word_length,
					replacement, replacement_length);
}

static void
gspell_enchant_checker_clear_session (GspellCheckerProvider *checker)
{
	/* Free and re-request dictionary. */
	create_new_dictionary (GSPELL_ENCHANT_CHECKER (checker));

	gspell_checker_provider_emit_session_cleared (checker);

}

static void
gspell_enchant_checker_set_language (GspellCheckerProvider        *checker,
		 			const GspellLanguage *language)
{
	create_new_dictionary (GSPELL_ENCHANT_CHECKER (checker));
}

static void
gspell_enchant_checker_class_init (GspellEnchantCheckerClass * klass)
{
	GspellCheckerProviderClass *object_class = GSPELL_CHECKER_PROVIDER_CLASS (klass);

	object_class->add_word_to_personal = gspell_enchant_checker_add_word_to_personal;
	object_class->check_word = gspell_enchant_checker_check_word;
	object_class->get_suggestions = gspell_enchant_checker_get_suggestions;
	object_class->add_word_to_session = gspell_enchant_checker_add_word_to_session;
	object_class->set_correction = gspell_enchant_checker_set_correction;
	object_class->clear_session = gspell_enchant_checker_clear_session;
	object_class->set_language = gspell_enchant_checker_set_language;
}

static void
gspell_enchant_checker_init (GspellEnchantChecker * self)
{
	GspellEnchantCheckerPrivate * priv;

	priv = gspell_enchant_checker_get_instance_private (self);

	priv->broker = enchant_broker_init ();
	priv->dict = NULL;
}

static void
create_new_dictionary (GspellEnchantChecker *checker)
{
	GspellEnchantCheckerPrivate *priv;
	const gchar *language_code;
	const GspellLanguage * active_lang;

	priv = gspell_enchant_checker_get_instance_private (checker);

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
		priv->dict = NULL;
	}

	active_lang = gspell_checker_provider_get_language(GSPELL_CHECKER_PROVIDER (checker));

	if (active_lang == NULL)
	{
		return;
	}

	language_code = gspell_language_get_code (active_lang);
	priv->dict = enchant_broker_request_dict (priv->broker, language_code);

	if (priv->dict == NULL)
	{
		/* Should never happen, no need to return a GError. */
		g_warning ("Impossible to create an Enchant dictionary for the language code '%s'.",
			   language_code);

		gspell_checker_provider_set_language (GSPELL_CHECKER_PROVIDER(checker), NULL);
		return;
	}
}

GspellCheckerProvider *
gspell_enchant_checker_new (const GspellLanguage *language)
{
  return g_object_new (GSPELL_TYPE_ENCHANT_CHECKER,
		       "language", language,
			NULL);
}


/* ex:set ts=8 noet: */
