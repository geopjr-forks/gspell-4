/* Do not edit: this file is generated from https://gitlab.gnome.org/GNOME/gtksourceview/raw/master/gtksourceview/gtksourceregion.h */

/*
 * This file is part of GtkSourceView
 *
 * Copyright 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright 2016 Sébastien Wilmet <swilmet@gnome.org>
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

#pragma once

#include <gspell/gspell-version.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GSPELL_TYPE_REGION (_gspell_region_get_type ())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GspellRegion, _gspell_region, GSPELL, REGION, GObject)

struct _GspellRegionClass
{
	GObjectClass parent_class;

	/*< private >*/
	gpointer _reserved[10];
};

/*
 * GspellRegionIter:
 *
 * An opaque datatype.
 *
 * Ignore all its fields and initialize the iter with [method@Region.get_start_region_iter].
 */
typedef struct _GspellRegionIter GspellRegionIter;
struct _GspellRegionIter
{
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;
};

GSPELL_AVAILABLE_IN_ALL
GspellRegion *_gspell_region_new                   (GtkTextBuffer       *buffer);
GSPELL_AVAILABLE_IN_ALL
GtkTextBuffer   *_gspell_region_get_buffer            (GspellRegion     *region);
GSPELL_AVAILABLE_IN_ALL
void             _gspell_region_add_subregion         (GspellRegion     *region,
                                                          const GtkTextIter   *_start,
                                                          const GtkTextIter   *_end);
GSPELL_AVAILABLE_IN_ALL
void             _gspell_region_add_region            (GspellRegion     *region,
                                                          GspellRegion     *region_to_add);
GSPELL_AVAILABLE_IN_ALL
void             _gspell_region_subtract_subregion    (GspellRegion     *region,
                                                          const GtkTextIter   *_start,
                                                          const GtkTextIter   *_end);
GSPELL_AVAILABLE_IN_ALL
void             _gspell_region_subtract_region       (GspellRegion     *region,
                                                          GspellRegion     *region_to_subtract);
GSPELL_AVAILABLE_IN_ALL
GspellRegion *_gspell_region_intersect_subregion   (GspellRegion     *region,
                                                          const GtkTextIter   *_start,
                                                          const GtkTextIter   *_end);
GSPELL_AVAILABLE_IN_ALL
GspellRegion *_gspell_region_intersect_region      (GspellRegion     *region1,
                                                          GspellRegion     *region2);
GSPELL_AVAILABLE_IN_ALL
gboolean         _gspell_region_is_empty              (GspellRegion     *region);
GSPELL_AVAILABLE_IN_ALL
gboolean         _gspell_region_get_bounds            (GspellRegion     *region,
                                                          GtkTextIter         *start,
                                                          GtkTextIter         *end);
GSPELL_AVAILABLE_IN_ALL
void             _gspell_region_get_start_region_iter (GspellRegion     *region,
                                                          GspellRegionIter *iter);
GSPELL_AVAILABLE_IN_ALL
gboolean         _gspell_region_iter_is_end           (GspellRegionIter *iter);
GSPELL_AVAILABLE_IN_ALL
gboolean         _gspell_region_iter_next             (GspellRegionIter *iter);
GSPELL_AVAILABLE_IN_ALL
gboolean         _gspell_region_iter_get_subregion    (GspellRegionIter *iter,
                                                          GtkTextIter         *start,
                                                          GtkTextIter         *end);
GSPELL_AVAILABLE_IN_ALL
gchar           *_gspell_region_to_string             (GspellRegion     *region);

G_END_DECLS
