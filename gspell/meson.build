subdir('resources')

gspell_sources += [
  'gspell-checker.c',
  'gspell-checker-dialog.c',
  'gspell-current-word-policy.c',
  # 'gspell-entry-buffer.c',
  # 'gspell-entry.c',
  # 'gspell-entry-utils.c',
  'gspell-icu.c',
  'gspell-init.c',
  'gspell-inline-checker-text-buffer.c',
  'gspell-language.c',
  'gspell-language-chooser-button.c',
  'gspell-language-chooser.c',
  'gspell-language-chooser-dialog.c',
  'gspell-navigator.c',
  'gspell-navigator-text-view.c',
  'gspell-region.c',
  'gspell-text-buffer.c',
  'gspell-text-iter.c',
  'gspell-text-view.c',
  'gspell-utils.c',
  'gspell-menu.c',
  'enchant/gspell-enchant-checker.c',
]

gspell_headers = [
  'gspell.h',
  'gspell-checker.h',
  'gspell-checker-dialog.h',
  # 'gspell-entry.h',
  # 'gspell-entry-buffer.h',
  'gspell-language.h',
  'gspell-language-chooser.h',
  'gspell-language-chooser-button.h',
  'gspell-language-chooser-dialog.h',
  'gspell-navigator.h',
  'gspell-navigator-text-view.h',
  'gspell-text-buffer.h',
  'gspell-text-view.h',
  'enchant/gspell-enchant-checker.h'
]

install_headers(gspell_headers, subdir: gspell_api_path)

# Features header
gspell_version_conf = configuration_data()
gspell_version_conf.set('GSPELL_MAJOR_VERSION',gspell_major_version)
gspell_version_conf.set('GSPELL_MINOR_VERSION',gspell_minor_version)
gspell_version_conf.set('GSPELL_MICRO_VERSION',gspell_micro_version)
gspell_version_conf.set('LT_CURRENT_MINUS_AGE', '0')
gspell_version_conf.set('GSPELL_VERSION', meson.project_version())

if platform_osx
   gspell_sources += [ 'gspell-osx.c' ]
   cocoa_dep = dependency('appleframeworks', modules: 'Cocoa', required: false)
   assert(cocoa_dep.found(), 'Dependency appleframeworks not found')
endif

gspell_version_h = configure_file(input: 'gspell-version.h.in',
  output: 'gspell-version.h',
  configuration: gspell_version_conf,
  install: true,
  install_dir: join_paths(gspell_includedir, gspell_api_path),
)

# Enumerations for GType
gspell_enums = gnome.mkenums('gspell-enum-types',
  sources: gspell_headers,
  c_template: 'gspell-enum-types.c.in',
  h_template: 'gspell-enum-types.h.in',
  install_dir: join_paths(gspell_includedir, gspell_api_path),
  install_header: true,
)
gspell_enum_h = gspell_enums[1]

gspell_cflags = [
  '-DG_LOG_DOMAIN="gspell"',
  '-DHAVE_CONFIG_H',
  '-DGSPELL_COMPILATION',
  '-DDATADIR="@0@"'.format(gspell_datadir),
]

libgspell = library(gspell_api_name,
  sources: gspell_sources + gspell_enums,
  install: true,
  dependencies: [glib_dep, gtk_dep, enchant_dep, icu_dep],
  include_directories: [ root_inc, gspell_inc ],
  c_args: gspell_cflags
)

gspeel_sources_dep = [gspell_enum_h]

# Introspection
if get_option('introspection')
  gspeel_gir = gnome.generate_gir(libgspell,
    sources: gspell_sources + gspell_headers + [ gspell_enum_h ] + [ gspell_version_h ],
    namespace: 'Gspell',
    nsversion: gspell_api_version,
    identifier_prefix: 'Gspell',
    symbol_prefix: 'gspell',
    export_packages: gspell_api_name,
    includes: [ 'GObject-2.0', 'Gtk-4.0' ],
    install: true,
    extra_args: [
      '--quiet',
      '--c-include=gspell/gspell.h',
      '-DGSPELL_COMPILATION',
    ],
  )

  gspeel_sources_dep += gspeel_gir

  if get_option('vapi')
    gspell_vapi_deps = [
      'gio-2.0',
      'glib-2.0',
      'gtk4'
    ]

    gspeel_vapi_dep = gnome.generate_vapi(
      gspell_api_name,
      sources: gspeel_gir[0],
      packages: gspell_vapi_deps,
      install: true
    )
  endif
endif

libgspell_dep = declare_dependency(link_with: libgspell,
  include_directories: [ root_inc, gspell_inc ],
  dependencies: [glib_dep, gtk_dep, enchant_dep, icu_dep],
  sources: gspeel_sources_dep,
)

pkgconfig.generate(libgspell,
  name: gspell_api_name,
  filebase: gspell_api_name,
  description: 'Spell-checking for GTK applications',
  subdirs: gspell_api_name,
  requires: [glib_dep, gtk_dep, enchant_dep],
  requires_private: [icu_dep]
)
