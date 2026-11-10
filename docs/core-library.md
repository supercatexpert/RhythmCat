# Core library (LibRhythmCat)

* Audio playback with GStreamer `playbin`: play, pause, stop, seek, volume,
  and play ranges (used for CUE tracks). Signals for state, metadata,
  duration, volume, seeking, end of stream and errors.
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
  item of the current playlist. A frontend can set a filter and a playing
  order for the music library (`rclib_player_set_library_filter()` and
  `rclib_player_set_library_order()`); then the next or previous library
  item is chosen in that order, from the matching items only. The order
  may have only a part of the library (like the shown group), then a
  library item is played in that part only, and List and All repeat it.
* MPRIS 2 D-Bus interface (`org.mpris.MediaPlayer2.RhythmCat3`, on by
  default, can be turned off in the settings): the media controls of the
  desktop (like GNOME Shell and KDE Plasma) and tools like `playerctl`
  show the playing track (title, artist, album, length and the embedded
  cover image) and control the player: play/pause, stop, previous/next,
  seek, volume, repeat (Loop) and random (Shuffle) modes. Both frontends
  can be quit through it, and the GTK4 window can be raised. The track
  list interface and opening URIs are not supported. The cover image is
  written to a temporary file in the user runtime directory, which is
  removed when the track changes or the program exits.
* Settings are stored in `~/.config/RhythmCat3/settings.conf`.
