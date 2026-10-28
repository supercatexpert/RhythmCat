install_data('artwork' / 'rhythmcat-icon.svg',
  rename: 'org.supercat.RhythmCat3.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

if gtk_dep.found()
  install_data('org.supercat.RhythmCat3.desktop',
    install_dir: get_option('datadir') / 'applications',
  )
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  update_desktop_database: gtk_dep.found(),
)
