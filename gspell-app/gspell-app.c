#include <gtk/gtk.h>
#include <gspell/gspell.h>

static void
activate(GtkApplication* app,
         gpointer user_data)
{
  GtkWidget *window;
  GtkTextView *gtk_view;
  GspellTextView *gspell_view;

  GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;


	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  gtk_view = GTK_TEXT_VIEW(gtk_text_view_new());
  gspell_view = gspell_text_view_get_from_gtk_text_view(gtk_view);
  gspell_text_view_basic_setup(gspell_view);

  gtk_buffer = gtk_text_view_get_buffer (gtk_view);

	checker = gspell_checker_new (lang);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	gspell_text_buffer_set_spell_checker (gspell_buffer, checker);


  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(gtk_view));

  gtk_widget_show(window);
}

int
main(int argc,
     char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

