# $Id: Makefile.am,v 1.8 2007/09/23 16:21:06 sarrazip Exp $

localedir = $(datadir)/locale
pixmapdir = $(datadir)/pixmaps
pkgdatadir = $(datadir)/$(PACKAGE)
pkgsounddir = $(datadir)/sounds/$(PACKAGE)
desktopdir = $(datadir)/applications

bin_PROGRAMS = sagasu

sagasu_SOURCES = \
	main.cpp \
	callbacks.h \
	SagasuApp.h \
	SagasuApp.cpp \
	ResultPage.cpp \
	ResultPage.h \
	util.cpp \
	util.h

sagasu_CXXFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DGNOMEICONDIR=\"$(pixmapdir)\" \
	-I$(srcdir)/intl \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DPKGSOUNDDIR=\"$(pkgsounddir)\" \
	-DPACKAGE_HOME_PAGE=\"$(PACKAGE_HOME_PAGE)\" \
	$(LIBGNOMEUI_CFLAGS)

sagasu_LDADD = $(LIBGNOMEUI_LIBS) $(INTLLIBS)


# Helper Perl script called by the C++ GNOME application:
pkgdata_DATA = sagasu-helper.pl


pixmap_DATA = \
	images/$(PACKAGE).png \
	images/close.xpm

pkgsound_DATA = sounds/sagasu.wav

desktop_DATA = $(PACKAGE).desktop

run: all
	PKGDATADIR=$(srcdir) \
		PKGSOUNDDIR=$(srcdir)/sounds \
		PIXMAPDIR=$(srcdir)/images \
		./$(PACKAGE)

check:
	$(PERL) -cw $(srcdir)/$(PACKAGE)-helper.pl
	test `$(PERL) $(srcdir)/$(PACKAGE)-helper.pl ^check: $(srcdir) Makefile.am 0 0 1 1 | wc -l` = 1
	test "`env DISPLAY= ./$(PACKAGE) --version | sed 's/gnome/GNOME/i'`" = "GNOME $(PACKAGE) $(VERSION)"
	./$(PACKAGE) --help >& /dev/null


EXTRA_DIST = $(pkgdata_DATA) $(pixmap_DATA) $(pkgsound_DATA)


MAINTAINERCLEANFILES = Makefile.in
