SUBDIRS = pics

bin_PROGRAMS = kimagemapeditor
kde_module_LTLIBRARIES = libkimagemapeditor.la

METASOURCES = AUTO

libkimagemapeditor_la_SOURCES = \
    qextfileinfo.cpp \
    areacreator.cpp \
    kimearea.cpp \
    kimecommands.cpp \
    kimedialogs.cpp \
    kimagemapeditor.cpp \
    drawzone.cpp \
    arealistview.cpp \
    arealistview.h \
    imageslistview.cpp \
    imageslistview.h \
    mapslistview.cpp \
    mapslistview.h

kimagemapeditor_SOURCES = \
    main.cpp \
    kimeshell.cpp \
    qextfileinfo.cpp \
    areacreator.cpp \
    kimearea.cpp \
    kimecommands.cpp \
    kimedialogs.cpp \
    kimagemapeditor.cpp \
    drawzone.cpp \
    arealistview.cpp \
    arealistview.h \
    imageslistview.cpp \
    imageslistview.h \
    mapslistview.cpp \
    mapslistview.h

noinst_HEADERS = \
	qextfileinfo.h \
	areacreator.h \
	kimearea.h \
	kimecommands.h \
	kimedialogs.h \
	kimagemapeditor.h \
	drawzone.h \
  kimeshell.h


libkimagemapeditor_la_LDFLAGS = -module -no-undefined $(all_libraries) -version-info 1:0:0
libkimagemapeditor_la_LIBADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)

kimagemapeditor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimagemapeditor_LDADD = $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)


install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kimagemapeditor
	$(INSTALL_DATA) pics/polygoncursor.png $(kde_datadir)/kimagemapeditor/polygoncursor.png
	$(INSTALL_DATA) pics/freehandcursor.png $(kde_datadir)/kimagemapeditor/freehandcursor.png
	$(INSTALL_DATA) pics/addpointcursor.png $(kde_datadir)/kimagemapeditor/addpointcursor.png
	$(INSTALL_DATA) pics/removepointcursor.png $(kde_datadir)/kimagemapeditor/removepointcursor.png

uninstall-local:
	-rm -f $(kde_datadir)/kimagemapeditor/polygoncursor.png
	-rm -f $(kde_datadir)/kimagemapeditor/freehandcursor.png
	-rm -f $(kde_datadir)/kimagemapeditor/addpointcursor.png
	-rm -f $(kde_datadir)/kimagemapeditor/removepointcursor.png
	-rmdir $(kde_datadir)/kimagemapeditor

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

apps_DATA = kimagemapeditor.desktop
appsdir = $(kde_appsdir)/Development

rcdir = $(kde_datadir)/kimagemapeditor
rc_DATA = kimagemapeditorui.rc \
          kimagemapeditorpartui.rc

kde_services_DATA = kimagemapeditorpart.desktop

messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kimagemapeditor.pot; \
	fi

