SUBDIRS = pics

bin_PROGRAMS = kimagemapeditor
kde_module_LTLIBRARIES = libkimagemapeditor.la

METASOURCES = AUTO

noinst_LTLIBRARIES = libkimagemapeditorcommon.la

libkimagemapeditorcommon_la_SOURCES = \
    qextfileinfo.cpp \
    areacreator.cpp \
    kimearea.cpp \
    kimecommands.cpp \
    kimedialogs.cpp \
    kimagemapeditor.cpp \
    drawzone.cpp \
    arealistview.cpp \
    imageslistview.cpp \
    mapslistview.cpp 

libkimagemapeditor_la_SOURCES = \
    dummy.cpp

kimagemapeditor_SOURCES = \
    main.cpp \
    kimeshell.cpp 
    

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
libkimagemapeditor_la_LIBADD = libkimagemapeditorcommon.la $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)

kimagemapeditor_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimagemapeditor_LDADD = libkimagemapeditorcommon.la $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KHTML)

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

