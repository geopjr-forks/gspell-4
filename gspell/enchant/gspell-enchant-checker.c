/*
 * This file is part of gspell, a spell-checking library.
 *
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

#include "config.h"
#include "gspell-enchant-checker.h"
#include <glib/gi18n-lib.h>
#include "gspell-utils.h"

typedef struct _GspellEnchantCheckerPrivate GspellEnchantCheckerPrivate;

struct _GspellEnchantCheckerPrivate
{
	const GspellLanguage * active_lang;
	EnchantBroker *broker;
	EnchantDict *dict;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
};

static void gspell_enchant_checker_iface_init (GspellCheckerInterface * iface);
G_DEFINE_TYPE_WITH_CODE (GspellEnchantChecker,
			 gspell_enchant_checker,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE(GspellEnchantChecker)
			 G_IMPLEMENT_INTERFACE (GSPELL_TYPE_CHECKER,
						gspell_enchant_checker_iface_init))


static void create_new_dictionary (GspellEnchantChecker *checker);

static void
gspell_enchant_checker_add_word_to_personal(GspellChecker *checker,
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
		gspell_checker_word_added_to_personal (checker, word);
	} else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		gspell_checker_word_added_to_personal (checker, nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

static gboolean
gspell_enchant_checker_check_word (GspellChecker  *checker,
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
			     GSPELL_CHECKER_ERROR,
			     GSPELL_CHECKER_ERROR_DICTIONARY,
			     _("Error when checking the spelling of word “%s”: %s"),
			     nul_terminated_word,
			     enchant_dict_get_error (priv->dict));

		g_free (nul_terminated_word);
	}

	return correctly_spelled;
}

static GSList *
gspell_enchant_checker_get_suggestions	(GspellChecker *checker,
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
gspell_enchant_checker_add_word_to_session	(GspellChecker *checker,
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
		gspell_checker_word_added_to_session (checker, word);
	}
	else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		gspell_checker_word_added_to_session (checker, nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

static void
gspell_enchant_checker_set_correction	(GspellChecker *checker,
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
gspell_enchant_checker_clear_session (GspellChecker *checker)
{
	/* Free and re-request dictionary. */
	create_new_dictionary (GSPELL_ENCHANT_CHECKER (checker));

	gspell_checker_session_cleared (checker);

}

/* Used for unit tests. Useful to force a NULL language. */
void
_gspell_checker_force_set_language (GspellChecker        *checker,
				    const GspellLanguage *language)
{
	GspellEnchantCheckerPrivate *priv;

	g_return_if_fail (GSPELL_IS_ENCHANT_CHECKER (checker));

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	if (priv->active_lang != language)
	{
		priv->active_lang = language;
		create_new_dictionary (GSPELL_ENCHANT_CHECKER(checker));
		g_object_notify (G_OBJECT (checker), "language");
	}
}

static void
gspell_enchant_checker_set_language 	(GspellChecker        *checker,
		 			const GspellLanguage *language)
{
	g_return_if_fail (GSPELL_IS_ENCHANT_CHECKER (checker));

	if (language == NULL)
	{
		language = gspell_language_get_default ();
	}

	_gspell_checker_force_set_language (checker, language);

}

static const GspellLanguage *
gspell_enchant_checker_get_language 	(GspellChecker        *checker)
{
	GspellEnchantCheckerPrivate *priv;

	priv = gspell_enchant_checker_get_instance_private (GSPELL_ENCHANT_CHECKER(checker));

	return priv->active_lang;
}

static void
gspell_enchant_checker_iface_init (GspellCheckerInterface * iface)
{
	iface->add_word_to_personal = gspell_enchant_checker_add_word_to_personal;
	iface->check_word = gspell_enchant_checker_check_word;
	iface->get_suggestions = gspell_enchant_checker_get_suggestions;
	iface->add_word_to_session = gspell_enchant_checker_add_word_to_session;
	iface->set_correction = gspell_enchant_checker_set_correction;
	iface->clear_session = gspell_enchant_checker_clear_session;
	iface->set_language = gspell_enchant_checker_set_language;
	iface->get_language = gspell_enchant_checker_get_language;
}

static void
gspell_enchant_checker_set_property 	(GObject    *object,
                      			guint       prop_id,
                              		const GValue     *value,
                              		GParamSpec *pspec)
{
	GspellChecker * checker = GSPELL_CHECKER(object);
  	switch (prop_id)
	{
	case PROP_LANGUAGE:
		gspell_enchant_checker_set_language (checker, g_value_get_boxed (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gspell_enchant_checker_get_property	(GObject    *object,
					guint       prop_id,
					GValue     *value,
					GParamSpec *pspec)
{
	GspellChecker * checker = GSPELL_CHECKER(object);
  	switch (prop_id)
	{
	case PROP_LANGUAGE:
		g_value_set_boxed (value, gspell_enchant_checker_get_language (checker));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gspell_enchant_checker_class_init (GspellEnchantCheckerClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gspell_enchant_checker_set_property;
	object_class->get_property = gspell_enchant_checker_get_property;

	g_object_class_override_property (object_class, PROP_LANGUAGE, "language");
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

	priv = gspell_enchant_checker_get_instance_private (checker);

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
		priv->dict = NULL;
	}

	language_code = gspell_language_get_code (priv->active_lang);
	priv->dict = enchant_broker_request_dict (priv->broker, language_code);

	if (priv->dict == NULL)
	{
		/* Should never happen, no need to return a GError. */
		g_warning ("Impossible to create an Enchant dictionary for the language code '%s'.",
			   language_code);

		gspell_enchant_checker_set_language (GSPELL_CHECKER(checker), NULL);
		return;
	}
}

/**
 * gspell_enchant_checker_get_dict:
 * @enchant_checker: a #GspellEnchantChecker.
 *
 * Gets the EnchantDict currently used by @checker.
 *
 * #GspellEnchantChecker re-creates a new EnchantDict when the #GspellChecker:language
 * is changed and when the session is cleared.
 *
 * Returns: (transfer none) (nullable): the #EnchantDict currently used by
 * @checker.
 * Since: 4.0
 */
EnchantDict *
gspell_enchant_checker_get_dict (GspellEnchantChecker * enchant_checker)
{
	GspellEnchantCheckerPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_CHECKER (enchant_checker), NULL);

	priv = gspell_enchant_checker_get_instance_private (enchant_checker);
	return priv->dict;
}

/**
 * gspell_enchant_checker_new:
 * @language: (nullable): the #GspellLanguage to use, or %NULL.
 *
 * Creates a new #GspellChecker. If @language is %NULL, the default language is
 * picked with gspell_language_get_default().
 *
 * Returns: (nullable) (transfer none): a new #GspellChecker object.
 * Since: 4.0
 */
GspellChecker *
gspell_enchant_checker_new (const GspellLanguage *language)
{
  return g_object_new (GSPELL_TYPE_ENCHANT_CHECKER,
		       "language", language,
			NULL);
}


/* ex:set ts=8 noet: */


