# RhythmCat 3

RhythmCat is a music player written in C, based on GLib, GStreamer 1.0 and
json-c. The core library, LibRhythmCat (`lib/`, `librhythmcat3.so`), plays
audio and manages the music library and playlists. Two frontends use it:

* **`rhythmcat`**, the GTK4 frontend (`ui-gtk/`): a desktop window with the
  album cover, synced lyrics, an OSD lyric window, library and playlist
  sidebars, drag and drop, and an equalizer window.
* **`rhythmcat-tui`**, the ncurses frontend (`ui-tui/`): the same player in
  a terminal, controlled with the keyboard (press `?` for the key list).

Only one frontend can run at a time; starting a second one shows an error
that names the running instance.

License: GNU GPL v3 or later (see `COPYING`).

## Building

Dependencies: GLib/GIO ≥ 2.74, GStreamer ≥ 1.16, json-c ≥ 0.15,
meson ≥ 0.64, and GTK ≥ 4.12 and/or ncursesw for the frontends.

```sh
meson setup build
ninja -C build
meson test -C build
sudo ninja -C build install
```

See [docs/building.md](docs/building.md) for the build options and the
Debian, Fedora (RPM) and Arch Linux packages.

## Features

* Plays local files and remote streams, with seeking, volume and a 10-band
  equalizer with presets.
* Music library, grouped by artist or album, and any number of playlists.
* Imports files and folders, reading the metadata in the background.
* CUE sheets, both external `.cue` files and embedded ones.
* Playlist files: M3U, M3U8, PLS and XSPF, to load and to save.
* Synced lyrics: LRC, SubRip, WebVTT and ASS/SSA.
* Repeat modes (off, list, all, track) and random play.
* Media controls of the desktop and `playerctl` (MPRIS 2).
* Fallback encodings for old ID3 tags, CUE sheets, lyrics and playlists.
* Translations: Simplified Chinese, Traditional Chinese and Japanese.

## Documentation

* [Building and packaging](docs/building.md)
* [Core library](docs/core-library.md)
* [GTK4 frontend](docs/gtk-frontend.md)
* [ncurses frontend](docs/tui-frontend.md)
* [Translations](docs/translations.md)
* Manual pages: `rhythmcat(1)` and `rhythmcat-tui(1)`
