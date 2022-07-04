/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016, 2017 - Sébastien Wilmet
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gspell-text-view.h"
#include <glib/gi18n-lib.h>
#include "gspell-inline-checker-text-buffer.h"
#include "gspell-checker.h"
#include "gspell-language.h"
#include "gspell-text-buffer.h"
#include "gspell-suggestion-menu.h"

/**
 * SECTION:text-view
 * @Title: GspellTextView
 * @Short_description: Spell checking support for GtkTextView
 *
 * #GspellTextView extends the #GtkTextView class with inline spell checking.
 * Misspelled words are highlighted with a red %PANGO_UNDERLINE_SINGLE.
 * Right-clicking a misspelled word pops up a context menu of suggested
 * replacements. The context menu also contains an “Ignore All” item to add the
 * misspelled word to the session dictionary. And an “Add” item to add the word
 * to the personal dictionary.
 *
 * For a basic use-case, there is the gspell_text_view_basic_setup() convenience
 * function.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * If you don't use the gspell_text_view_basic_setup() function, you need to
 * call gspell_text_buffer_set_spell_checker() to associate a #GspellChecker to
 * the #GtkTextBuffer.
 *
 * Note that #GspellTextView extends the #GtkTextView class but without
 * subclassing it, because the GtkSourceView library has already a #GtkTextView
 * subclass.
 *
 * If you want a %PANGO_UNDERLINE_ERROR instead (a wavy underline), please fix
 * [this bug](https://bugzilla.gnome.org/show_bug.cgi?id=763741) first.
 */

typedef struct _GspellTextViewPrivate GspellTextViewPrivate;

struct _GspellTextViewPrivate {
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *inline_checker;
	guint enable_language_menu : 1;

	/* Addiction menu with language selector and spelling suggestions
	 * so you don't have to recreate it every time you change the "language-menu" property */
	GMenuModel * gspell_extra_menu;

	/* Binding menu to the "extra-menu" of GtkTextView.
	 * Need to hide or show the gpsell_extra_menu
	 * NULL when property "language-menu" is false*/
	GMenuModel * extra_menu;

	GMenuModel  * suggestions_menu;

	gchar * misspelled_word;
};

enum {
	PROP_0,
	PROP_VIEW,
	PROP_EXTRA_MENU,
	PROP_INLINE_SPELL_CHECKING,
	PROP_ENABLE_LANGUAGE_MENU,
};

#define GSPELL_TEXT_VIEW_KEY "gspell-text-view-key"

G_DEFINE_TYPE_WITH_PRIVATE(GspellTextView, gspell_text_view, G_TYPE_OBJECT)

static GMenuModel * gspell_get_language_menu_new();

static void
create_inline_checker(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->inline_checker != NULL) {
		return;
	}

	buffer = gtk_text_view_get_buffer(priv->view);
	priv->inline_checker = _gspell_inline_checker_text_buffer_new(buffer);
	_gspell_inline_checker_text_buffer_attach_view(priv->inline_checker,
						       priv->view);
}

static void
gspell_text_view_set_extra_menu(GspellTextView *gspell_view,
				GMenuModel * extra_menu)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->extra_menu == extra_menu)
		return;

	priv->extra_menu = extra_menu;

	g_object_notify(G_OBJECT(gspell_view), "extra-menu");
}

static void
destroy_inline_checker(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->view == NULL || priv->inline_checker == NULL) {
		return;
	}

	_gspell_inline_checker_text_buffer_detach_view(priv->inline_checker,
						       priv->view);

	g_clear_object(&priv->inline_checker);
}

static void
notify_buffer_cb(GtkTextView    *gtk_view,
		 GParamSpec     *pspec,
		 GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->inline_checker == NULL) {
		return;
	}


	destroy_inline_checker(gspell_view);
	create_inline_checker(gspell_view);
}

static void
language_activated_cb(const GspellLanguage *lang,
		      gpointer user_data)
{
	GspellTextView *gspell_view;
	GspellTextViewPrivate *priv;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;

	g_return_if_fail(GSPELL_IS_TEXT_VIEW(user_data));

	gspell_view = GSPELL_TEXT_VIEW(user_data);
	priv = gspell_text_view_get_instance_private(gspell_view);

	gtk_buffer = gtk_text_view_get_buffer(priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer(gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker(gspell_buffer);

	gspell_checker_set_language(checker, lang);
}

static const GspellLanguage *
get_current_language(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->view == NULL) {
		return NULL;
	}

	gtk_buffer = gtk_text_view_get_buffer(priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer(gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker(gspell_buffer);
	return gspell_checker_get_language(checker);
}

static void
spelling_ignore_all(GSimpleAction *action,
		    GVariant      *value,
		    gpointer user_data)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(GSPELL_TEXT_VIEW(user_data));

	gtk_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer(gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker(gspell_buffer);

	gspell_checker_add_word_to_session(checker, priv->misspelled_word, -1);
}

static void
spelling_add(GSimpleAction *action,
	     GVariant      *value,
	     gpointer user_data)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(GSPELL_TEXT_VIEW(user_data));

	gtk_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer(gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker(gspell_buffer);

	gspell_checker_add_word_to_personal(checker, priv->misspelled_word, -1);
}

static void
spelling_correct(GSimpleAction *action,
		 GVariant      *value,
		 gpointer user_data)
{
	GspellTextViewPrivate *priv;
	priv = gspell_text_view_get_instance_private(GSPELL_TEXT_VIEW(user_data));

	const char *correct;

	correct = g_variant_get_string(value, NULL);

	_gspell_inline_checker_text_buffer_correct(priv->inline_checker, correct);
}

static GSimpleActionGroup *action_group = NULL;

static void
change_default_language(GSimpleAction *action,
			GVariant *value,
			gpointer user_data)
{
	const char *code;

	code = g_variant_get_string(value, NULL);

	g_simple_action_set_state(action, value);

	language_activated_cb(gspell_language_lookup(code), GSPELL_TEXT_VIEW(user_data));
}

static void
gspell_suggestions_action_set_enable(gboolean enabled)
{
	GAction * action;

	action = g_action_map_lookup_action(G_ACTION_MAP(action_group), "has-suggestions");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);

	action = g_action_map_lookup_action(G_ACTION_MAP(action_group), "add");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);

	action = g_action_map_lookup_action(G_ACTION_MAP(action_group), "ignore-all");
	g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);
}

static void
add_actions(GtkTextView    *gtk_view,
	    GspellTextView *gspell_view)
{
	action_group = g_simple_action_group_new();

	GActionEntry entries[] = {
		{ "has-suggestions", NULL },
		{ "language", NULL, "s", "''", change_default_language },
		{ "correct", spelling_correct, "s" },
		{ "add", spelling_add },
		{ "ignore-all", spelling_ignore_all },
	};

	g_action_map_add_action_entries(G_ACTION_MAP(action_group), entries, G_N_ELEMENTS(entries), gspell_view);

	gspell_suggestions_action_set_enable(FALSE);

	gtk_widget_insert_action_group(GTK_WIDGET(gtk_view), "spelling", G_ACTION_GROUP(action_group));
}

/* When the user right-clicks on a word, they want to check that word.
 * Here, we do NOT move the cursor to the location of the clicked-upon word
 * since that prevents the use of edit functions on the context menu.
 */
static void
on_pressed_cb(GtkGestureClick* self,
	      gint n_press,
	      gdouble x,
	      gdouble y,
	      gpointer user_data)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(GSPELL_TEXT_VIEW(user_data));

	priv->misspelled_word = NULL;

	if (priv->enable_language_menu &&
	    priv->inline_checker != NULL) {
		priv->misspelled_word = gspell_inline_checker_get_word_at_click_position(priv->inline_checker);

		if (priv->misspelled_word != NULL) {
			GSList * suggestions = gspell_inline_checker_get_suggestions(priv->inline_checker, priv->misspelled_word);

			gspell_suggestion_menu_set_suggestions(GSPELL_SUGGESTION_MENU(priv->suggestions_menu), suggestions);
		}
	}

	gspell_suggestions_action_set_enable(priv->misspelled_word != NULL);
}

static GMenuModel *
gspell_text_view_extra_menu_new(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(gspell_view);

	if (priv->suggestions_menu == NULL)
		priv->suggestions_menu = gspell_suggestion_menu_new();

	g_autoptr(GMenu) top_menu = g_menu_new();

	g_autoptr(GMenu) language_menu = g_menu_new();
	g_autoptr(GMenuItem) languages_item = g_menu_item_new_submenu(_("_Language"), G_MENU_MODEL(gspell_get_language_menu_new()));
	g_menu_append_item(language_menu, languages_item);

	g_autoptr(GMenuItem) suggestions_items = g_menu_item_new_submenu(_("_Spelling Suggestions…"), G_MENU_MODEL(priv->suggestions_menu));
  g_menu_item_set_detailed_action(suggestions_items, "spelling.has-suggestions");

	g_autoptr(GMenuItem) add_item = g_menu_item_new(_("_Add"), "spelling.add");
	g_autoptr(GMenuItem) ignore_item = g_menu_item_new(_("_Ignore All"), "spelling.ignore-all");

	g_autoptr(GMenu) spelling_menu = g_menu_new();
	g_menu_append_item(spelling_menu, suggestions_items);
	g_menu_append_item(spelling_menu, add_item);
	g_menu_append_item(spelling_menu, ignore_item);

	g_menu_append_section(top_menu, NULL, G_MENU_MODEL(language_menu));
	g_menu_append_section(top_menu, NULL, G_MENU_MODEL(spelling_menu));

	return G_MENU_MODEL(g_steal_pointer(&top_menu));
}

static void
set_view(GspellTextView *gspell_view,
	 GtkTextView    *gtk_view)
{
	GspellTextViewPrivate *priv;
	GtkGesture * gesture_click;

	g_return_if_fail(GTK_IS_TEXT_VIEW(gtk_view));

	priv = gspell_text_view_get_instance_private(gspell_view);

	g_assert(priv->view == NULL);
	g_assert(priv->inline_checker == NULL);

	priv->view = gtk_view;

	g_signal_connect_object(priv->view,
				"notify::buffer",
				G_CALLBACK(notify_buffer_cb),
				gspell_view,
				0);

	add_actions(gtk_view, gspell_view);

	g_object_bind_property(gspell_view, "extra-menu", priv->view, "extra-menu", G_BINDING_SYNC_CREATE);

	gesture_click = gtk_gesture_click_new();

	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture_click), 3);

	g_signal_connect_object(gesture_click, "pressed", G_CALLBACK(on_pressed_cb), gspell_view, 0);

	gtk_widget_add_controller(GTK_WIDGET(priv->view), GTK_EVENT_CONTROLLER(gesture_click));

	g_object_notify(G_OBJECT(gspell_view), "view");
}

static void
gspell_text_view_get_property(GObject    *object,
			      guint prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GspellTextViewPrivate *priv;
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW(object);

	priv = gspell_text_view_get_instance_private(gspell_view);

	switch (prop_id) {
	case PROP_VIEW:
		g_value_set_object(value, gspell_text_view_get_view(gspell_view));
		break;

	case PROP_EXTRA_MENU:
		g_value_set_object(value, priv->extra_menu);
		break;

	case PROP_INLINE_SPELL_CHECKING:
		g_value_set_boolean(value, gspell_text_view_get_inline_spell_checking(gspell_view));
		break;

	case PROP_ENABLE_LANGUAGE_MENU:
		g_value_set_boolean(value, gspell_text_view_get_enable_language_menu(gspell_view));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
gspell_text_view_set_property(GObject      *object,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GspellTextViewPrivate *priv;
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW(object);

	priv = gspell_text_view_get_instance_private(gspell_view);

	switch (prop_id) {
	case PROP_VIEW:
		set_view(gspell_view, g_value_get_object(value));
		break;
	case PROP_EXTRA_MENU:
		gspell_text_view_set_extra_menu(gspell_view, g_value_get_object(value));
	case PROP_INLINE_SPELL_CHECKING:
		gspell_text_view_set_inline_spell_checking(gspell_view, g_value_get_boolean(value));
		break;

	case PROP_ENABLE_LANGUAGE_MENU:
		gspell_text_view_set_enable_language_menu(gspell_view, g_value_get_boolean(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
gspell_text_view_dispose(GObject *object)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private(GSPELL_TEXT_VIEW(object));

	if (priv->view != NULL && priv->inline_checker != NULL) {
		_gspell_inline_checker_text_buffer_detach_view(priv->inline_checker,
							       priv->view);
	}

	priv->view = NULL;
	g_clear_object(&priv->inline_checker);

	G_OBJECT_CLASS(gspell_text_view_parent_class)->dispose(object);
}

static void
gspell_text_view_class_init(GspellTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = gspell_text_view_get_property;
	object_class->set_property = gspell_text_view_set_property;
	object_class->dispose = gspell_text_view_dispose;

	/**
	 * GspellTextView:view:
	 *
	 * The #GtkTextView.
	 */
	g_object_class_install_property(object_class,
					PROP_VIEW,
					g_param_spec_object("view",
							    "View",
							    "",
							    GTK_TYPE_TEXT_VIEW,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));


	g_object_class_install_property(object_class,
					PROP_EXTRA_MENU,
					g_param_spec_object("extra-menu",
							    "extra-menu",
							    "extra-menu",
							    G_TYPE_MENU_MODEL,
							    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE));

	/**
	 * GspellTextView:inline-spell-checking:
	 *
	 * Whether the inline spell checking is enabled.
	 */
	g_object_class_install_property(object_class,
					PROP_INLINE_SPELL_CHECKING,
					g_param_spec_boolean("inline-spell-checking",
							     "Inline Spell Checking",
							     "",
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * GspellTextView:enable-language-menu:
	 *
	 * When the context menu is shown, whether to add a sub-menu to select
	 * the language for the spell checking.
	 *
	 * Since: 1.2
	 */
	g_object_class_install_property(object_class,
					PROP_ENABLE_LANGUAGE_MENU,
					g_param_spec_boolean("enable-language-menu",
							     "Enable Language Menu",
							     "",
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_STATIC_STRINGS));
}

GMenuModel *
gspell_get_language_menu_new()
{
	g_autoptr(GMenu) menu = g_menu_new();
	const GList * l;
	const GList * languages = gspell_language_get_available();
	for (l = languages; l != NULL; l = l->next) {
		const GspellLanguage *lang = l->data;
		const gchar *lang_name;
		const gchar *code;
		g_autoptr(GMenuItem) menu_item;

		lang_name = gspell_language_get_name(lang);
		code = gspell_language_get_code(lang);
		menu_item = g_menu_item_new(lang_name, NULL);
		g_menu_item_set_action_and_target(menu_item, "spelling.language", "s", code);
		g_menu_append_item(menu, menu_item);
	}

	return G_MENU_MODEL(g_steal_pointer(&menu));
}

static void
gspell_text_view_init(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view));

	priv = gspell_text_view_get_instance_private(gspell_view);

	priv->gspell_extra_menu = gspell_text_view_extra_menu_new(gspell_view);
}

/**
 * gspell_text_view_get_from_gtk_text_view:
 * @gtk_view: a #GtkTextView.
 *
 * Returns the #GspellTextView of @gtk_view. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_view.
 *
 * Returns: (transfer none): the #GspellTextView of @gtk_view.
 */
GspellTextView *
gspell_text_view_get_from_gtk_text_view(GtkTextView *gtk_view)
{
	GspellTextView *gspell_view;

	g_return_val_if_fail(GTK_IS_TEXT_VIEW(gtk_view), NULL);

	gspell_view = g_object_get_data(G_OBJECT(gtk_view), GSPELL_TEXT_VIEW_KEY);

	if (gspell_view == NULL) {
		gspell_view = g_object_new(GSPELL_TYPE_TEXT_VIEW,
					   "view", gtk_view,
					   NULL);

		g_object_set_data_full(G_OBJECT(gtk_view),
				       GSPELL_TEXT_VIEW_KEY,
				       gspell_view,
				       g_object_unref);
	}

	g_return_val_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view), NULL);
	return gspell_view;
}

/**
 * gspell_text_view_basic_setup:
 * @gspell_view: a #GspellTextView.
 *
 * This function is a convenience function that does the following:
 * - Set a spell checker. The language chosen is the one returned by
 *   gspell_language_get_default().
 * - Set the #GspellTextView:inline-spell-checking property to %TRUE.
 * - Set the #GspellTextView:enable-language-menu property to %TRUE.
 *
 * Example:
 * |[
 * GtkTextView *gtk_view;
 * GspellTextView *gspell_view;
 *
 * gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
 * gspell_text_view_basic_setup (gspell_view);
 * ]|
 *
 * This is equivalent to:
 * |[
 * GtkTextView *gtk_view;
 * GspellTextView *gspell_view;
 * GspellChecker *checker;
 * GtkTextBuffer *gtk_buffer;
 * GspellTextBuffer *gspell_buffer;
 *
 * checker = gspell_checker_new (NULL);
 * gtk_buffer = gtk_text_view_get_buffer (gtk_view);
 * gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
 * gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
 * g_object_unref (checker);
 *
 * gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
 * gspell_text_view_set_inline_spell_checking (gspell_view, TRUE);
 * gspell_text_view_set_enable_language_menu (gspell_view, TRUE);
 * ]|
 *
 * Since: 1.2
 */
void
gspell_text_view_basic_setup(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GspellChecker *checker;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;

	g_return_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view));

	priv = gspell_text_view_get_instance_private(gspell_view);

	checker = gspell_checker_new(NULL);
	gtk_buffer = gtk_text_view_get_buffer(priv->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer(gtk_buffer);
	gspell_text_buffer_set_spell_checker(gspell_buffer, checker);
	g_object_unref(checker);

	gspell_text_view_set_inline_spell_checking(gspell_view, TRUE);
	gspell_text_view_set_enable_language_menu(gspell_view, TRUE);
}

/**
 * gspell_text_view_get_view:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: (transfer none): the #GtkTextView of @gspell_view.
 */
GtkTextView *
gspell_text_view_get_view(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view), NULL);

	priv = gspell_text_view_get_instance_private(gspell_view);
	return priv->view;
}

/**
 * gspell_text_view_get_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: whether the inline spell checking is enabled.
 */
gboolean
gspell_text_view_get_inline_spell_checking(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view), FALSE);

	priv = gspell_text_view_get_instance_private(gspell_view);
	return priv->inline_checker != NULL;
}

/**
 * gspell_text_view_set_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 * @enable: the new state.
 *
 * Enables or disables the inline spell checking.
 */
void
gspell_text_view_set_inline_spell_checking(GspellTextView *gspell_view,
					   gboolean enable)
{
	g_return_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view));

	enable = enable != FALSE;

	if (enable == gspell_text_view_get_inline_spell_checking(gspell_view)) {
		return;
	}

	if (enable) {
		create_inline_checker(gspell_view);
	}else {
		destroy_inline_checker(gspell_view);
	}

	g_object_notify(G_OBJECT(gspell_view), "inline-spell-checking");
}

/**
 * gspell_text_view_get_enable_language_menu:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: whether the language context menu is enabled.
 * Since: 1.2
 */
gboolean
gspell_text_view_get_enable_language_menu(GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view), FALSE);

	priv = gspell_text_view_get_instance_private(gspell_view);
	return priv->enable_language_menu;
}

/**
 * gspell_text_view_set_enable_language_menu:
 * @gspell_view: a #GspellTextView.
 * @enable_language_menu: whether to enable the language context menu.
 *
 * Sets whether to enable the language context menu. If enabled, doing a right
 * click on the #GtkTextView will show a sub-menu to choose the language for the
 * spell checking. If another language is chosen, it changes the
 * #GspellChecker:language property of the #GspellTextBuffer:spell-checker of
 * the #GtkTextView:buffer of the #GspellTextView:view.
 *
 * Since: 1.2
 */
void
gspell_text_view_set_enable_language_menu(GspellTextView *gspell_view,
					  gboolean enable_language_menu)
{
	GspellTextViewPrivate *priv;

	GMenu * spelling_menu;
	g_return_if_fail(GSPELL_IS_TEXT_VIEW(gspell_view));

	priv = gspell_text_view_get_instance_private(gspell_view);

	enable_language_menu = enable_language_menu != FALSE;


	spelling_menu = g_menu_new();
	if (priv->enable_language_menu == enable_language_menu)
		return;

	priv->enable_language_menu = enable_language_menu;

	if (priv->enable_language_menu) {
		const GspellLanguage *current_language;
		GAction *action;
		GVariant *value;

		current_language = get_current_language(gspell_view);

		action = g_action_map_lookup_action(G_ACTION_MAP(action_group), "language");
		value = g_variant_ref_sink(g_variant_new_string(gspell_language_get_code(current_language)));
		change_default_language(G_SIMPLE_ACTION(action), value, gspell_view);
		g_variant_unref(value);
	}

	gspell_text_view_set_extra_menu(gspell_view,
					priv->enable_language_menu ?
					priv->gspell_extra_menu :
					NULL);

	g_object_notify(G_OBJECT(gspell_view), "enable-language-menu");
}

/* ex:set ts=8 noet: */




