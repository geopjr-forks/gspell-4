#include <gspell/gspell.h>

gint
main (gint    argc,
      gchar **argv)
{

  GspellChecker *checker;
	const GspellLanguage *language;
  gboolean correctly_spelled;

  language = gspell_language_lookup ("en_US");
  checker = gspell_enchant_checker_new (NULL);

  GError *error = NULL;

  correctly_spelled = gspell_checker_check_word (checker, "helxi", -1, &error);

  printf("correctly_spelled: %d\n", correctly_spelled);

  GSList * l;
  GSList * suggestions = gspell_checker_get_suggestions (checker, "helxi", -1);

  for (l = suggestions; l != NULL ; l = l->next)
    {
      /* printf("%s\n", (gchar *) l->data); */
    }

  gspell_checker_add_word_to_personal (checker, "helxi", -1);

  return 0;
}
