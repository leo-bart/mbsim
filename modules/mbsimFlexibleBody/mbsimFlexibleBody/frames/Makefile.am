SUBDIRS = .

noinst_LTLIBRARIES = libframe.la
libframe_la_SOURCES = node_frame.cc\
		      frame_1s.cc\
		      floating_relative_flexible_contour_frame.cc\
		      frame_2s.cc

libframe_la_CPPFLAGS = -I$(top_srcdir) $(DEPS_CFLAGS) $(OPENMBVCPPINTERFACE_CFLAGS) $(NURBS_CFLAGS)
libframe_la_LIBADD = $(DEPS_LIBS) $(OPENMBVCPPINTERFACE_LIBS) $(NURBS_LIBS)

frameincludedir = $(includedir)/mbsimFlexibleBody/frames
frameinclude_HEADERS = node_frame.h\
		       frame_1s.h\
		       floating_relative_flexible_contour_frame.h\
		       frame_2s.h
