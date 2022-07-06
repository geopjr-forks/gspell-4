subdir('xml')

doc_module = gspell_api_name

ignore_headers = [
  'gspell.h',
  'gspell-region.h',
  'gspell-checker-private.h',
  'gspell-context-menu.h',
  'gspell-current-word-policy.h',
  'gspell-entry-private.h',
  'gspell-entry-buffer.h',
  'gspell-entry.h',
  'gspell-entry-utils.h',
  'gspell-icu.h',
  'gspell-inline-checker-text-buffer.h',
  'gspell-osx.h',
  'gspell-text-iter.h',
  'gspell-utils.h',
  'gspell-version.h'
]


intro_xml = configuration_data()
intro_xml.set('GSPELL_API_VERSION', gspell_api_version)

doc_xml = configure_file(
  input: 'intro.xml.in',
  output: 'intro.xml',
  configuration: intro_xml,
  install: false,
)

glib_prefix = dependency('glib-2.0').get_variable(pkgconfig: 'prefix')
glib_docpath = glib_prefix / 'share/gtk-doc/html'
gtk_prefix = dependency('gtk+-3.0').get_variable(pkgconfig: 'prefix')
gtk_docpath = gtk_prefix / 'share/gtk-doc/html'
docpath = gspell_datadir / 'gtk-doc/html'


docpath = join_paths (gspell_gtkdocdir, 'html', doc_module)

gnome.gtkdoc(doc_module,
  main_xml: 'gspell-docs.xml',
  src_dir: gspell_inc,
  dependencies: libgspell_dep,
  gobject_typesfile: 'gspell.types',
  scan_args: [
    '--rebuild-types',
    '--ignore-headers=' + ' '.join(ignore_headers),
  ],
  mkdb_args: [
    '--output-format=xml',
  ],
  fixxref_args: [
    '--html-dir=@0@'.format(docpath),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(gtk_docpath, 'gtk3')),
  ],
  install: true,
)
