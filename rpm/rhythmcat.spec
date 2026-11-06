%global app_id org.supercat.RhythmCat3

Name:           rhythmcat
Version:        3.0.0
Release:        1%{?dist}
Summary:        Music player based on GLib, GStreamer and json-c

License:        GPL-3.0-or-later
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.64.0
BuildRequires:  gettext
BuildRequires:  desktop-file-utils
BuildRequires:  pkgconfig(glib-2.0) >= 2.74
BuildRequires:  pkgconfig(gio-2.0) >= 2.74
BuildRequires:  pkgconfig(gstreamer-1.0) >= 1.16
BuildRequires:  pkgconfig(gstreamer-pbutils-1.0) >= 1.16
BuildRequires:  pkgconfig(gstreamer-tag-1.0) >= 1.16
BuildRequires:  pkgconfig(gstreamer-audio-1.0) >= 1.16
BuildRequires:  pkgconfig(json-c) >= 0.15
BuildRequires:  pkgconfig(gtk4) >= 4.12
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(ncursesw)
# For the unit tests
BuildRequires:  gstreamer1-plugins-base
BuildRequires:  gstreamer1-plugins-good
BuildRequires:  /usr/bin/dbus-daemon

Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       hicolor-icon-theme
Suggests:       %{name}-tui

%description
RhythmCat is a music player based on GLib, GStreamer and json-c.

Features:
 * music library grouped by artist or album, and user playlists
 * CUE sheets (external and embedded) and legacy tag encodings
 * synchronized lyrics (LRC, SRT, WebVTT, ASS)
 * M3U, M3U8, PLS and XSPF playlist files
 * 10-band equalizer with presets
 * media controls of the desktop (MPRIS)

This package contains the GTK 4 frontend.

%package libs
Summary:        RhythmCat music player - core library
Requires:       gstreamer1-plugins-base%{?_isa}
Recommends:     gstreamer1-plugins-good%{?_isa}
Suggests:       gstreamer1-plugins-bad-free%{?_isa}
Suggests:       gstreamer1-plugins-ugly-free%{?_isa}
Suggests:       gstreamer1-plugin-libav%{?_isa}

%description libs
RhythmCat is a music player based on GLib, GStreamer and json-c, with a
GTK 4 frontend and an ncurses frontend.

This package contains the core library, which plays audio and manages
the music library, playlists, metadata, CUE sheets, lyrics, playlist
files and the equalizer, and the translations: Chinese (simplified),
Chinese (traditional) and Japanese.

%package devel
Summary:        RhythmCat music player - development files
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

%description devel
RhythmCat is a music player based on GLib, GStreamer and json-c, with a
GTK 4 frontend and an ncurses frontend.

This package contains the header files and the pkg-config file of the
core library.

%package tui
Summary:        RhythmCat music player - terminal frontend
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Suggests:       %{name}

%description tui
RhythmCat is a music player based on GLib, GStreamer and json-c.

This package contains the ncurses frontend, which plays music and
manages the music library and playlists in a terminal.

%prep
%autosetup

%build
%meson -Dgtk=enabled -Dncurses=enabled
%meson_build

%install
%meson_install
# The manual pages are kept with the Debian packaging files
install -Dpm 0644 debian/rhythmcat.1 %{buildroot}%{_mandir}/man1/rhythmcat.1
install -Dpm 0644 debian/rhythmcat-tui.1 \
    %{buildroot}%{_mandir}/man1/rhythmcat-tui.1
%find_lang rhythmcat3

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{app_id}.desktop
%meson_test

%files
%license COPYING
%doc README.md
%{_bindir}/rhythmcat
%{_datadir}/applications/%{app_id}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{app_id}.svg
%{_mandir}/man1/rhythmcat.1*

%files libs -f rhythmcat3.lang
%license COPYING
%{_libdir}/librhythmcat3.so.3{,.*}

%files devel
%{_includedir}/rhythmcat3/
%{_libdir}/librhythmcat3.so
%{_libdir}/pkgconfig/rhythmcat3.pc

%files tui
%{_bindir}/rhythmcat-tui
%{_mandir}/man1/rhythmcat-tui.1*

%changelog
* Thu Oct 29 2026 Kyosuke Nekoyashiki <supercatexpert@gmail.com> - 3.0.0-1
- Initial release.
