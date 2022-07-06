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


#include "config.h"

#include <stdio.h>
#include "gspell-checker.h"

enum
{
	SIGNAL_WORD_ADDED_TO_PERSONAL,
	SIGNAL_WORD_ADDED_TO_SESSION,
	SIGNAL_SESSION_CLEARED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_INTERFACE (GspellChecker, gspell_checker, G_TYPE_OBJECT)

static void
gspell_checker_default_init (GspellCheckerInterface * iface)
{
	/**
	 * GspellChecker:language:
	 *
	 * The #GspellLanguage used.
	 */
	g_object_interface_install_property (iface,
					     g_param_spec_boxed ("language",
							     "Language",
							     "",
							     GSPELL_TYPE_LANGUAGE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * GspellChecker::word-added-to-personal:
	 * @spell_checker: the #GspellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the personal dictionary.
	 */
	signals[SIGNAL_WORD_ADDED_TO_PERSONAL] =
		g_signal_new ("word-added-to-personal",
			      GSPELL_TYPE_CHECKER,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerInterface, word_added_to_personal),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellChecker::word-added-to-session:
	 * @spell_checker: the #GspellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the session dictionary. See
	 * gspell_checker_add_word_to_session().
	 */
	signals[SIGNAL_WORD_ADDED_TO_SESSION] =
		g_signal_new ("word-added-to-session",
			      GSPELL_TYPE_CHECKER,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerInterface, word_added_to_session),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellChecker::session-cleared:
	 * @spell_checker: the #GspellChecker.
	 *
	 * Emitted when the session dictionary is cleared.
	 */
	signals[SIGNAL_SESSION_CLEARED] =
		g_signal_new ("session-cleared",
			      GSPELL_TYPE_CHECKER,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerInterface, session_cleared),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      0);
}

GQuark
gspell_checker_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gspell-checker-error-quark");
	}

	return quark;
}

/**
 * gspell_checker_set_language:
 * @checker: a #GspellChecker.
 * @language: (nullable): the #GspellLanguage to use, or %NULL.
 *
 * Sets the language to use for the spell checking. If @language is %NULL, the
 * default language is picked with gspell_language_get_default().
 *
 */
void
gspell_checker_set_language	(GspellChecker        *checker,
				 const GspellLanguage *language)
{

  	g_return_if_fail (GSPELL_IS_CHECKER (checker));

	return GSPELL_CHECKER_GET_IFACE(checker)->set_language(checker, language);
}

/**
 * gspell_checker_get_language:
 * @checker: a #GspellChecker.
 *
 * Returns: (nullable): the #GspellLanguage currently used, or %NULL
 * if no dictionaries are available.
 */
const GspellLanguage *
gspell_checker_get_language		(GspellChecker *checker)
{
  	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);

  	return GSPELL_CHECKER_GET_IFACE(checker)->get_language(checker);
}

/**
 * gspell_checker_check_word:
 * @checker: a #GspellChecker.
 * @word: the word to check.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * If the #GspellChecker:language is %NULL, i.e. when no dictonaries are
 * available, this function returns %TRUE to limit the damage.
 *
 * Returns: %TRUE if @word is correctly spelled, %FALSE otherwise.
 */
gboolean
gspell_checker_check_word	(GspellChecker  *checker,
					 const gchar    *word,
					 gssize          word_length,
					 GError        **error)
{
    	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), FALSE);
	g_return_val_if_fail (word != NULL, FALSE);
	g_return_val_if_fail (word_length >= -1, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return GSPELL_CHECKER_GET_IFACE(checker)->check_word(checker, word, word_length, error);

}

/**
 * gspell_checker_get_suggestions:
 * @checker: a #GspellChecker.
 * @word: a misspelled word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Gets the suggestions for @word. Free the return value with
 * g_slist_free_full(suggestions, g_free).
 *
 * Returns: (transfer full) (element-type utf8): the list of suggestions.
 */
GSList *
gspell_checker_get_suggestions (GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length)
{
	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);
	g_return_val_if_fail (word != NULL, NULL);
	g_return_val_if_fail (word_length >= -1, NULL);

	return GSPELL_CHECKER_GET_IFACE(checker)->get_suggestions(checker, word, word_length);
}

/**
 * gspell_checker_add_word_to_personal:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Adds a word to the personal dictionary. It is typically saved in the user's
 * home directory.
 */
void
gspell_checker_add_word_to_personal	(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	return GSPELL_CHECKER_GET_IFACE(checker)->add_word_to_personal(checker, word, word_length);
}

/**
 * gspell_checker_add_word_to_session:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Adds a word to the session dictionary. Each #GspellChecker instance has a
 * different session dictionary. The session dictionary is lost when the
 * #GspellChecker:language property changes or when @checker is destroyed or
 * when gspell_checker_clear_session() is called.
 *
 * This function is typically called for an “Ignore All” action.
 */
void
gspell_checker_add_word_to_session	(GspellChecker *checker,
						 const gchar   *word,
						 gssize         word_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	return GSPELL_CHECKER_GET_IFACE(checker)->add_word_to_session(checker, word, word_length);
}

/**
 * gspell_checker_clear_session:
 * @checker: a #GspellChecker.
 *
 * Clears the session dictionary.
 */
void
gspell_checker_clear_session		(GspellChecker *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

	return GSPELL_CHECKER_GET_IFACE(checker)->clear_session(checker);
}

/**
 * gspell_checker_set_correction:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 * @replacement: the replacement word.
 * @replacement_length: the byte length of @replacement, or -1 if @replacement
 *   is nul-terminated.
 *
 * Informs the spell checker that @word is replaced/corrected by @replacement.
 */
void
gspell_checker_set_correction		(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length,
					 const gchar   *replacement,
					 gssize         replacement_length)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);
	g_return_if_fail (replacement != NULL);
	g_return_if_fail (replacement_length >= -1);

	return GSPELL_CHECKER_GET_IFACE(checker)->set_correction(checker, word, word_length, replacement, replacement_length);
}

void
gspell_checker_word_added_to_personal	(GspellChecker *checker,
 					const gchar   *word)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

  	g_signal_emit (checker, signals [SIGNAL_WORD_ADDED_TO_PERSONAL], 0, word);
}

void
gspell_checker_word_added_to_session	(GspellChecker *checker,
					const gchar   *word)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

  	g_signal_emit (checker, signals [SIGNAL_WORD_ADDED_TO_SESSION], 0, word);
}

void
gspell_checker_session_cleared		(GspellChecker *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

  	g_signal_emit (checker, signals [SIGNAL_SESSION_CLEARED], 0);
}

/* ex:set ts=8 noet: */


