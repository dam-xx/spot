if WITH_INCLUDED_BUDDY
  MAYBE_BUDDY = buddy
endif WITH_INCLUDED_BUDDY
if WITH_INCLUDED_LBTT
  MAYBE_LBTT = lbtt
endif WITH_INCLUDED_LBTT

SUBDIRS = $(MAYBE_BUDDY) $(MAYBE_LBTT) doc src wrap iface

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = \
  m4/buddy.m4 \
  m4/debug.m4 \
  m4/devel.m4 \
  m4/gccoptim.m4 \
  m4/gccwarn.m4 \
  m4/gspnlib.m4 \
  m4/lbtt.m4 \
  m4/ndebug.m4 \
  m4/pypath.m4 \
  HACKING
