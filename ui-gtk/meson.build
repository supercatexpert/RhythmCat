rc_ui_resources = gnome.compile_resources('rc-ui-resources',
  'resources' / 'rc-ui.gresource.xml',
  source_dir: ['resources', '..' / 'data' / 'artwork'],
  c_name: 'rc_ui',
)

rc_ui_deps = [rclib_dep, gtk_dep]
rc_ui_args = ['-DG_LOG_DOMAIN="RhythmCat3-GTK"']
# Xlib is used to place the OSD lyric window above other windows on X11
x11_dep = dependency('x11', required: false)
if x11_dep.found() and 'x11' in gtk_dep.get_variable(pkgconfig: 'targets',
    default_value: '').split()
  rc_ui_deps += x11_dep
  rc_ui_args += '-DRC_UI_HAVE_X11'
endif

executable('rhythmcat',
  'main.c',
  'rc-ui-equalizer.c',
  'rc-ui-info.c',
  'rc-ui-main.c',
  'rc-ui-osd.c',
  'rc-ui-settings.c',
  'rc-ui-style.c',
  'rc-ui-track.c',
  'rc-ui-window.c',
  rc_ui_resources,
  dependencies: rc_ui_deps,
  include_directories: config_inc,
  c_args: rc_ui_args,
  install: true,
)
