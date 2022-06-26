/*
 * gspell-checker-provider.h
 *
 * This file is part of gspell, a spell-checking library.
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

#ifndef GSPELL_CHECKER_PROVIDER_H
#define GSPELL_CHECKER_PROVIDER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_CHECKER_PROVIDER (gspell_checker_provider_get_type())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GspellCheckerProvider, gspell_checker_provider, GSPELL, CHECKER_PROVIDER, GObject)

/**
 * GSPELL_CHECKER_PROVIDER_ERROR:
 *
 * Error domain for the spell checker. Errors in this domain will be from the
 * #GspellCheckerProviderError enumeration. See #GError for more information on
 * error domains.
 */
#define GSPELL_CHECKER_PROVIDER_ERROR (gspell_checker_provider_error_quark ())

/**
 * GspellCheckerProviderError:
 * @GSPELL_CHECKER_PROVIDER_ERROR_DICTIONARY: dictionary error.
 * @GSPELL_CHECKER_PROVIDER_ERROR_NO_LANGUAGE_SET: no language set.
 *
 * An error code used with %GSPELL_CHECKER_PROVIDER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GspellCheckerProviderError
{
	GSPELL_CHECKER_PROVIDER_ERROR_DICTIONARY,
	GSPELL_CHECKER_PROVIDER_ERROR_NO_LANGUAGE_SET,
} GspellCheckerProviderError;

struct _GspellCheckerProviderClass
{
	GObjectClass parent_class;

	gboolean (* check_word)		(GspellCheckerProvider  *checker,
					 const gchar    *word,
					 gssize          word_length,
					 GError        **error);

	GSList * (* get_suggestions)	(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* add_word_to_session)	(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* add_word_to_personal)	(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length);

	void (* set_correction)		(GspellCheckerProvider *checker,
					 const gchar   *word,
					 gssize         word_length,
					 const gchar   *replacement,
					 gssize         replacement_length);

	void (* clear_session)		(GspellCheckerProvider *checker);

	void (* set_language)		(GspellCheckerProvider        *checker,
					 const GspellLanguage *language);

	const GspellLanguage * (*get_language)	(GspellCheckerProvider *checker);

	/* Signals */
	void (* word_added_to_personal)	(GspellCheckerProvider *checker,
					 const gchar   *word);

	void (* word_added_to_session)	(GspellCheckerProvider *checker,
					 const gchar   *word);

	void (* session_cleared)	(GspellCheckerProvider *checker);

	/* Padding for future expansion */
	gpointer padding[12];
};

GSPELL_AVAILABLE_IN_ALL
GQuark		gspell_checker_provider_error_quark		(void);

GSPELL_AVAILABLE_IN_ALL
GspellCheckerProvider *	gspell_checker_provider_new			(const GspellLanguage *language);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_provider_set_language		(GspellCheckerProvider        *checker,
							 const GspellLanguage *language);

GSPELL_AVAILABLE_IN_ALL
const GspellLanguage * gspell_checker_provider_get_language	(GspellCheckerProvider *checker);

GSPELL_AVAILABLE_IN_ALL
gboolean	gspell_checker_provider_check_word		(GspellCheckerProvider  *checker,
							 const gchar    *word,
							 gssize          word_length,
							 GError        **error);

GSPELL_AVAILABLE_IN_ALL
GSList *	gspell_checker_provider_get_suggestions		(GspellCheckerProvider *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_provider_add_word_to_personal	(GspellCheckerProvider *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_provider_add_word_to_session	(GspellCheckerProvider *checker,
							 const gchar   *word,
							 gssize         word_length);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_provider_clear_session		(GspellCheckerProvider *checker);

GSPELL_AVAILABLE_IN_ALL
void		gspell_checker_provider_set_correction		(GspellCheckerProvider *checker,
							 const gchar   *word,
							 gssize         word_length,
							 const gchar   *replacement,
							 gssize         replacement_length);


/* Signals */
GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_provider_emit_word_added_to_personal	(GspellCheckerProvider *checker,
				 					const gchar   *word);

GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_provider_emit_word_added_to_session	(GspellCheckerProvider *checker,
			 						const gchar   *word);

GSPELL_AVAILABLE_IN_ALL
void 		gspell_checker_provider_emit_session_cleared		(GspellCheckerProvider *checker);

G_END_DECLS

#endif  /* GSPELL_CHECKER_PROVIDER_H */

/* ex:set ts=8 noet: */

