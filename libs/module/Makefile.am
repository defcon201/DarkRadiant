AM_CPPFLAGS = -DPKGLIBDIR='"$(pkglibdir)"' \
              -DPKGDATADIR='"$(pkgdatadir)"' \
              -DLOCALEDIR='"$(localedir)"' \
              -DHTMLDIR='"$(htmldir)"' \
			  -I$(top_srcdir)/include \
			  -I$(top_srcdir)/libs 
AM_CXXFLAGS = -fPIC

pkglib_LTLIBRARIES = libmodule.la
libmodule_la_LDFLAGS = -static $(DL_LIBS) -release @PACKAGE_VERSION@
libmodule_la_SOURCES = CoreModule.cpp \
                     DynamicLibrary.cpp \
					 ApplicationContextBase.cpp \
					 StaticModule.cpp