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

#ifndef GSPELL_CHECKER_H
#define GSPELL_CHECKER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_CHECKER (gspell_checker_get_type())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (GspellChecker, gspell_checker, GSPELL, CHECKER, GObject)

/**
 * GSPELL_CHECKER_ERROR:
 *
 * Error domain for the spell checker. Errors in this domain will be from the
 * #GspellCheckerError enumeration. See #GError for more information on
 * error domains.
 */
#define GSPELL_CHECKER_ERROR (gspell_checker_error_quark ())

/**
 * GspellCheckerError:
 * @GSPELL_CHECKER_ERROR_DICTIONARY: dictionary error.
 * @GSPELL_CHECKER_ERROR_NO_LANGUAGE_SET: no language set.
 *
 * An error code used with %GSPELL_CHECKER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GspellCheckerError
{
	GSPELL_CHECKER_ERROR_DICTIONARY,
	GSPELL_CHECKER_ERROR_NO_LANGUAGE_SET,
} GspellCheckerError;

struct _GspellCheckerInterface
{
	GTypeInterface parent_interface;

	gboolean (* check_word)		(GspellChecker  *checker,
					 const gchar    *word,
					 gssize          word_length,
					 GError        **error);

	GSList * (* get_suggestions)	(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* add_word_to_session)	(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* add_word_to_personal)	(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* set_correction)		(GspellChecker *checker,
					 const gchar   *word,
					 gssize         word_length,
					 const gchar   *replacement,
					 gssize         replacement_length);

	void (* clear_session)		(GspellChecker *checker);

	void (* set_language)		(GspellChecker        *checker,
					 const GspellLanguage *language);

	const GspellLanguage * (*get_language)	(GspellChecker *checker);

	/* Signals */
	void (* word_added_to_personal)	(GspellChecker *checker,
					 const gchar   *word);

	void (* word_added_to_session)	(GspellChecker *checker,
					 const gchar   *word);

	void (* session_cleared)	(GspellChecker *checker);

	/* Padding for future expansion */
	gpointer padding[12];
};

GSPELL_AVAILABLE_IN_ALL
GQuark		gspell_checker_error_quark		(void);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_set_language		(GspellChecker        *checker,
							 const GspellLanguage *language);

GSPELL_AVAILABLE_IN_ALL
const GspellLanguage * gspell_checker_get_language	(GspellChecker *checker);

GSPELL_AVAILABLE_IN_ALL
gboolean	gspell_checker_check_word		(GspellChecker  *checker,
							 const gchar    *word,
							 gssize          word_length,
							 GError        **error);

GSPELL_AVAILABLE_IN_ALL
GSList *	gspell_checker_get_suggestions		(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_add_word_to_personal	(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_add_word_to_session	(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_clear_session		(GspellChecker *checker);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_set_correction		(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length,
							 const gchar   *replacement,
							 gssize         replacement_length);

/* Signals */
GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_word_added_to_personal	(GspellChecker *checker,
				 					const gchar   *word);

GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_word_added_to_session	(GspellChecker *checker,
			 						const gchar   *word);

GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_session_cleared		(GspellChecker *checker);

G_END_DECLS

#endif  /* GSPELL_CHECKER_H */

/* ex:set ts=8 noet: */

