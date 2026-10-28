executable('rhythmcat-tui',
  'main.c',
  'rc-tui.c',
  dependencies: [rclib_dep, ncurses_dep],
  include_directories: config_inc,
  c_args: ['-DG_LOG_DOMAIN="RhythmCat3-TUI"', '-D_XOPEN_SOURCE_EXTENDED'],
  install: true,
)
