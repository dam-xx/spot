AUTOMAKE_OPTIONS = subdir-objects

# List directories in the order they must be built.
# Keep tests at the end.
SUBDIRS = misc ltlenv ltlast ltlvisit ltlparse tgba tgbaalgos tgbaparse . \
	  ltltest tgbatest

lib_LTLIBRARIES = libspot.la
libspot_la_SOURCES =
libspot_la_LDFLAGS = $(BUDDY_LDFLAGS)
libspot_la_LIBADD =  \
  misc/libmisc.la \
  ltlenv/libltlenv.la \
  ltlparse/libltlparse.la \
  ltlvisit/libltlvisit.la \
  ltlast/libltlast.la \
  tgba/libtgba.la \
  tgbaalgos/libtgbaalgos.la \
  tgbaparse/libtgbaparse.la
