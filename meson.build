project('gspell', 'c',
        version: '1.11.1',
        license: 'LGPL-2.1-or-later',
        default_options: [
          'buildtype=debugoptimized',
          'warning_level=1',
          'c_std=c99',
        ],
        meson_version : '>= 0.56.2')

cc = meson.get_compiler('c')
host_system = host_machine.system()

gspell_url=''
gspell_bugreport=''

version = meson.project_version().split('.')
gspell_major_version = version[0].to_int()
gspell_minor_version = version[1].to_int()
gspell_micro_version = version[2].to_int()

gspell_lt_current='5'
gspell_lt_revision='1'
gspell_lt_age='3'
gspell_lt_version='@0@:@1@:@2@'.format(gspell_lt_current, gspell_lt_revision, gspell_lt_age)

gspell_api_version='1'
gspell_api_name = 'gspell-@0@'.format(gspell_api_version)
gspell_api_path = '@0@/gspell'.format(gspell_api_name)

gspell_prefix = get_option('prefix')
gspell_libdir = join_paths(gspell_prefix, get_option('libdir'))
gspell_sysconfdir = join_paths(gspell_prefix, get_option('sysconfdir'))
gspell_includedir = join_paths(gspell_prefix, get_option('includedir'))
gspell_datadir = join_paths(gspell_prefix, get_option('datadir'))
gspell_gtkdocdir = join_paths(gspell_datadir, 'gtk-doc')
gspell_libexecdir = join_paths(gspell_prefix, get_option('libexecdir'))

gspell_conf = configuration_data()

gspell_conf.set_quoted('VERSION', meson.project_version())
gspell_conf.set_quoted('GETTEXT_PACKAGE', gspell_api_name)
gspell_conf.set_quoted('PACKAGE_BUGREPORT', gspell_bugreport)

add_project_arguments([], language: 'c')

platform_osx=false

# Check for Mac OS X platform
# FIXME: add parements from Mac OS X compiler
if host_machine.system() == 'darwin'
  platform_osx=true
endif

# Dependencies
glib_req_version	= '>= 2.44'
gtk_req_version  	= '>= 4.6.5'
enchant_req_version	= '>= 2.1.1'
ic_req_version		= '>= 71.1'

glib_dep = [dependency('glib-2.0', version: glib_req_version,
                          fallback : ['glib', 'libglib_dep']),
            dependency('gobject-2.0', version: glib_req_version,
                          fallback : ['glib', 'libgobject_dep'])]

gtk_dep = [dependency('gtk4', version: gtk_req_version)]

enchant_dep = [dependency('enchant-2', version: enchant_req_version)]

icu_dep = [dependency('icu-uc', version: ic_req_version)]

pkgconfig = import('pkgconfig')
gnome = import('gnome')

# Internal configuration header
configure_file(output: 'config.h', configuration: gspell_conf)

root_inc = include_directories('.')
gspell_inc = include_directories('gspell')

subdir('gspell')

if get_option('demo')
  subdir('gspell-app')
endif

subdir('po')

if get_option('tests')
  subdir('tests')
endif

if get_option('docs')
  subdir('docs')
endif