# RhythmCat RPM SPEC File

Name: RhythmCat
Version: 1.0.0
Release: Beta1
Summary: A Music Player with Plugin Support
Source0: %{name}-%{version}-alpha3~110606.tar.gz
License: GPLv3
Group: Application/Multimedia
URL: http://code.google.com/p/rhythmcat

Requires: gtk2 gstreamer gstreamer-plugins-base gstreamer-plugins-good
BuildRequires: gtk2-devel gstreamer-devel gstreamer-plugins-base-devel desktop-file-utils

%description
RhythmCat Music Player is a music player which can be
running under Linux. It can be used as a normal music 
player, and it can also show lyrics in the player 
window, it can extend its functions by plugins...

%prep
%setup -q -n %{name}-%{version}-beta1~110606

%build
./configure --prefix=/usr
make %{_smp_mflags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
# install desktop file and install
desktop-file-install --add-category="AudioVideo" --delete-original \
    --dir=%{buildroot}%{_datadir}/applications \
    %{buildroot}/%{_datadir}/applications/%{name}.desktop
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%post
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
update-desktop-database &> /dev/null || :
update-mime-database %{_datadir}/mime &> /dev/null || :

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc COPYING AUTHORS
%{_bindir}/*
%{_datadir}/%{name}/*
%{_datadir}/locale/*
%{_datadir}/applications/%{name}.desktop

%changelog
* Sat Jun 05 2011 SuperCat <supercatexpert@gmail.com> - 1.0.0-Beta1~110606
- The 1.0.0-Beta1 Version Package.

