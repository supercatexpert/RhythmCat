rclib_headers = files(
  'rclib.h',
  'rclib-core.h',
  'rclib-cue.h',
  'rclib-db.h',
  'rclib-instance.h',
  'rclib-lyric.h',
  'rclib-player.h',
  'rclib-settings.h',
  'rclib-tag.h',
  'rclib-util.h',
)

rclib_sources = files(
  'rclib.c',
  'rclib-core.c',
  'rclib-cue.c',
  'rclib-db.c',
  'rclib-instance.c',
  'rclib-lyric.c',
  'rclib-player.c',
  'rclib-settings.c',
  'rclib-tag.c',
  'rclib-util.c',
)

rclib_deps = [
  glib_dep,
  gobject_dep,
  gio_dep,
  gst_dep,
  gst_pbutils_dep,
  gst_tag_dep,
  gst_audio_dep,
  json_dep,
]

rclib = shared_library('rhythmcat3',
  rclib_sources,
  version: meson.project_version(),
  soversion: '3',
  dependencies: rclib_deps,
  include_directories: config_inc,
  c_args: ['-DG_LOG_DOMAIN="RhythmCat3"'],
  install: true,
)

install_headers(rclib_headers, subdir: 'rhythmcat3')

pkgconfig.generate(rclib,
  name: 'rhythmcat3',
  description: 'RhythmCat music player core library',
  subdirs: 'rhythmcat3',
  requires: ['glib-2.0', 'gobject-2.0', 'gio-2.0', 'gstreamer-1.0'],
)

rclib_dep = declare_dependency(
  link_with: rclib,
  include_directories: include_directories('.'),
  dependencies: [glib_dep, gobject_dep, gio_dep, gst_dep],
)
