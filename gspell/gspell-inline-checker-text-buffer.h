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

/* This is a modified version of GtkSpell 2.0.2 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#ifndef GSPELL_INLINE_CHECKER_TEXT_BUFFER_H
#define GSPELL_INLINE_CHECKER_TEXT_BUFFER_H

#include <gtk/gtk.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_INLINE_CHECKER_TEXT_BUFFER (_gspell_inline_checker_text_buffer_get_type ())

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (GspellInlineCheckerTextBuffer, _gspell_inline_checker_text_buffer,
		      GSPELL, INLINE_CHECKER_TEXT_BUFFER,
		      GObject)

struct _GspellInlineCheckerTextBufferClass
{
	GObjectClass parent_class;

	/* Signals */
	void (* language_changed)	(GspellInlineCheckerTextBuffer *spell,
					 const GspellLanguage   *language);
};

G_GNUC_INTERNAL
GspellInlineCheckerTextBuffer *
	_gspell_inline_checker_text_buffer_new			(GtkTextBuffer *buffer);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_attach_view		(GspellInlineCheckerTextBuffer *spell,
								 GtkTextView                   *view);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_detach_view		(GspellInlineCheckerTextBuffer *spell,
								 GtkTextView                   *view);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_correct		(GspellInlineCheckerTextBuffer *spell,
					   			const gchar *suggested_word);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_populate_popup	(GspellInlineCheckerTextBuffer *spell,
								 GMenu                       *menu);

/* For unit tests */

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_set_unit_test_mode	(GspellInlineCheckerTextBuffer *spell,
								 gboolean                       unit_test_mode);

G_GNUC_INTERNAL
GtkTextTag *
	_gspell_inline_checker_text_buffer_get_highlight_tag	(GspellInlineCheckerTextBuffer *spell);

G_GNUC_INTERNAL
gchar * gspell_inline_checker_get_word_at_click_position	(GspellInlineCheckerTextBuffer *spell);

G_GNUC_INTERNAL
GSList * gspell_inline_checker_get_suggestions			(GspellInlineCheckerTextBuffer *spell,
						   		gchar * misspelled_word);

GSPELL_AVAILABLE_IN_4_0
void gspell_inline_checker_text_buffer_language_changed (GspellInlineCheckerTextBuffer * spell,
							 const GspellLanguage * language);

G_END_DECLS

#endif  /* GSPELL_INLINE_CHECKER_TEXT_BUFFER_H */

/* ex:set ts=8 noet: */

