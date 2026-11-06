install_data('artwork' / 'rhythmcat-icon.svg',
  rename: 'org.supercat.RhythmCat3.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

if gtk_dep.found()
  install_man('rhythmcat.1')
  i18n.merge_file(
    input: 'org.supercat.RhythmCat3.desktop.in',
    output: 'org.supercat.RhythmCat3.desktop',
    type: 'desktop',
    po_dir: meson.project_source_root() / 'po',
    install: true,
    install_dir: get_option('datadir') / 'applications',
  )
endif

if ncurses_dep.found()
  install_man('rhythmcat-tui.1')
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  update_desktop_database: gtk_dep.found(),
)
