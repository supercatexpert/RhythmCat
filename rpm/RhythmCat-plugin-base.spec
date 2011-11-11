# RhythmCat base plugins RPM SPEC File

Name: RhythmCat-pluings-base
Version: 1.0.0
Release: 1
Summary: The base plugins for RhythmCat Music Player
Source0: RhythmCat-1.0.0.tar.gz
License: GPLv3
Group: Application/Multimedia
URL: http://code.google.com/p/rhythmcat

Requires: gtk3 gstreamer gstreamer-plugins-base gstreamer-plugins-good
BuildRequires: gtk3-devel gstreamer-devel gstreamer-plugins-base-devel

%description
The base pluins for RhythmCat Music Player.
This package contains lyric show plugin, desktop lyric 
show plugin, and notify plugin.

%prep
%setup -q -n RhythmCat-1.0.0

%build
pwd
cd plugins/base
cd lyric-show
make %{_smp_mflags} -f Makefile3
cd ..
cd desktop-lyric
make %{_smp_mflags} -f Makefile3
cd ..
cd notify
make %{_smp_mflags} -f Makefile3
cd ..
cd music-locator
make %{_smp_mflags} -f Makefile3
cd ..

%install
rm -rf %{buildroot}

cd plugins/base
cd lyric-show
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/lyric-show install
cd ..
cd desktop-lyric
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/desktop-lyric install
cd ..
cd notify
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/notify install
cd ..
cd music-locator
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/music-locator install
cd ..

%post


%postun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_datadir}/RhythmCat/plugins/*
%{_datadir}/RhythmCat/plugins/lyric-show/*
%{_datadir}/RhythmCat/plugins/desktop-lyric/*
%{_datadir}/RhythmCat/plugins/notify/*

%changelog
* Sat Nov 19 2011 SuperCat <supercatexpert@gmail.com> - 1.0.0-1
- The 1.0.0-1 Stable Version Package.

