gspell_app_name = 'gspell-app@0@'.format(gspell_api_version)

executable(gspell_app_name,
  [ 'gspell-app.c' ],
  dependencies: [ libgspell_dep ],
  c_args: [ '-DG_LOG_DOMAIN="gspell-app"',
	  '-DGSPELL_DATADIR="@0@"'.format(gspell_datadir),
	  '-DHAVE_CONFIG_H'
	  ],
  install: true
)