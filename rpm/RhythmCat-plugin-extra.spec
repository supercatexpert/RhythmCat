# RhythmCat extra plugins RPM SPEC File

Name: RhythmCat-plugins-extra
Version: 1.0.0
Release: 1
Summary: The base plugins for RhythmCat Music Player
Source0: RhythmCat-1.0.0.tar.gz
License: GPLv3
Group: Application/Multimedia
URL: http://code.google.com/p/rhythmcat

Requires: gtk3 gstreamer gstreamer-plugins-base gstreamer-plugins-good gtksourceview3 libcurl
BuildRequires: gtk3-devel gstreamer-devel gstreamer-plugins-base-devel gtksourceview3-devel libcurl-devel

%description
The extra pluins for RhythmCat Music Player.
This package contains lyric editor plugin, tag editor plugin,
and lyric crawler plugin.

%prep
%setup -q -n RhythmCat-1.0.0

%build
pwd
cd plugins/extra
cd lyric-editor
make %{_smp_mflags} -f Makefile3
cd ..
cd tageditor
make %{_smp_mflags} -f Makefile3
cd ..
cd lyric-crawler
make %{_smp_mflags} -f Makefile3
cd ..

%install
rm -rf %{buildroot}

cd plugins/extra
cd lyric-editor
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/lyric-editor install
cd ..
cd tageditor
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/tageditor install
cd ..
cd lyric-crawler
make -f Makefile3 INSTALL_DIR=%{buildroot}/usr/share/RhythmCat/plugins/lyric-crawler install
cd ..

%post


%postun

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_datadir}/RhythmCat/plugins/*
%{_datadir}/RhythmCat/plugins/lyric-editor/*
%{_datadir}/RhythmCat/plugins/tageditor/*
%{_datadir}/RhythmCat/plugins/lyric-crawler/*

%changelog
* Sat Nov 19 2011 SuperCat <supercatexpert@gmail.com> - 1.0.0-1
- The 1.0.0-1 Version Package.

