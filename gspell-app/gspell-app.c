#include <gtk/gtk.h>
#include <gspell/gspell.h>

static void
activate(GtkApplication* app,
         gpointer user_data)
{
  GtkWidget *window;
  GtkWidget * titlebar;
  GtkWidget * scrolled_window;
  GtkTextView *gtk_view;
  GspellTextView *gspell_view;

  window = gtk_application_window_new(app);
  titlebar = gtk_header_bar_new();

  gtk_header_bar_set_title_widget (GTK_HEADER_BAR (titlebar), gtk_label_new("GSpell APP DEMO"));


  gtk_window_set_titlebar (GTK_WINDOW(window), titlebar);

	gtk_view = GTK_TEXT_VIEW (gtk_text_view_new ());
	gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
	gspell_text_view_basic_setup (gspell_view);

	scrolled_window = gtk_scrolled_window_new ();
  gtk_widget_set_vexpand (scrolled_window, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET (gtk_view));


  gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(scrolled_window));

  gtk_widget_show(window);
}

int
main(int argc,
     char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gspell.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
