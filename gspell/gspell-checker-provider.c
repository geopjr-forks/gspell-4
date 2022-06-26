/* editor-spell-provider.c
 *
 * Copyright 2021 Christian Hergert <chergert@redhat.com>
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

#include <stdio.h>
#include "gspell-checker-provider.h"

typedef struct _GspellCheckerProviderPrivate GspellCheckerProviderPrivate;

struct _GspellCheckerProviderPrivate
{
  const GspellLanguage *active_lang;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
};

enum
{
	SIGNAL_WORD_ADDED_TO_PERSONAL,
	SIGNAL_WORD_ADDED_TO_SESSION,
	SIGNAL_SESSION_CLEARED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GspellCheckerProvider, gspell_checker_provider, G_TYPE_OBJECT)

GQuark
gspell_checker_provider_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gspell-checker-provider-error-quark");
	}

	return quark;
}

static void
gspell_checker_provider_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	GspellCheckerProvider *checker = GSPELL_CHECKER_PROVIDER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			gspell_checker_provider_set_language (checker, g_value_get_boxed (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_provider_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	GspellCheckerProvider *checker = GSPELL_CHECKER_PROVIDER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			g_value_set_boxed (value, gspell_checker_provider_get_language (checker));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_provider_finalize (GObject *object)
{
	GspellCheckerProviderPrivate *priv;

	priv = gspell_checker_provider_get_instance_private (GSPELL_CHECKER_PROVIDER (object));

	G_OBJECT_CLASS (gspell_checker_provider_parent_class)->finalize (object);
}

static void
gspell_checker_provider_class_init (GspellCheckerProviderClass *klass)
{
  	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gspell_checker_provider_set_property;
	object_class->get_property = gspell_checker_provider_get_property;
	object_class->finalize = gspell_checker_provider_finalize;

	/**
	 * GspellCheckerProvider:language:
	 *
	 * The #GspellLanguage used.
	 */
	g_object_class_install_property (object_class,
					 PROP_LANGUAGE,
					 g_param_spec_boxed ("language",
							     "Language",
							     "",
							     GSPELL_TYPE_LANGUAGE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * GspellCheckerProvider::word-added-to-personal:
	 * @spell_checker: the #GspellCheckerProvider.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the personal dictionary.
	 */
	signals[SIGNAL_WORD_ADDED_TO_PERSONAL] =
		g_signal_new ("word-added-to-personal",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerProviderClass, word_added_to_personal),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellCheckerProvider::word-added-to-session:
	 * @spell_checker: the #GspellCheckerProvider.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the session dictionary. See
	 * gspell_checker_add_word_to_session().
	 */
	signals[SIGNAL_WORD_ADDED_TO_SESSION] =
		g_signal_new ("word-added-to-session",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerProviderClass, word_added_to_session),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellCheckerProvider::session-cleared:
	 * @spell_checker: the #GspellCheckerProvider.
	 *
	 * Emitted when the session dictionary is cleared.
	 */
	signals[SIGNAL_SESSION_CLEARED] =
		g_signal_new ("session-cleared",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerProviderClass, session_cleared),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      0);
}

static void
gspell_checker_provider_init (GspellCheckerProvider *self)
{
}


void
gspell_checker_provider_set_language	(GspellCheckerProvider        *checker,
					 const GspellLanguage *language)
{
	GspellCheckerProviderPrivate *priv = gspell_checker_provider_get_instance_private (checker);

  	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));

	if (language == NULL)
	{
		language = gspell_language_get_default ();
	}

  	if (priv->active_lang != language)
		priv->active_lang = language;

	g_object_notify (G_OBJECT(checker), "language");

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->set_language(checker, language);
}


const GspellLanguage *
gspell_checker_provider_get_language		(GspellCheckerProvider *checker)
{
	GspellCheckerProviderPrivate *priv = gspell_checker_provider_get_instance_private (checker);

  	g_return_val_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker), NULL);

  	return priv->active_lang;
}


gboolean
gspell_checker_provider_check_word	(GspellCheckerProvider  *checker,
					 const gchar    *word,
					 gssize          word_length,
					 GError        **error)
{
    	g_return_val_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker), FALSE);
	g_return_val_if_fail (word != NULL, FALSE);
	g_return_val_if_fail (word_length >= -1, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->check_word(checker, word, word_length, error);

}

GSList *
gspell_checker_provider_get_suggestions (GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length)
{
	g_return_val_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker), NULL);
	g_return_val_if_fail (word != NULL, NULL);
	g_return_val_if_fail (word_length >= -1, NULL);

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->get_suggestions(checker, word, word_length);
}

void
gspell_checker_provider_add_word_to_personal	(GspellCheckerProvider *checker,
						 const gchar   *word,
						 gssize         word_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->add_word_to_personal(checker, word, word_length);
}


void
gspell_checker_provider_add_word_to_session	(GspellCheckerProvider *checker,
						 const gchar   *word,
						 gssize         word_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->add_word_to_session(checker, word, word_length);
}

void
gspell_checker_provider_clear_session		(GspellCheckerProvider *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->clear_session(checker);
}

void
gspell_checker_provider_set_correction		(GspellCheckerProvider *checker,
						 const gchar   *word,
						 gssize         word_length,
						 const gchar   *replacement,
						 gssize         replacement_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);
	g_return_if_fail (replacement != NULL);
	g_return_if_fail (replacement_length >= -1);

	return GSPELL_CHECKER_PROVIDER_GET_CLASS(checker)->set_correction(checker, word, word_length, replacement, replacement_length);
}


void
gspell_checker_provider_emit_word_added_to_personal	(GspellCheckerProvider *checker,
		 					const gchar   *word)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));

  	g_signal_emit (checker, signals [SIGNAL_WORD_ADDED_TO_PERSONAL], 0, word);
}

void
gspell_checker_provider_emit_word_added_to_session	(GspellCheckerProvider *checker,
 							const gchar   *word)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));

  	g_signal_emit (checker, signals [SIGNAL_WORD_ADDED_TO_SESSION], 0, word);
}

void
gspell_checker_provider_emit_session_cleared		(GspellCheckerProvider *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER_PROVIDER (checker));

  	g_signal_emit (checker, signals [SIGNAL_SESSION_CLEARED], 0);
}

/* ex:set ts=8 noet: */
