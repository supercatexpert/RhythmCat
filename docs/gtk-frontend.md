# GTK4 frontend

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
  font size (in points) follows the font scaling of the desktop (for
  example the text scaling factor of GNOME), and the window is resized
  when the scaling or the monitors change. The
  window always has this width and the text is shown in one line in the
  middle of it; a line which is too wide moves horizontally with its
  progress, and a title which is too wide moves from the start to the end
  and back, resting for 3 seconds at each end. By default, the bottom of the window is at 90%
  of the screen height. Drag the text to move it, right click it to hide
  it or reset its position (on X11). On X11, the window stays above other
  windows on all workspaces, and the position is remembered. Wayland has no way
  for a program to do this, so on Wayland the compositor decides where
  the window is placed and whether it stays on top (for example, use
  "Always on Top" in the window menu of GNOME, opened with Alt+Space). It works
  even if the lyric in the main window is turned off.
* Library sidebar with grouping, and a playlist sidebar with new, rename,
  delete and move up/down.
* The library can be sorted by title, artist, album, track number or
  length: click a column header to sort in ascending order, again for
  descending order, and a third time to show the library in its own
  order. Tracks without the value are placed at the end, tracks with the
  same value keep their library order. The choice is remembered.
  Playlists are not sorted (their headers can not be clicked). Tracks in
  the library are played in the sorted order (previous/next, and when a
  track ends).
* A track played in the library is played in the list shown when it was
  started: the selected artist or album group (or the whole library),
  with the filter and the sorting. Previous/next, random mode and the end
  of a track stay in that list, and the repeat modes List and All repeat
  it, even when another group is shown later.
* Library filter (the entry in the library sidebar, or Ctrl+F): only the
  tracks whose title (or file name), artist or album contains every word
  of the filter are shown. Case is ignored, and so are the differences of
  full width and half width forms. The filter is applied to the library
  only, not to playlists; Escape clears it. While the filter is set, the
  next and previous tracks in the library (also in random mode and when
  a track ends) are chosen from the filtered tracks only.
* The playlist of the playing track is shown in bold and marked with
  the playing icon.
* A button next to the random button shows or hides the library and
  playlist area. When it is hidden, the window shrinks to a compact
  player; the choice is remembered.
* Track list: import files or folders (also by drag and drop), add to
  playlist, remove, reorder, refresh metadata. The list is updated in place
  while importing, so the selection is kept and tracks can be played.
  While files are being imported, a "Cancel Import" button is shown in
  the status bar (also in the menu).
* Drag and drop tracks (a drag starts as soon as the pointer moves, like
  other drags in GTK; the dragged item is selected). If the item is one
  of several selected items, all selected items are dragged together and
  keep their order.
  * Drop them in the same playlist to reorder them: a line shows where
    they will be placed, and dropping below the last item moves them to
    the end. The up/down buttons move a single item.
  * Drop them on another playlist in the playlist sidebar to move them to
    the end of it (the playlist under the pointer is highlighted); they
    are removed from the playlist they are dragged from. Tracks dragged
    from the library are copied to the playlist and kept in the library.
* Drag a playlist in the playlist sidebar to reorder the playlists: a line
  shows where it will be placed, and dropping below the last playlist
  moves it to the end. The up/down buttons below the sidebar also move
  the selected playlist.
* "Show Playing Track" (the button above the track list, the menu, or
  Ctrl+J) shows the playing track: the playlist or the library group it
  is played in is selected and scrolled to in the sidebar (the grouping
  is changed if needed, and the library filter is cleared if it hides the
  track), and the track is selected and scrolled to in the track list.
* Right-click menus. On a track: play, track details, add to a playlist,
  move up/down (in a playlist), refresh metadata and remove from the
  playlist or the library; a track which is not selected is selected first,
  otherwise the menu is for all selected tracks. On a playlist in the
  sidebar: new, load, rename, save, delete and move up/down (the playlist is
  selected first); on the empty area below the playlists: new and load.
* "Track Details" (the button above the track list, the menu, or Ctrl+I)
  shows the details of the selected track (the first one if several are
  selected): the cover image embedded in the file on the left (the default
  cover if there is none), the metadata, the format, bitrate, sample rate
  and channels, the full path of the file, its size and modification time,
  the CUE sheet of a CUE track, and whether it is in the library or a
  playlist. Showing another track while the window is open replaces its
  contents.
* "Remove Missing Files…" in the menu removes the items in the library and
  all playlists whose audio files do not exist any more (after a
  confirmation). The files are checked in a background thread; remote
  streams are kept.
* Load a playlist file as a new playlist, or save the shown track list
  to a playlist file (buttons below the playlist sidebar, or the menu).
* Settings: style, lyric display, OSD lyric, media controls of the
  desktop (MPRIS), update metadata when playing, and the fallback encodings for ID3, CUE, lyrics and playlist
  files.
* Equalizer window (header bar button, menu, or Ctrl+E): an on/off switch,
  a preset list and a slider for each band. Moving a slider changes the
  custom preset, starting from the gains of the current preset. "Reset
  Custom" sets all custom bands to 0 dB.
* Shortcuts: Space or Ctrl+Space play/pause (Space types a space in text
  fields), Ctrl+←/→ previous/next, Ctrl+O import, Ctrl+F filter the library,
  Ctrl+J show the playing track, Ctrl+I track details, Ctrl+, settings,
  Ctrl+E equalizer, Ctrl+Q quit.

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
dock and the window switcher (see [Building](building.md#installing-for-the-current-user)).
