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

#ifndef GSPELL_ENCHANT_CHECKER_H
#define GSPELL_ENCHANT_CHECKER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <enchant-2/enchant.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-checker.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_ENCHANT_CHECKER (gspell_enchant_checker_get_type())

G_DECLARE_DERIVABLE_TYPE (GspellEnchantChecker, gspell_enchant_checker,
			  GSPELL, ENCHANT_CHECKER, GObject);

struct _GspellEnchantCheckerClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

EnchantDict *
gspell_enchant_checker_get_dict (GspellEnchantChecker * enchant_checker);

GspellChecker *gspell_enchant_checker_new (const GspellLanguage *language);

G_END_DECLS

#endif  /* GSPELL_ENCHANT_CHECKER_H */

/* ex:set ts=8 noet: */
