ent_conf = configuration_data()
ent_conf.set('PACKAGE', meson.project_name())
ent_conf.set('PACKAGE_BUGREPORT', gspell_bugreport)
ent_conf.set('PACKAGE_NAME', meson.project_name())
ent_conf.set('PACKAGE_STRING', meson.project_name())
ent_conf.set('PACKAGE_TARNAME', '@0@-@1@'.format(meson.project_name(), meson.project_version()))
ent_conf.set('PACKAGE_URL',  gspell_url)
ent_conf.set('PACKAGE_VERSION', meson.project_version())
ent_conf.set('PACKAGE_API_VERSION', gspell_api_version)
configure_file(
  input: 'gtkdocentities.ent.in',
  output: 'gtkdocentities.ent',
  configuration: ent_conf,
)
