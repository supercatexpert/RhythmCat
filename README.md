# RhythmCat 3

RhythmCat is a music player written in C, based on GLib, GStreamer 1.0 and
json-c. It has two parts:

* **LibRhythmCat** (`lib/`, `librhythmcat3.so`): the core library. It plays
  audio and manages the music library, playlists, metadata, CUE sheets and
  lyrics.
* **Frontends**, which link the core library dynamically:
  * `rhythmcat`: the GTK4 frontend (`ui-gtk/`)
  * `rhythmcat-tui`: the ncurses frontend (`ui-tui/`)

Only one frontend can run at a time. Starting a second one prints an error
(the GTK frontend also shows a dialog) that names the running instance.

License: GNU GPL v3 or later (see `COPYING`).

## Building

Dependencies: GLib/GIO ≥ 2.74, GStreamer ≥ 1.16 (core, base: pbutils,
tag, audio), json-c ≥ 0.15, GTK ≥ 4.12 (optional), ncursesw (optional),
meson ≥ 0.64. Xlib (`x11`, optional) is used by the GTK frontend to
keep the OSD lyric window above other windows (on X11 and XWayland).

```sh
meson setup build
ninja -C build
meson test -C build          # unit tests for the core library
sudo ninja -C build install
```

Debian packages can be built from the `debian/` directory (needs
`dpkg-dev`, `debhelper` and the build dependencies listed in
`debian/control`):

```sh
dpkg-buildpackage -us -uc -b
```

The packages are written to the parent directory: `librhythmcat3-3` (the
core library), `librhythmcat3-dev` (headers and pkg-config file),
`rhythmcat` (the GTK4 frontend), `rhythmcat-tui` (the ncurses frontend)
and `rhythmcat-l10n` (the translations).

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
the icon is also installed as the application icon
(`share/icons/hicolor/scalable/apps/org.supercat.RhythmCat3.svg`), together
with the desktop file `org.supercat.RhythmCat3.desktop`.

The GTK frontend uses the icon for its windows and the About dialog. On X11
this works without installing. On Wayland, the desktop (for example GNOME
Shell) takes the icon from the desktop file whose name matches the
application ID, so the program must be installed to show its icon in the
dock and the window switcher. To install for the current user only:

```sh
meson setup build --prefix="$HOME/.local"
ninja -C build install
```

## Translations

The core library and both frontends are translated (gettext domain
`rhythmcat3`, in `po/`) into:

* Chinese (simplified), `zh_CN`
* Chinese (traditional), `zh_TW`
* Japanese, `ja`

The language follows the locale (`LANG`, `LC_ALL` or `LANGUAGE`), for
example `LANG=ja_JP.UTF-8 rhythmcat`. The translations are installed to
`share/locale`, so the program must be installed to use them. The name,
generic name, comment and keywords in the desktop file are translated too.

To update the translations after changing the strings:

```sh
ninja -C build rhythmcat3-pot        # regenerate po/rhythmcat3.pot
ninja -C build rhythmcat3-update-po  # merge it into the .po files
```

To add a language, add its code to `po/LINGUAS` and create the `.po` file
from the template (for example with `msginit`).

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
  * Items whose audio files are missing can be removed from the library
    and all playlists (checked in a background thread).
  * Read asynchronously with `GstDiscoverer` when files or folders are
    imported. Pending imports can be cancelled; the items already imported
    are kept.
  * Updated from stream tags while a track plays (configurable), or
    refreshed on demand for selected items or the whole database.
  * Legacy ID3 tags (ID3v1, or ID3v2 marked as ISO-8859-1) are decoded
    with the configurable fallback encoding list before Latin-1 is tried.
* CUE sheets: external `.cue` files and embedded `CUESHEET` tags. Missing
  audio files are looked up by base name with common extensions; tracks
  whose audio files still cannot be found are not imported. Non-UTF-8
  sheets are decoded with the fallback encoding list.
* Playlist files: M3U (including extended M3U), M3U8, PLS and XSPF.
  * Load a file as a new playlist (named by the title in the file, or the
    file name), or import a playlist file into the library or a playlist
    like an audio file. The order of the entries is kept. Remote streams
    (such as web radio URLs) are added with the title in the file.
  * Save a playlist (or any list of tracks) to a file; the format is chosen
    by the extension name. Files in the folder of the playlist file are
    written with relative paths, others with absolute paths. Files are
    written in UTF-8.
  * M3U and PLS files which are not UTF-8 are decoded with the fallback
    encoding list. Windows-style relative paths (`dir\song.mp3`) are
    supported.
  * CUE tracks are written as the CUE sheet with the track number in an
    extension (`#RHYTHMCAT-CUE-TRACK:` in M3U, a `meta` element in XSPF),
    so they can be loaded again exactly; other players play the whole CUE
    sheet. PLS has no extension, so a CUE sheet is written once.
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
  a seek bar, and synced lyrics (previous/current/next line, at the bottom
  of the window above the status bar). The lyric display can be turned on
  or off in the menu ("Lyric Display") or Settings, and it is always shown
  when it is on; if the playing music has no lyric file, it shows
  "title - artist - album" of the music. The current line is shown in one
  line; if it is too wide, it moves horizontally with the progress of the
  line, and the music information moves from the start to the end and
  back, resting for 3 seconds at each end. The previous and next lines are
  also shown in one line, ending with an ellipsis if they are too wide.
  The status bar shows the format
  (codec) and bitrate of the playing music; the bitrate is updated while
  playing VBR streams, and for streams without bitrate information it is
  the average from the file size and the duration. Playback errors (such
  as a missing file) are shown in the status bar until music is played
  successfully.
* OSD lyric (off by default; menu "OSD Lyric" or Settings): the current
  lyric line in a transparent window on the desktop, drawn with an outline
  so it can be read on any background. When it is on, the window is
  always shown; if the playing music has no lyric file, it shows
  "title - artist" of the music. The played part of the line is
  filled with the progress color, moving from the start time of the line
  to its end time (the end time in the lyric file, the start of the next
  line, or the end of the track). The font,
  font size, text color, progress color (the progress can be turned off)
  and width (percent of the screen width) can be set in Settings. The
  window always has this width and the text is shown in one line in the
  middle of it; a line which is too wide moves horizontally with its
  progress, and a title which is too wide moves from the start to the end
  and back, resting for 3 seconds at each end. By default, the bottom of the window is at 90%
  of the screen height. Drag the text to move it, right click it to hide
  it or reset its position. The window stays above other windows on all
  workspaces, and the position is remembered. Wayland has no way for a
  program to do this, so on Wayland the window is shown with XWayland (the
  X11 display in `DISPLAY`, or in the environment of the systemd user
  manager if `DISPLAY` is not set, for example in a terminal multiplexer
  started before XWayland); if XWayland is not available, the compositor
  decides where the window is placed and whether it stays on top. It works
  even if the lyric in the main window is turned off.
* Library sidebar with grouping, and a playlist sidebar with new, rename,
  delete and move up/down.
  The playlist of the playing track is shown in bold (in the ncurses
  frontend, also marked with `>`).
* A button next to the random button shows or hides the library and
  playlist area. When it is hidden, the window shrinks to a compact
  player; the choice is remembered.
* Track list: import files or folders (also by drag and drop), add to
  playlist, remove, reorder, refresh metadata. The list is updated in place
  while importing, so the selection is kept and tracks can be played.
  While files are being imported, a "Cancel Import" button is shown in
  the status bar (also in the menu).
* Reorder playlist items by long press dragging: press and hold an item for
  half a second until it is selected, then drag it to the new position (a
  line shows where it will be placed). If the item is one of several
  selected items, all selected items are moved together and keep their
  order. Dropping below the last item moves them to the end. The up/down
  buttons move a single item.
* "Remove Missing Files…" in the menu removes the items in the library and
  all playlists whose audio files do not exist any more (after a
  confirmation). The files are checked in a background thread; remote
  streams are kept.
* Load a playlist file as a new playlist, or save the shown track list
  to a playlist file (buttons below the playlist sidebar, or the menu).
* Settings: style, lyric display, OSD lyric, update metadata when
  playing, and the fallback encodings for ID3, CUE, lyrics and playlist
  files.
* Equalizer window (header bar button, menu, or Ctrl+E): an on/off switch,
  a preset list and a slider for each band. Moving a slider changes the
  custom preset, starting from the gains of the current preset. "Reset
  Custom" sets all custom bands to 0 dB.
* Shortcuts: Space or Ctrl+Space play/pause (Space types a space in text
  fields), Ctrl+←/→ previous/next, Ctrl+O import, Ctrl+, settings, Ctrl+E
  equalizer, Ctrl+Q quit.

### ncurses frontend

Press `?` in the program for the key list. Main keys: Enter play, Space
pause, `r` repeat mode, `n`/`p` next/previous, ←/→ seek, `+`/`-` volume,
`g` grouping, `a` add a file/folder/playlist file, `c` cancel import,
`L`/`W` load/save a playlist file, `M` remove missing files, `A` add to
playlist, `N`/`R`/`D` new/rename/delete playlist, `J`/`K` reorder, `e`
equalizer on/off, `E` next equalizer preset, `q` quit. The header shows
the current equalizer preset. Text too wide for the playlist and track
lists is cut with an ellipsis (`…`). A lyric line too wide for the
terminal moves horizontally with the progress of the line. Log output goes
to
`~/.cache/RhythmCat3/tui.log`.
