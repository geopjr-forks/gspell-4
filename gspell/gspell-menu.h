/*
 * This file is part of gspell, a spell-checking library.
 *
 * gspell-men
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

#ifndef GSPELL_SUGGESTION_MENU_H
#define GSPELL_SUGGESTION_MENU_H

#include <gio/gio.h>
#include <glib-object.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_ACTION_HAS_SUGGESTIONS 	"has-suggestions"
#define GSPELL_ACTION_LANGUAGE 		"language"
#define GSPELL_ACTION_CORRECT 		"correct"
#define GSPELL_ACTION_ADD 		"add"
#define GSPELL_ACTION_IGNORE_ALL	"ignore-all"
#define GSPELL_ACTION_GROUP		"spelling"

#define GSPELL_TYPE_SUGGESTION_MENU (gspell_suggestion_menu_get_type())
G_DECLARE_FINAL_TYPE(GspellSuggestionMenu, gspell_suggestion_menu, GSPELL, SUGGESTION_MENU, GMenuModel);

GMenuModel *    gspell_suggestion_menu_new(void);
void            gspell_suggestion_menu_set_suggestions(GspellSuggestionMenu * menu, GSList * suggestions);

GMenu   *       gspell_menu_new_from_suggestion_menu(GspellSuggestionMenu * menu);

G_END_DECLS

#endif /* GSPELL_SUGGESTION_MENU_H */

/* ex:set ts=8 noet: */
