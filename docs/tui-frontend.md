# ncurses frontend

The ncurses frontend is `rhythmcat-tui`. Press `?` in the program for the
list of keys.

## Keys

| Key           | Action                                        |
|---------------|-----------------------------------------------|
| Enter         | Play                                          |
| Space         | Pause                                         |
| `r`           | Repeat mode                                   |
| `n` / `p`     | Next / previous                               |
| ← / →         | Seek                                          |
| `+` / `-`     | Volume                                        |
| `g`           | Grouping                                      |
| `a`           | Add a file, folder or playlist file           |
| `c`           | Cancel import                                 |
| `L` / `W`     | Load / save a playlist file                   |
| `M`           | Remove missing files                          |
| `A`           | Add to playlist                               |
| `N` / `R` / `D` | New / rename / delete playlist              |
| `J` / `K`     | Reorder                                       |
| `e`           | Equalizer on/off                              |
| `E`           | Next equalizer preset                         |
| `m`           | MPRIS on/off                                  |
| `q`           | Quit                                          |

## Display

* The header shows the current equalizer preset.
* The playlist of the playing track is marked with `>`.
* Text too wide for the playlist and track lists is cut with an ellipsis
  (`…`).
* A lyric line too wide for the terminal moves horizontally with the
  progress of the line.

Log output goes to `~/.cache/RhythmCat3/tui.log`.
