# RhythmCat 3

RhythmCat is a music player written in C, based on GLib, GStreamer 1.0 and
json-c. It has two parts:

* **LibRhythmCat** (`lib/`, `librhythmcat3.so`): the core library. It plays
  audio and manages the music library, playlists, metadata, CUE sheets and
  lyrics.
* **Frontends**, which link the core library dynamically:
  * `rhythmcat3`: the GTK4 frontend (`ui-gtk/`)
  * `rhythmcat3-tui`: the ncurses frontend (`ui-tui/`)

Only one frontend can run at a time. Starting a second one prints an error
(the GTK frontend also shows a dialog) that names the running instance.

License: GNU GPL v3 or later (see `COPYING`).

## Building

Dependencies: GLib/GIO ≥ 2.74, GStreamer ≥ 1.16 (core, base: pbutils,
tag, audio), json-c ≥ 0.15, GTK ≥ 4.12 (optional), ncursesw (optional),
meson ≥ 0.64.

```sh
meson setup build
ninja -C build
meson test -C build          # unit tests for the core library
sudo ninja -C build install
```

Build options (`meson configure build -D<option>=<value>`):

| Option    | Default | Description                                  |
|-----------|---------|----------------------------------------------|
| `gtk`     | auto    | Build the GTK4 frontend                      |
| `ncurses` | auto    | Build the ncurses frontend                   |

## Artwork

The application icon (`data/artwork/rhythmcat-icon.svg`) and the default
album cover for music without an embedded image
(`data/artwork/rhythmcat-cover.svg`) are SVG images of a cat as the head of
an eighth note. Both are compiled into the GTK executable as GResources, and
the icon is also installed as the application icon.

## Features

### Core library

* Audio playback with GStreamer `playbin`: play, pause, stop, seek, volume,
  and play ranges (used for CUE tracks). Signals for state, metadata,
  duration, volume, end of stream and errors.
* 10-band equalizer (29 Hz – 15 kHz, ±12 dB, using `equalizer-10bands`
  from gst-plugins-good). It can be turned on or off, and has presets
  (Flat, Pop, Rock, Jazz, Classical, Dance, Electronic, Hip-Hop, Vocal,
  Bass Boost, Treble Boost) and a user customized preset. The state is
  saved in the settings and applied automatically.
* Music library and user playlists (`RCLibDb`):
  * Group the library by artist or album.
  * Reorder playlists and the items in them.
  * Artist, album and genre are interned strings (`g_intern_string`), so
    each name is stored once and grouping compares pointers.
  * Saved as gzip-compressed UTF-8 JSON (json-c and `GZlibCompressor`) in
    `~/.local/share/RhythmCat3/library.json.gz`. The file is replaced
    atomically and autosaved after changes.
* Metadata:
  * Read asynchronously with `GstDiscoverer` when files or folders are
    imported.
  * Updated from stream tags while a track plays (configurable), or
    refreshed on demand for selected items or the whole database.
  * Legacy ID3 tags (ID3v1, or ID3v2 marked as ISO-8859-1) are decoded
    with the configurable fallback encoding list before Latin-1 is tried.
* CUE sheets: external `.cue` files and embedded `CUESHEET` tags. Missing
  audio files are looked up by base name with common extensions. Non-UTF-8
  sheets are decoded with the fallback encoding list.
* Lyrics: LRC (including enhanced word timestamps and `[offset:]`), SubRip
  (`.srt`), WebVTT (`.vtt`) and ASS/SSA. A lyric file must be in the same
  directory as the audio file and have the same base name. Non-Unicode
  files use the fallback encoding list.
* Playing order with four repeat modes:
  * **Off**: after the last item of a playlist, continue with the first
    item of the next playlist; stop after the last playlist.
  * **List**: repeat the current playlist (or the music library).
  * **All**: like Off, but continue with the first playlist after the
    last one.
  * **Track**: repeat the current item; skipping to the next item works
    like Off.

  Empty playlists are skipped. In random mode, the next item is a random
  item of the current playlist.
* Settings are stored in `~/.config/RhythmCat3/settings.conf`.

### GTK4 frontend

* The UI is built with GtkBuilder. UI files, CSS and images are compiled
  into the executable as GResources.
* Light/dark style switch (header bar toggle or Settings).
* Shows the cover (embedded image or the default cover), title (the file
  base name if there's no title tag), artist, album, position/duration with
  a seek bar, and synced lyrics (previous/current/next line).
* Library sidebar with grouping, and a playlist sidebar with new, rename,
  delete and move up/down.
* A button next to the random button shows or hides the library and
  playlist area. When it is hidden, the window shrinks to a compact
  player; the choice is remembered.
* Track list: import files or folders (also by drag and drop), add to
  playlist, remove, reorder, refresh metadata.
* Reorder playlist items by long press dragging: press and hold an item for
  half a second until it is selected, then drag it to the new position (a
  line shows where it will be placed). If the item is one of several
  selected items, all selected items are moved together and keep their
  order. Dropping below the last item moves them to the end. The up/down
  buttons move a single item.
* Settings: style, lyric display, update metadata when playing, and the
  fallback encodings for ID3, CUE and lyrics.
* Equalizer window (header bar button, menu, or Ctrl+E): an on/off switch,
  a preset list and a slider for each band. Moving a slider changes the
  custom preset, starting from the gains of the current preset. "Reset
  Custom" sets all custom bands to 0 dB.
* Shortcuts: Ctrl+Space play/pause, Ctrl+←/→ previous/next, Ctrl+O import,
  Ctrl+, settings, Ctrl+E equalizer, Ctrl+Q quit.

### ncurses frontend

Press `?` in the program for the key list. Main keys: Enter play, Space
pause, `r` repeat mode, `n`/`p` next/previous, ←/→ seek, `+`/`-` volume,
`g` grouping, `a` add a file/folder, `A` add to playlist, `N`/`R`/`D` new/rename/delete
playlist, `J`/`K` reorder, `e` equalizer on/off, `E` next equalizer preset,
`q` quit. The header shows the current equalizer preset. Log output goes to
`~/.cache/RhythmCat3/tui.log`.
