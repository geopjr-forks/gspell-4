/*
 * This file is part of gspell, a spell-checking library.
 *
 * gspell-enchant-checker.
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

#ifndef GSPELL_ENCHANT_CHECKER_H
#define GSPELL_ENCHANT_CHECKER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <enchant-2/enchant.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-checker-provider.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_ENCHANT_CHECKER (gspell_enchant_checker_get_type())

G_DECLARE_FINAL_TYPE (GspellEnchantChecker, gspell_enchant_checker, GSPELL, ENCHANT_CHECKER, GspellCheckerProvider);

struct _GspellEnchantChecker
{
	GspellCheckerProviderClass parent_class;
};

GspellCheckerProvider *gspell_enchant_checker_new (const GspellLanguage *language);

G_END_DECLS

#endif  /* GSPELL_ENCHANT_CHECKER_H */

/* ex:set ts=8 noet: */
