project('RhythmCat', 'c',
  version: '3.0.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>= 0.64.0',
  default_options: [
    'c_std=gnu11',
    'warning_level=2',
    'buildtype=debugoptimized',
  ],
)

gnome = import('gnome')
i18n = import('i18n')
pkgconfig = import('pkgconfig')

package_name = 'rhythmcat3'
cc = meson.get_compiler('c')

glib_dep = dependency('glib-2.0', version: '>= 2.74')
gobject_dep = dependency('gobject-2.0', version: '>= 2.74')
gio_dep = dependency('gio-2.0', version: '>= 2.74')
gst_dep = dependency('gstreamer-1.0', version: '>= 1.16')
gst_pbutils_dep = dependency('gstreamer-pbutils-1.0', version: '>= 1.16')
gst_tag_dep = dependency('gstreamer-tag-1.0', version: '>= 1.16')
json_dep = dependency('json-c', version: '>= 0.15')
gst_audio_dep = dependency('gstreamer-audio-1.0', version: '>= 1.16')

conf = configuration_data()
conf.set_quoted('GETTEXT_PACKAGE', package_name)
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))
conf.set_quoted('APPLICATION_ID', 'org.supercat.RhythmCat3')
configure_file(output: 'config.h', configuration: conf)
config_inc = include_directories('.')

add_project_arguments(
  '-DHAVE_CONFIG_H',
  '-Wno-unused-parameter',
  '-Wno-missing-field-initializers',
  '-Wno-cast-function-type',
  language: 'c',
)

gtk_dep = dependency('gtk4', version: '>= 4.12', required: get_option('gtk'))
ncurses_dep = dependency('ncursesw', required: get_option('ncurses'))

subdir('lib')
subdir('data')
subdir('po')
subdir('tests')

if gtk_dep.found()
  subdir('ui-gtk')
endif

if ncurses_dep.found()
  subdir('ui-tui')
endif

summary({
  'GTK4 frontend': gtk_dep.found(),
  'ncurses frontend': ncurses_dep.found(),
}, section: 'RhythmCat')
