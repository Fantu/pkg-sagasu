# $Id: sagasu.spec.in,v 1.14 2012/11/25 00:58:19 sarrazip Exp $

# RPM .spec file

# Release number can be specified with rpm --define 'rel SOMETHING' ...
# If no such --define is used, the release number is 1.
#
# Source archive's extension can be specified with rpm --define 'srcext .foo'
# where .foo is the source archive's actual extension.
# To compile an RPM from a .bz2 source archive, give the command
#   rpmbuild -ta --define 'srcext .bz2' sagasu-2.0.12.tar.bz2
#
%if %{?rel:0}%{!?rel:1}
%define rel 1
%endif
%if %{?srcext:0}%{!?srcext:1}
%define srcext .gz
%endif

Summary: GNOME tool to find strings in a set of files
Summary(fr): Outil GNOME cherchant des chaines dans des fichiers
Name: sagasu
Version: 2.0.12
Release: %{rel}
License: GPL
Group: Applications/Text
Source0: %{name}-%{version}.tar%{srcext}
URL: http://sarrazip.com/dev/sagasu.html
Prefix: /usr
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: libgnomeui-devel >= 2.8.0
BuildRequires: gettext
BuildRequires: desktop-file-utils
Requires: libgnomeui >= 2.8.0

%description
GNOME tool to find words in a set of files.
The user specifies the search directory and the set of files
to be searched.  Double-clicking on a search result launches a
user command that can for example load the file in an editor
at the appropriate line.  The search can optionally ignore
CVS directories.

%description -l fr
Outil GNOME cherchant des chaines dans un ensemble de fichiers.
L'usager spécifie le répertoire de recherche et l'ensemble des
fichiers à fouiller.  Un double-clic sur un résultat de recherche
démarre une commande spécifiée par l'usager qui peut par exemple
ouvrir un éditeur sur le fichier concerné, à la bonne ligne.
La recherche peut optionnellement ignorer les répertoires CVS.


%prep
%setup -q

%build
%configure --disable-dependency-tracking --disable-maintainer-mode
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
%find_lang %{name}
desktop-file-validate $RPM_BUILD_ROOT/%{_datadir}/applications/%{name}.desktop

%clean
rm -fR $RPM_BUILD_ROOT

%files -f %{name}.lang
%defattr(-, root, root)
%{_bindir}/*
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/sagasu
%{_datadir}/gnome/help/*/*/*
%{_mandir}/man*/*
%doc %{_defaultdocdir}/*
