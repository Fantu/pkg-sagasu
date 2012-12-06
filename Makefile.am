# $Id: Makefile.am,v 1.8 2004/07/31 03:51:42 sarrazip Exp $

SUBDIRS = intl src po macros

RPMBUILD = rpmbuild
rpm: $(PACKAGE).spec dist
	$(RPMBUILD) -ta $(distdir).tar.gz

helpdir = $(datadir)/gnome/help/$(PACKAGE)
enhelpdir = $(helpdir)/C
frhelpdir = $(helpdir)/fr

enhelp_DATA = help/C/sagasu.html
frhelp_DATA = help/fr/sagasu.html

man_MANS = help/sagasu.1

pkgdatadir = $(datadir)/$(PACKAGE)
pkgdata_DATA = COPYING

docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA = \
	AUTHORS \
	COPYING \
	INSTALL \
	NEWS \
	README \
	THANKS \
	TODO

EXTRA_DIST = \
	config.rpath \
	bootstrap \
	autogen.sh \
	mkinstalldirs \
	$(PACKAGE).spec \
	$(enhelp_DATA) \
	$(frhelp_DATA) \
	$(man_MANS)

DISTCHECK_CONFIGURE_FLAGS = --without-included-gettext

ACLOCAL_AMFLAGS = -I macros

MAINTAINERCLEANFILES = \
	Makefile.in \
	aclocal.m4 \
	configure \
	sic/config-h.in \
	sic/stamp-h.in \
	$(AUX_DIST) \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	missing
