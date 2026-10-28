install_data('artwork' / 'rhythmcat-icon.svg',
  rename: 'org.supercat.RhythmCat3.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)

if gtk_dep.found()
  install_data('org.supercat.RhythmCat3.desktop',
    install_dir: get_option('datadir') / 'applications',
  )
endif
