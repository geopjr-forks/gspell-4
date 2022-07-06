if get_option('introspection')

dependency('gi-docgen', version: '>= 2021.1',
           fallback: ['gi-docgen', 'dummy_dep'],
           )

gidocgen = find_program('gi-docgen')

toml_data = configuration_data()
toml_data.set('VERSION', gspell_api_version)

libgspell_toml = configure_file(
  input: 'libgspell.toml.in',
  output: 'libgspell.toml',
  configuration: toml_data
)


docs_dir = gspell_datadir / 'doc'

custom_target('libgspell-doc',
  input: [libgspell_toml, gspeel_gir[0]],
  output: gspell_api_name,
  command: [
    gidocgen,
    'generate',
    '--quiet',
    '--add-include-path=@0@'.format(gspell_inc),
    '--config=@INPUT0@',
    '--no-namespace-dir',
    '--output-dir=@OUTPUT@',
    '--content-dir=@0@'.format(gspell_inc),
    '@INPUT1@',
  ],
  build_by_default: true,
  install: true,
  install_dir: docs_dir,
)

endif