AUTOMAKE_OPTIONS = subdir-objects

# List directories in the order they must be built.
SUBDIRS = misc ltlenv ltlast ltlvisit ltlparse ltltest tgba tgbaalgos

lib_LTLIBRARIES = libspot.la
libspot_la_SOURCES =
libspot_la_LIBADD =  \
  ltlenv/libltlenv.la \
  ltlparse/libltlparse.la \
  ltlvisit/libltlvisit.la \
  ltlast/libltlast.la \
  tgba/libtgba.la \
  tgbaalgos/libtgbaalgos.la
