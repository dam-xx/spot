noinst_HEADERS = \
   bytecode.h \
   hashtab.h \
   instr.h \
   instr_step.h \
   instr_tools.h \
   instr_wrap.h \
   interactive.h \
   nipsvm.h \
   rt_err.h \
   search.h \
   split.h \
   state.h \
   state_inline.h \
   state_parts.h \
   timeval.h \
   tools.h

noinst_LTLIBRARIES = libnipsvm.la

libnipsvm_la_SOURCES = \
  bytecode.c \
  hashtab.c \
  instr.c \
  instr_step.c \
  instr_tools.c \
  instr_wrap.c \
  interactive.c \
  nipsvm.c \
  rt_err.c \
  search.c \
  split.c \
  state.c \
  state_parts.c

noinst_PROGRAMS = nipsvm

nipsvm_SOURCES = main.c
nipsvm_LDADD = libnipsvm.la
