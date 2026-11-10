# Building RhythmCat 3

## Dependencies

* GLib/GIO ≥ 2.74
* GStreamer ≥ 1.16 (core, base: pbutils, tag, audio)
* json-c ≥ 0.15
* meson ≥ 0.64
* GTK ≥ 4.12 (optional, for the GTK4 frontend)
* ncursesw (optional, for the ncurses frontend)
* Xlib (`x11`, optional): used by the GTK frontend to keep the OSD lyric
  window above other windows (on X11)

## Building from source

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

The manual pages `rhythmcat(1)` and `rhythmcat-tui(1)` (`data/rhythmcat.1`
and `data/rhythmcat-tui.1`) are installed together with the frontends.

## Installing for the current user

The program can run from the build directory, but on Wayland the desktop
(for example GNOME Shell) takes the window icon from the desktop file whose
name matches the application ID, and the translations are loaded from
`share/locale`. So the program must be installed to show its icon in the
dock and the window switcher, and to use the translations. To install for
the current user only:

```sh
meson setup build --prefix="$HOME/.local"
ninja -C build install
```

## Debian packages

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

## RPM packages (Fedora)

RPM packages for Fedora can be built from `rpm/rhythmcat.spec` (needs
`rpm-build` and the build dependencies listed in the spec file, which
`sudo dnf builddep rpm/rhythmcat.spec` installs):

```sh
mkdir -p ~/rpmbuild/SOURCES
git archive --prefix=rhythmcat-3.0.0/ -o ~/rpmbuild/SOURCES/rhythmcat-3.0.0.tar.gz HEAD
rpmbuild -ba rpm/rhythmcat.spec
```

The packages are written to `~/rpmbuild/RPMS`: `rhythmcat-libs` (the core
library and the translations), `rhythmcat-devel` (headers and pkg-config
file), `rhythmcat` (the GTK4 frontend) and `rhythmcat-tui` (the ncurses
frontend).

## Arch Linux packages

Arch Linux packages can be built from `arch/PKGBUILD` (needs `base-devel`;
`-s` installs the build dependencies):

```sh
git archive --prefix=rhythmcat-3.0.0/ -o arch/rhythmcat-3.0.0.tar.gz HEAD
cd arch && makepkg -s
```

The packages are written to `arch/`: `librhythmcat3` (the core library,
headers, pkg-config file and translations), `rhythmcat` (the GTK4
frontend) and `rhythmcat-tui` (the ncurses frontend).
