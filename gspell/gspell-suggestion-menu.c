/*
 *  This file is part of gspell, a spell-checking library.
 *
 * gspell-menu.c
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

#include <glib.h>
#include "config.h"
#include "gspell-suggestion-menu.h"

typedef struct _GspellSuggestionMenuPrivate GspellSuggestionMenuPrivate;

struct _GspellSuggestionMenuPrivate {
	GSList * suggestions;
};

struct _GspellSuggestionMenu {
	GMenuModel parent_instance;
};

enum {
	PROP_0,
	PROP_SUGGESTIONS
};

G_DEFINE_TYPE_WITH_PRIVATE(GspellSuggestionMenu, gspell_suggestion_menu, G_TYPE_MENU_MODEL);

static GSList * gspell_suggestion_menu_get_suggestions(GspellSuggestionMenu * menu);

void
gspell_suggestion_menu_set_property(GObject        *object,
			 guint property_id,
			 const GValue   *value,
			 GParamSpec     *pspec)
{
	GspellSuggestionMenu * menu = GSPELL_SUGGESTION_MENU(object);

	switch (property_id) {
	case PROP_SUGGESTIONS:
		gspell_suggestion_menu_set_suggestions(menu, g_value_get_pointer(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gspell_suggestion_menu_get_property(GObject        *object,
			 guint property_id,
			 GValue         *value,
			 GParamSpec     *pspec)
{
	GspellSuggestionMenu * menu = GSPELL_SUGGESTION_MENU(object);

	switch (property_id) {
	case PROP_SUGGESTIONS:
		g_value_set_pointer(value, gspell_suggestion_menu_get_suggestions(menu));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static gint
gspell_suggestion_menu_get_n_items (GMenuModel *model)
{
	GspellSuggestionMenuPrivate * priv;

	priv = gspell_suggestion_menu_get_instance_private(GSPELL_SUGGESTION_MENU (model));

	return priv->suggestions ? g_slist_length(priv->suggestions) : 0;
}

static void
gspell_suggestion_menu_get_item_attributes (GMenuModel  *model,
                      		int          position,
                              	GHashTable **attributes)
{
	GspellSuggestionMenuPrivate * priv;
	GHashTable * hash_table;

	priv = gspell_suggestion_menu_get_instance_private(GSPELL_SUGGESTION_MENU (model));

	gchar * suggestion =  g_slist_nth_data (priv->suggestions, position);

	hash_table = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (hash_table, g_strdup ( G_MENU_ATTRIBUTE_ACTION ), g_variant_ref_sink (g_variant_new_string ("spelling.correct")));
	g_hash_table_insert (hash_table, g_strdup (G_MENU_ATTRIBUTE_TARGET), g_variant_ref_sink (g_variant_new_string (suggestion)));
  	g_hash_table_insert (hash_table, g_strdup (G_MENU_ATTRIBUTE_LABEL), g_variant_ref_sink (g_variant_new_string (suggestion)));

	*attributes = hash_table;
}

static gboolean
gspell_suggestion_menu_is_mutable (GMenuModel *model)
{
  return TRUE;
}

static GMenuModel *
gspell_suggestion_menu_get_item_link (GMenuModel *model,
                                        int         position,
                                        const char *link)
{
  return NULL;
}

static void
gspell_suggestion_menu_get_item_links (GMenuModel  *model,
                                         int          position,
                                         GHashTable **links)
{
  *links = NULL;
}

static void
gspell_suggestion_menu_class_init(GspellSuggestionMenuClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GMenuModelClass *menu_model_class = G_MENU_MODEL_CLASS(klass);

	object_class->set_property = gspell_suggestion_menu_set_property;
	object_class->get_property = gspell_suggestion_menu_get_property;

	g_object_class_install_property(object_class, PROP_SUGGESTIONS,
					g_param_spec_pointer(
					"suggestions", "", "",
					G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE)
					);

	menu_model_class->get_n_items = gspell_suggestion_menu_get_n_items;
	menu_model_class->is_mutable = gspell_suggestion_menu_is_mutable;
	menu_model_class->get_item_link = gspell_suggestion_menu_get_item_link;
  	menu_model_class->get_item_links = gspell_suggestion_menu_get_item_links;
	menu_model_class->get_item_attributes = gspell_suggestion_menu_get_item_attributes;
}

static GSList *
gspell_suggestion_menu_get_suggestions(GspellSuggestionMenu * menu)
{
	GspellSuggestionMenuPrivate * priv;

	g_return_val_if_fail(menu != NULL, NULL);

	priv = gspell_suggestion_menu_get_instance_private(menu);

	return priv->suggestions;
}

void
gspell_suggestion_menu_set_suggestions(GspellSuggestionMenu * menu,
			    GSList * suggestions)
{
	GspellSuggestionMenuPrivate * priv;
	gint removed = 0;
	gint added = 0;

	g_return_if_fail(GSPELL_IS_SUGGESTION_MENU(menu));

	priv = gspell_suggestion_menu_get_instance_private(menu);

	if (priv->suggestions == suggestions)
		return;

	if (priv->suggestions != NULL)
		removed = g_slist_length (priv->suggestions);

	if (suggestions != NULL)
		added = g_slist_length (suggestions);

	priv->suggestions = suggestions;

	g_menu_model_items_changed (G_MENU_MODEL(menu), 0, removed, added);

	g_object_notify(G_OBJECT(menu), "suggestions");
}

static void
gspell_suggestion_menu_init(GspellSuggestionMenu * self)
{
}

GMenuModel *
gspell_suggestion_menu_new (void)
{
	return g_object_new (GSPELL_TYPE_SUGGESTION_MENU, NULL);
}

/* ex:set ts=8 noet: */
